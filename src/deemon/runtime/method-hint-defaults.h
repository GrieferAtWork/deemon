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
#ifndef GUARD_DEEMON_RUNTIME_METHOD_HINT_DEFAULTS_H
#define GUARD_DEEMON_RUNTIME_METHOD_HINT_DEFAULTS_H 1

#include <deemon/api.h>

#include <deemon/method-hints.h>  /* Dee_seq_enumerate_index_t, Dee_seq_enumerate_t */
#include <deemon/none-operator.h> /* _DeeNone_* */
#include <deemon/object.h>        /* DREF, DeeObject, DeeObject_NewRef, DeeTypeObject, Dee_foreach_pair_t, Dee_foreach_t, Dee_hash_t, Dee_ssize_t */
#include <deemon/rodict.h>        /* DeeRoDict_FromSequence */
#include <deemon/roset.h>         /* DeeRoSet_FromSequence */

#include "../objects/generic-proxy.h" /* generic_obj__asmap */
#include "../objects/seq/concat.h"    /* DeeSeq_Concat */

#include <stddef.h> /* size_t */

DECL_BEGIN

/* clang-format off */
/*[[[deemon (printDefaultImplDecls from "..method-hints.method-hints")();]]]*/
/* seq_operator_bool */
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_bool(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_bool__with_callattr___seq_bool__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_bool__with_callobjectcache___seq_bool__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__seq_operator_bool__with_callobjectcache___seq_bool__(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_bool__unsupported(DeeObject *__restrict self);
#define default__seq_operator_bool__empty (*(int (DCALL *)(DeeObject *__restrict))&_DeeNone_reti0_1)
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_bool__with__seq_operator_foreach(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_bool__with__seq_operator_foreach_pair(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_bool__with__seq_operator_iter(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_bool__with__seq_operator_size(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_bool__with__seq_operator_sizeob(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_bool__with__seq_operator_compare_eq(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_bool__with__set_operator_compare_eq(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_bool__with__map_operator_compare_eq(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_bool__with__set_trygetfirst(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_bool__with__set_trygetlast(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_bool__with__set_boundfirst(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_bool__with__set_boundlast(DeeObject *__restrict self);

/* seq_operator_sizeob */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_sizeob(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_sizeob__with_callattr___seq_size__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_sizeob__with_callobjectcache___seq_size__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__seq_operator_sizeob__with_callobjectcache___seq_size__(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_sizeob__unsupported(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_sizeob__with__seq_operator_size(DeeObject *__restrict self);
#define default__seq_operator_sizeob__none (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&_DeeNone_NewRef1)
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_sizeob__empty(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_sizeob__with__seq_enumerate(DeeObject *__restrict self);

/* seq_operator_size */
INTDEF WUNUSED NONNULL((1)) size_t DCALL default__seq_operator_size(DeeObject *__restrict self);
#define default__seq_operator_size__with_callattr___seq_size__ (*(size_t (DCALL *)(DeeObject *__restrict))&default__seq_operator_size__with__seq_operator_sizeob)
#define default__seq_operator_size__with_callobjectcache___seq_size__ (*(size_t (DCALL *)(DeeObject *__restrict))&default__seq_operator_size__with__seq_operator_sizeob)
INTDEF WUNUSED NONNULL((1)) size_t DCALL default__seq_operator_size__unsupported(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) size_t DCALL default__seq_operator_size__with__seq_operator_sizeob(DeeObject *__restrict self);
#define default__seq_operator_size__empty (*(size_t (DCALL *)(DeeObject *__restrict))&_DeeNone_rets0_1)
INTDEF WUNUSED NONNULL((1)) size_t DCALL default__seq_operator_size__with__seq_operator_foreach(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) size_t DCALL default__seq_operator_size__with__seq_operator_foreach_pair(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) size_t DCALL default__seq_operator_size__with__set_operator_sizeob(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) size_t DCALL default__seq_operator_size__with__map_operator_sizeob(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) size_t DCALL default__seq_operator_size__with__map_enumerate(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) size_t DCALL default__seq_operator_size__with__seq_operator_iter(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) size_t DCALL default__seq_operator_size__with__seq_enumerate_index(DeeObject *__restrict self);

/* seq_operator_iter */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_iter(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_iter__with_callattr___seq_iter__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_iter__with_callobjectcache___seq_iter__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__seq_operator_iter__with_callobjectcache___seq_iter__(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_iter__unsupported(DeeObject *__restrict self);
#define default__seq_operator_iter__none (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&_DeeNone_NewRef1)
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_iter__empty(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_iter__with__seq_operator_size__and__operator_getitem_index_fast(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_iter__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_iter__with__seq_operator_size__and__seq_operator_getitem_index(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_iter__with__seq_operator_getitem_index(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_iter__with__seq_operator_sizeob__and__seq_operator_getitem(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_iter__with__seq_operator_getitem(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_iter__with__map_enumerate(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_iter__with__seq_enumerate(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_iter__with__seq_enumerate_index(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_iter__with__map_iterkeys__and__map_operator_trygetitem(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_iter__with__map_iterkeys__and__map_operator_getitem(DeeObject *__restrict self);

/* seq_operator_foreach */
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_operator_foreach(DeeObject *__restrict self, Dee_foreach_t cb, void *arg);
#define default__seq_operator_foreach__with_callattr___seq_iter__ (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&default__seq_operator_foreach__with__seq_operator_iter)
#define default__seq_operator_foreach__with_callobjectcache___seq_iter__ (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&default__seq_operator_foreach__with__seq_operator_iter)
#define default__seq_operator_foreach__unsupported (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&default__seq_operator_foreach__with__seq_operator_iter)
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_operator_foreach__with__seq_operator_foreach_pair(DeeObject *__restrict self, Dee_foreach_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_operator_foreach__with__seq_operator_iter(DeeObject *__restrict self, Dee_foreach_t cb, void *arg);
#define default__seq_operator_foreach__empty (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&_DeeNone_rets0_3)
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_operator_foreach__with__seq_enumerate(DeeObject *__restrict self, Dee_foreach_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_operator_foreach__with__seq_enumerate_index(DeeObject *__restrict self, Dee_foreach_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_operator_foreach__with__seq_operator_size__and__operator_getitem_index_fast(DeeObject *__restrict self, Dee_foreach_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *__restrict self, Dee_foreach_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_getitem_index(DeeObject *__restrict self, Dee_foreach_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_operator_foreach__with__seq_operator_sizeob__and__seq_operator_getitem(DeeObject *__restrict self, Dee_foreach_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_operator_foreach__with__seq_operator_getitem_index(DeeObject *__restrict self, Dee_foreach_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_operator_foreach__with__seq_operator_getitem(DeeObject *__restrict self, Dee_foreach_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_operator_foreach__with__map_enumerate(DeeObject *__restrict self, Dee_foreach_t cb, void *arg);

/* seq_operator_foreach_pair */
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_operator_foreach_pair(DeeObject *__restrict self, Dee_foreach_pair_t cb, void *arg);
#define default__seq_operator_foreach_pair__with_callattr___seq_iter__ (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&default__seq_operator_foreach_pair__with__seq_operator_foreach)
#define default__seq_operator_foreach_pair__with_callobjectcache___seq_iter__ (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&default__seq_operator_foreach_pair__with__seq_operator_foreach)
#define default__seq_operator_foreach_pair__unsupported (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&default__seq_operator_foreach_pair__with__seq_operator_foreach)
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_operator_foreach_pair__with__seq_operator_foreach(DeeObject *__restrict self, Dee_foreach_pair_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_operator_foreach_pair__with__seq_operator_iter(DeeObject *__restrict self, Dee_foreach_pair_t cb, void *arg);
#define default__seq_operator_foreach_pair__empty (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&_DeeNone_rets0_3)
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_operator_foreach_pair__with__map_enumerate(DeeObject *__restrict self, Dee_foreach_pair_t cb, void *arg);

/* seq_operator_getitem */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_getitem(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_getitem__with_callattr___seq_getitem__(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_getitem__with_callobjectcache___seq_getitem__(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__seq_operator_getitem__with_callobjectcache___seq_getitem__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_getitem__unsupported(DeeObject *self, DeeObject *index);
#define default__seq_operator_getitem__none (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&_DeeNone_NewRef2)
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_getitem__empty(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_getitem__with__seq_operator_getitem_index(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_getitem__with__seq_enumerate(DeeObject *self, DeeObject *index);

/* seq_operator_getitem_index */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_getitem_index(DeeObject *__restrict self, size_t index);
#define default__seq_operator_getitem_index__with_callattr___seq_getitem__ (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict, size_t))&default__seq_operator_getitem_index__with__seq_operator_getitem)
#define default__seq_operator_getitem_index__with_callobjectcache___seq_getitem__ (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict, size_t))&default__seq_operator_getitem_index__with__seq_operator_getitem)
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_getitem_index__unsupported(DeeObject *__restrict self, size_t index);
#define default__seq_operator_getitem_index__none (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict, size_t))&_DeeNone_NewRef2)
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_getitem_index__empty(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_getitem_index__with__seq_operator_foreach(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_getitem_index__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_getitem_index__with__seq_operator_getitem(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_getitem_index__with__map_enumerate(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_getitem_index__with__seq_enumerate_index(DeeObject *__restrict self, size_t index);

/* seq_operator_trygetitem */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_trygetitem(DeeObject *self, DeeObject *index);
#define default__seq_operator_trygetitem__with_callattr___seq_getitem__ (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&default__seq_operator_trygetitem__with__seq_operator_getitem)
#define default__seq_operator_trygetitem__with_callobjectcache___seq_getitem__ (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&default__seq_operator_trygetitem__with__seq_operator_getitem)
#define default__seq_operator_trygetitem__unsupported (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&default__seq_operator_getitem__unsupported)
#define default__seq_operator_trygetitem__none (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&_DeeNone_NewRef2)
#define default__seq_operator_trygetitem__empty (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&_DeeNone_retsm1_2)
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_trygetitem__with__seq_operator_getitem(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_trygetitem__with__seq_operator_trygetitem_index(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_trygetitem__with__seq_enumerate(DeeObject *self, DeeObject *index);

/* seq_operator_trygetitem_index */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_trygetitem_index(DeeObject *__restrict self, size_t index);
#define default__seq_operator_trygetitem_index__with_callattr___seq_getitem__ (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict, size_t))&default__seq_operator_trygetitem_index__with__seq_operator_getitem_index)
#define default__seq_operator_trygetitem_index__with_callobjectcache___seq_getitem__ (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict, size_t))&default__seq_operator_trygetitem_index__with__seq_operator_getitem_index)
#define default__seq_operator_trygetitem_index__unsupported (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict, size_t))&default__seq_operator_getitem_index__unsupported)
#define default__seq_operator_trygetitem_index__none (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict, size_t))&_DeeNone_NewRef2)
#define default__seq_operator_trygetitem_index__empty (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict, size_t))&_DeeNone_retsm1_2)
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_trygetitem_index__with__seq_operator_getitem_index(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_trygetitem_index__with__seq_operator_foreach(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_trygetitem_index__with__map_enumerate(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_trygetitem_index__with__seq_enumerate_index(DeeObject *__restrict self, size_t index);

/* seq_operator_hasitem */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_hasitem(DeeObject *self, DeeObject *index);
#define default__seq_operator_hasitem__with_callattr___seq_getitem__ (*(int (DCALL *)(DeeObject *, DeeObject *))&default__seq_operator_hasitem__with__seq_operator_getitem)
#define default__seq_operator_hasitem__with_callobjectcache___seq_getitem__ (*(int (DCALL *)(DeeObject *, DeeObject *))&default__seq_operator_hasitem__with__seq_operator_getitem)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_hasitem__unsupported(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_hasitem__none(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_hasitem__empty(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_hasitem__with__seq_operator_sizeob(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_hasitem__with__seq_operator_hasitem_index(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_hasitem__with__seq_operator_getitem(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_hasitem__with__seq_enumerate(DeeObject *self, DeeObject *index);

/* seq_operator_hasitem_index */
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_hasitem_index(DeeObject *__restrict self, size_t index);
#define default__seq_operator_hasitem_index__with_callattr___seq_getitem__ (*(int (DCALL *)(DeeObject *__restrict, size_t))&default__seq_operator_hasitem_index__with__seq_operator_getitem_index)
#define default__seq_operator_hasitem_index__with_callobjectcache___seq_getitem__ (*(int (DCALL *)(DeeObject *__restrict, size_t))&default__seq_operator_hasitem_index__with__seq_operator_getitem_index)
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_hasitem_index__unsupported(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_hasitem_index__none(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_hasitem_index__empty(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_hasitem_index__with__seq_operator_size(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_hasitem_index__with__seq_operator_getitem_index(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_hasitem_index__with__seq_enumerate_index(DeeObject *__restrict self, size_t index);

/* seq_operator_bounditem */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_bounditem(DeeObject *self, DeeObject *index);
#define default__seq_operator_bounditem__with_callattr___seq_getitem__ (*(int (DCALL *)(DeeObject *, DeeObject *))&default__seq_operator_bounditem__with__seq_operator_getitem)
#define default__seq_operator_bounditem__with_callobjectcache___seq_getitem__ (*(int (DCALL *)(DeeObject *, DeeObject *))&default__seq_operator_bounditem__with__seq_operator_getitem)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_bounditem__unsupported(DeeObject *self, DeeObject *index);
#define default__seq_operator_bounditem__none (*(int (DCALL *)(DeeObject *, DeeObject *))&_DeeNone_reti1_2)
#define default__seq_operator_bounditem__empty (*(int (DCALL *)(DeeObject *, DeeObject *))&_DeeNone_reti0_2)
#define default__seq_operator_bounditem__with__seq_operator_sizeob (*(int (DCALL *)(DeeObject *, DeeObject *))&default__seq_operator_hasitem__with__seq_operator_sizeob)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_bounditem__with__seq_operator_bounditem_index(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_bounditem__with__seq_operator_getitem(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_bounditem__with__seq_enumerate(DeeObject *self, DeeObject *index);

/* seq_operator_bounditem_index */
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_bounditem_index(DeeObject *__restrict self, size_t index);
#define default__seq_operator_bounditem_index__with_callattr___seq_getitem__ (*(int (DCALL *)(DeeObject *__restrict, size_t))&default__seq_operator_bounditem_index__with__seq_operator_getitem_index)
#define default__seq_operator_bounditem_index__with_callobjectcache___seq_getitem__ (*(int (DCALL *)(DeeObject *__restrict, size_t))&default__seq_operator_bounditem_index__with__seq_operator_getitem_index)
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_bounditem_index__unsupported(DeeObject *__restrict self, size_t index);
#define default__seq_operator_bounditem_index__none (*(int (DCALL *)(DeeObject *__restrict, size_t))&_DeeNone_reti1_2)
#define default__seq_operator_bounditem_index__empty (*(int (DCALL *)(DeeObject *__restrict, size_t))&_DeeNone_reti0_2)
#define default__seq_operator_bounditem_index__with__seq_operator_size (*(int (DCALL *)(DeeObject *__restrict, size_t))&default__seq_operator_hasitem_index__with__seq_operator_size)
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_bounditem_index__with__seq_operator_getitem_index(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_bounditem_index__with__map_enumerate(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_bounditem_index__with__seq_enumerate_index(DeeObject *__restrict self, size_t index);

/* seq_operator_delitem */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_delitem(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_delitem__with_callattr___seq_delitem__(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_delitem__with_callobjectcache___seq_delitem__(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__seq_operator_delitem__with_callobjectcache___seq_delitem__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_delitem__unsupported(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_delitem__with__seq_operator_delitem_index(DeeObject *self, DeeObject *index);
#define default__seq_operator_delitem__none (*(int (DCALL *)(DeeObject *, DeeObject *))&_DeeNone_reti0_2)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_delitem__empty(DeeObject *self, DeeObject *index);

/* seq_operator_delitem_index */
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_delitem_index(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_delitem_index__with_callattr___seq_delitem__(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_delitem_index__with_callobjectcache___seq_delitem__(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__seq_operator_delitem_index__with_callobjectcache___seq_delitem__(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_delitem_index__unsupported(DeeObject *__restrict self, size_t index);
#define default__seq_operator_delitem_index__none (*(int (DCALL *)(DeeObject *__restrict, size_t))&_DeeNone_reti0_2)
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_delitem_index__empty(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_delitem_index__with__seq_operator_delrange_index(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_delitem_index__with__seq_operator_size__and__seq_erase(DeeObject *__restrict self, size_t index);

/* seq_operator_setitem */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_operator_setitem(DeeObject *self, DeeObject *index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_operator_setitem__with_callattr___seq_setitem__(DeeObject *self, DeeObject *index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_operator_setitem__with_callobjectcache___seq_setitem__(DeeObject *self, DeeObject *index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL tdefault__seq_operator_setitem__with_callobjectcache___seq_setitem__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_operator_setitem__unsupported(DeeObject *self, DeeObject *index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_operator_setitem__with__seq_operator_setitem_index(DeeObject *self, DeeObject *index, DeeObject *value);
#define default__seq_operator_setitem__none (*(int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&_DeeNone_reti0_3)
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_operator_setitem__empty(DeeObject *self, DeeObject *index, DeeObject *value);

/* seq_operator_setitem_index */
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_operator_setitem_index(DeeObject *self, size_t index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_operator_setitem_index__with_callattr___seq_setitem__(DeeObject *self, size_t index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_operator_setitem_index__with_callobjectcache___seq_setitem__(DeeObject *self, size_t index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 4)) int DCALL tdefault__seq_operator_setitem_index__with_callobjectcache___seq_setitem__(DeeTypeObject *tp_self, DeeObject *self, size_t index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_operator_setitem_index__unsupported(DeeObject *self, size_t index, DeeObject *value);
#define default__seq_operator_setitem_index__none (*(int (DCALL *)(DeeObject *, size_t, DeeObject *))&_DeeNone_reti0_3)
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_operator_setitem_index__empty(DeeObject *self, size_t index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_operator_setitem_index__with__seq_operator_setrange_index(DeeObject *self, size_t index, DeeObject *value);

/* seq_operator_getrange */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_operator_getrange(DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_operator_getrange__with_callattr___seq_getrange__(DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_operator_getrange__with_callobjectcache___seq_getrange__(DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *DCALL tdefault__seq_operator_getrange__with_callobjectcache___seq_getrange__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_operator_getrange__unsupported(DeeObject *self, DeeObject *start, DeeObject *end);
#define default__seq_operator_getrange__none (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *))&_DeeNone_NewRef3)
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_operator_getrange__empty(DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_operator_getrange__with__seq_operator_getrange_index__and__seq_operator_getrange_index_n(DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_operator_getrange__with__seq_operator_sizeob__and__seq_operator_getitem(DeeObject *self, DeeObject *start, DeeObject *end);

/* seq_operator_getrange_index */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_getrange_index(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);
#define default__seq_operator_getrange_index__with_callattr___seq_getrange__ (*(DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&default__seq_operator_getrange_index__with__seq_operator_getrange)
#define default__seq_operator_getrange_index__with_callobjectcache___seq_getrange__ (*(DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&default__seq_operator_getrange_index__with__seq_operator_getrange)
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_getrange_index__unsupported(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_getrange_index__with__seq_operator_getrange(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);
#define default__seq_operator_getrange_index__none (*(DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&_DeeNone_NewRef3)
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_getrange_index__empty(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_getrange_index__with__seq_operator_size__and__operator_getitem_index_fast(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_getitem_index(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_getitem(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_iter(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);

/* seq_operator_getrange_index_n */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_getrange_index_n(DeeObject *self, Dee_ssize_t start);
#define default__seq_operator_getrange_index_n__with_callattr___seq_getrange__ (*(DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t))&default__seq_operator_getrange_index_n__with__seq_operator_getrange)
#define default__seq_operator_getrange_index_n__with_callobjectcache___seq_getrange__ (*(DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t))&default__seq_operator_getrange_index_n__with__seq_operator_getrange)
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_getrange_index_n__unsupported(DeeObject *self, Dee_ssize_t start);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_getrange_index_n__with__seq_operator_getrange(DeeObject *self, Dee_ssize_t start);
#define default__seq_operator_getrange_index_n__none (*(DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t))&_DeeNone_NewRef2)
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_getrange_index_n__empty(DeeObject *self, Dee_ssize_t start);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_getrange_index_n__with__seq_operator_size__and__operator_getitem_index_fast(DeeObject *self, Dee_ssize_t start);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_getrange_index(DeeObject *self, Dee_ssize_t start);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_getitem_index(DeeObject *self, Dee_ssize_t start);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *self, Dee_ssize_t start);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_getitem(DeeObject *self, Dee_ssize_t start);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_iter(DeeObject *self, Dee_ssize_t start);

/* seq_operator_delrange */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_operator_delrange(DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_operator_delrange__with_callattr___seq_delrange__(DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_operator_delrange__with_callobjectcache___seq_delrange__(DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL tdefault__seq_operator_delrange__with_callobjectcache___seq_delrange__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_operator_delrange__unsupported(DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_operator_delrange__with__seq_operator_delrange_index__and__seq_operator_delrange_index_n(DeeObject *self, DeeObject *start, DeeObject *end);
#define default__seq_operator_delrange__empty (*(int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&_DeeNone_reti0_3)
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_operator_delrange__with__seq_operator_setrange(DeeObject *self, DeeObject *start, DeeObject *end);

/* seq_operator_delrange_index */
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_delrange_index(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);
#define default__seq_operator_delrange_index__with_callattr___seq_delrange__ (*(int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&default__seq_operator_delrange_index__with__seq_operator_delrange)
#define default__seq_operator_delrange_index__with_callobjectcache___seq_delrange__ (*(int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&default__seq_operator_delrange_index__with__seq_operator_delrange)
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_delrange_index__unsupported(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_delrange_index__with__seq_operator_delrange(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);
#define default__seq_operator_delrange_index__empty (*(int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&_DeeNone_reti0_3)
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_delrange_index__with__seq_operator_size__and__seq_erase(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_delrange_index__with__seq_operator_setrange_index(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);

/* seq_operator_delrange_index_n */
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_delrange_index_n(DeeObject *self, Dee_ssize_t start);
#define default__seq_operator_delrange_index_n__with_callattr___seq_delrange__ (*(int (DCALL *)(DeeObject *, Dee_ssize_t))&default__seq_operator_delrange_index_n__with__seq_operator_delrange)
#define default__seq_operator_delrange_index_n__with_callobjectcache___seq_delrange__ (*(int (DCALL *)(DeeObject *, Dee_ssize_t))&default__seq_operator_delrange_index_n__with__seq_operator_delrange)
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_delrange_index_n__unsupported(DeeObject *self, Dee_ssize_t start);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_delrange_index_n__with__seq_operator_size__and__seq_operator_delrange_index(DeeObject *self, Dee_ssize_t start);
#define default__seq_operator_delrange_index_n__empty (*(int (DCALL *)(DeeObject *, Dee_ssize_t))&_DeeNone_reti0_2)
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_delrange_index_n__with__seq_operator_delrange(DeeObject *self, Dee_ssize_t start);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_delrange_index_n__with__seq_operator_setrange_index_n(DeeObject *self, Dee_ssize_t start);

/* seq_operator_setrange */
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL default__seq_operator_setrange(DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL default__seq_operator_setrange__with_callattr___seq_setrange__(DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL default__seq_operator_setrange__with_callobjectcache___seq_setrange__(DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5)) int DCALL tdefault__seq_operator_setrange__with_callobjectcache___seq_setrange__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL default__seq_operator_setrange__unsupported(DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL default__seq_operator_setrange__with__seq_operator_setrange_index__and__seq_operator_setrange_index_n(DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *items);
#define default__seq_operator_setrange__none (*(int (DCALL *)(DeeObject *, DeeObject *, DeeObject *, DeeObject *))&_DeeNone_reti0_4)
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL default__seq_operator_setrange__empty(DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *items);

/* seq_operator_setrange_index */
INTDEF WUNUSED NONNULL((1, 4)) int DCALL default__seq_operator_setrange_index(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *items);
#define default__seq_operator_setrange_index__with_callattr___seq_setrange__ (*(int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t, DeeObject *))&default__seq_operator_setrange_index__with__seq_operator_setrange)
#define default__seq_operator_setrange_index__with_callobjectcache___seq_setrange__ (*(int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t, DeeObject *))&default__seq_operator_setrange_index__with__seq_operator_setrange)
INTDEF WUNUSED NONNULL((1, 4)) int DCALL default__seq_operator_setrange_index__unsupported(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL default__seq_operator_setrange_index__with__seq_operator_setrange(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *items);
#define default__seq_operator_setrange_index__none (*(int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t, DeeObject *))&_DeeNone_reti0_4)
INTDEF WUNUSED NONNULL((1, 4)) int DCALL default__seq_operator_setrange_index__empty(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL default__seq_operator_setrange_index__with__seq_operator_size__and__seq_erase__and__seq_insertall(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *items);

/* seq_operator_setrange_index_n */
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_operator_setrange_index_n(DeeObject *self, Dee_ssize_t start, DeeObject *items);
#define default__seq_operator_setrange_index_n__with_callattr___seq_setrange__ (*(int (DCALL *)(DeeObject *, Dee_ssize_t, DeeObject *))&default__seq_operator_setrange_index_n__with__seq_operator_setrange)
#define default__seq_operator_setrange_index_n__with_callobjectcache___seq_setrange__ (*(int (DCALL *)(DeeObject *, Dee_ssize_t, DeeObject *))&default__seq_operator_setrange_index_n__with__seq_operator_setrange)
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_operator_setrange_index_n__unsupported(DeeObject *self, Dee_ssize_t start, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_operator_setrange_index_n__with__seq_operator_size__and__seq_operator_setrange_index(DeeObject *self, Dee_ssize_t start, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_operator_setrange_index_n__with__seq_operator_setrange(DeeObject *self, Dee_ssize_t start, DeeObject *items);
#define default__seq_operator_setrange_index_n__none (*(int (DCALL *)(DeeObject *, Dee_ssize_t, DeeObject *))&_DeeNone_reti0_3)
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_operator_setrange_index_n__empty(DeeObject *self, Dee_ssize_t start, DeeObject *items);

/* seq_operator_assign */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_assign(DeeObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_assign__with_callattr___seq_assign__(DeeObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_assign__with_callobjectcache___seq_assign__(DeeObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__seq_operator_assign__with_callobjectcache___seq_assign__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_assign__unsupported(DeeObject *self, DeeObject *items);
#define default__seq_operator_assign__none (*(int (DCALL *)(DeeObject *, DeeObject *))&_DeeNone_reti0_2)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_assign__empty(DeeObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_assign__with__seq_operator_setrange(DeeObject *self, DeeObject *items);

/* seq_operator_hash */
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL default__seq_operator_hash(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL default__seq_operator_hash__with_callattr___seq_hash__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL default__seq_operator_hash__with_callobjectcache___seq_hash__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) Dee_hash_t DCALL tdefault__seq_operator_hash__with_callobjectcache___seq_hash__(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL default__seq_operator_hash__unsupported(DeeObject *__restrict self);
#define default__seq_operator_hash__none (*(Dee_hash_t (DCALL *)(DeeObject *__restrict))&_DeeNone_rets0_1)
#define default__seq_operator_hash__empty (*(Dee_hash_t (DCALL *)(DeeObject *__restrict))&_DeeNone_rets0_1)
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL default__seq_operator_hash__with__seq_operator_foreach(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL default__seq_operator_hash__with__seq_operator_foreach_pair(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL default__seq_operator_hash__with__seq_operator_size__and__operator_getitem_index_fast(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL default__seq_operator_hash__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL default__seq_operator_hash__with__seq_operator_size__and__seq_operator_getitem_index(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL default__seq_operator_hash__with__seq_operator_sizeob__and__seq_operator_getitem(DeeObject *__restrict self);

/* seq_operator_compare */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_compare(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_compare__with_callattr_compare(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_compare__with_callattr___seq_compare__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_compare__with_callobjectcache___seq_compare__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__seq_operator_compare__with_callobjectcache___seq_compare__(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_compare__unsupported(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_compare__empty(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_compare__with__seq_operator_eq__and__seq_operator__lo(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_compare__with__seq_operator_eq__and__seq_operator__le(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_compare__with__seq_operator_eq__and__seq_operator__gr(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_compare__with__seq_operator_eq__and__seq_operator__ge(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_compare__with__seq_operator_ne__and__seq_operator__lo(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_compare__with__seq_operator_ne__and__seq_operator__le(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_compare__with__seq_operator_ne__and__seq_operator__gr(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_compare__with__seq_operator_ne__and__seq_operator__ge(DeeObject *lhs, DeeObject *rhs);
#define default__seq_operator_compare__with__seq_operator_lo__and__seq_operator__gr (*(int (DCALL *)(DeeObject *, DeeObject *))&default__seq_operator_compare_eq__with__seq_operator_lo__and__seq_operator__gr)
#define default__seq_operator_compare__with__seq_operator_le__and__seq_operator__ge (*(int (DCALL *)(DeeObject *, DeeObject *))&default__seq_operator_compare_eq__with__seq_operator_le__and__seq_operator__ge)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_compare__with__seq_operator_foreach(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_compare__with__seq_operator_size__and__operator_getitem_index_fast(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_compare__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_compare__with__seq_operator_size__and__seq_operator_getitem_index(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_compare__with__seq_operator_sizeob__and__seq_operator_getitem(DeeObject *lhs, DeeObject *rhs);

/* seq_operator_compare_eq */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_compare_eq(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_compare_eq__with_callattr_equals(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_compare_eq__with_callattr___seq_compare_eq__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_compare_eq__with_callobjectcache___seq_compare_eq__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__seq_operator_compare_eq__with_callobjectcache___seq_compare_eq__(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_compare_eq__unsupported(DeeObject *lhs, DeeObject *rhs);
#define default__seq_operator_compare_eq__empty (*(int (DCALL *)(DeeObject *, DeeObject *))&default__seq_operator_compare__empty)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_compare_eq__with__seq_operator_eq(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_compare_eq__with__seq_operator_ne(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_compare_eq__with__seq_operator_lo__and__seq_operator__gr(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_compare_eq__with__seq_operator_le__and__seq_operator__ge(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_compare_eq__with__seq_operator_foreach(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_compare_eq__with__seq_operator_size__and__operator_getitem_index_fast(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_compare_eq__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_compare_eq__with__seq_operator_size__and__seq_operator_getitem_index(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_compare_eq__with__seq_operator_sizeob__and__seq_operator_getitem(DeeObject *lhs, DeeObject *rhs);

/* seq_operator_trycompare_eq */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_trycompare_eq(DeeObject *lhs, DeeObject *rhs);
#define default__seq_operator_trycompare_eq__with_callattr_equals (*(int (DCALL *)(DeeObject *, DeeObject *))&default__seq_operator_trycompare_eq__with__seq_operator_compare_eq)
#define default__seq_operator_trycompare_eq__with_callattr___seq_compare_eq__ (*(int (DCALL *)(DeeObject *, DeeObject *))&default__seq_operator_trycompare_eq__with__seq_operator_compare_eq)
#define default__seq_operator_trycompare_eq__with_callobjectcache___seq_compare_eq__ (*(int (DCALL *)(DeeObject *, DeeObject *))&default__seq_operator_trycompare_eq__with__seq_operator_compare_eq)
#define default__seq_operator_trycompare_eq__unsupported (*(int (DCALL *)(DeeObject *, DeeObject *))&_DeeNone_reti1_2)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_trycompare_eq__empty(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_trycompare_eq__with__seq_operator_compare_eq(DeeObject *lhs, DeeObject *rhs);

/* seq_operator_eq */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_eq(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_eq__with_callattr___seq_eq__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_eq__with_callobjectcache___seq_eq__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__seq_operator_eq__with_callobjectcache___seq_eq__(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_eq__unsupported(DeeObject *lhs, DeeObject *rhs);
#define default__seq_operator_eq__empty (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&default__seq_operator_eq__with__seq_operator_compare_eq)
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_eq__with__seq_operator_ne(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_eq__with__seq_operator_compare_eq(DeeObject *lhs, DeeObject *rhs);

/* seq_operator_ne */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_ne(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_ne__with_callattr___seq_ne__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_ne__with_callobjectcache___seq_ne__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__seq_operator_ne__with_callobjectcache___seq_ne__(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_ne__unsupported(DeeObject *lhs, DeeObject *rhs);
#define default__seq_operator_ne__empty (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&default__seq_operator_ne__with__seq_operator_compare_eq)
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_ne__with__seq_operator_eq(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_ne__with__seq_operator_compare_eq(DeeObject *lhs, DeeObject *rhs);

/* seq_operator_lo */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_lo(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_lo__with_callattr___seq_lo__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_lo__with_callobjectcache___seq_lo__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__seq_operator_lo__with_callobjectcache___seq_lo__(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_lo__unsupported(DeeObject *lhs, DeeObject *rhs);
#define default__seq_operator_lo__empty (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&default__seq_operator_lo__with__seq_operator_compare)
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_lo__with__seq_operator_ge(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_lo__with__seq_operator_compare(DeeObject *lhs, DeeObject *rhs);

/* seq_operator_le */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_le(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_le__with_callattr___seq_le__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_le__with_callobjectcache___seq_le__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__seq_operator_le__with_callobjectcache___seq_le__(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_le__unsupported(DeeObject *lhs, DeeObject *rhs);
#define default__seq_operator_le__empty (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&default__seq_operator_le__with__seq_operator_compare)
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_le__with__seq_operator_gr(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_le__with__seq_operator_compare(DeeObject *lhs, DeeObject *rhs);

/* seq_operator_gr */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_gr(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_gr__with_callattr___seq_gr__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_gr__with_callobjectcache___seq_gr__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__seq_operator_gr__with_callobjectcache___seq_gr__(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_gr__unsupported(DeeObject *lhs, DeeObject *rhs);
#define default__seq_operator_gr__empty (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&default__seq_operator_gr__with__seq_operator_compare)
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_gr__with__seq_operator_le(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_gr__with__seq_operator_compare(DeeObject *lhs, DeeObject *rhs);

/* seq_operator_ge */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_ge(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_ge__with_callattr___seq_ge__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_ge__with_callobjectcache___seq_ge__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__seq_operator_ge__with_callobjectcache___seq_ge__(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_ge__unsupported(DeeObject *lhs, DeeObject *rhs);
#define default__seq_operator_ge__empty (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&default__seq_operator_ge__with__seq_operator_compare)
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_ge__with__seq_operator_lo(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_ge__with__seq_operator_compare(DeeObject *lhs, DeeObject *rhs);

/* seq_operator_add */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_add(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_add__with_callattr___seq_add__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_add__with_callobjectcache___seq_add__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__seq_operator_add__with_callobjectcache___seq_add__(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
#define default__seq_operator_add__unsupported (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&default__seq_operator_add__with__DeeSeq_Concat)
#define default__seq_operator_add__none (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&_DeeNone_NewRef2)
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_add__empty(DeeObject *lhs, DeeObject *rhs);
#define default__seq_operator_add__with__DeeSeq_Concat (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&DeeSeq_Concat)

/* seq_operator_mul */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_mul(DeeObject *self, DeeObject *repeat);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_mul__with_callattr___seq_mul__(DeeObject *self, DeeObject *repeat);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_mul__with_callobjectcache___seq_mul__(DeeObject *self, DeeObject *repeat);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__seq_operator_mul__with_callobjectcache___seq_mul__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *repeat);
#define default__seq_operator_mul__unsupported (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&default__seq_operator_mul__with__DeeSeq_Repeat)
#define default__seq_operator_mul__none (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&_DeeNone_NewRef2)
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_mul__empty(DeeObject *self, DeeObject *repeat);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_mul__with__DeeSeq_Repeat(DeeObject *self, DeeObject *repeat);

/* seq_operator_inplace_add */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_inplace_add(DREF DeeObject **__restrict p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_inplace_add__with_callattr___seq_inplace_add__(DREF DeeObject **__restrict p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_inplace_add__with_callobjectcache___seq_inplace_add__(DREF DeeObject **__restrict p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__seq_operator_inplace_add__with_callobjectcache___seq_inplace_add__(DeeTypeObject *tp_self, DREF DeeObject **p_lhs, DeeObject *rhs);
#define default__seq_operator_inplace_add__unsupported (*(int (DCALL *)(DREF DeeObject **__restrict, DeeObject *))&default__seq_operator_inplace_add__with__seq_operator_add)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_inplace_add__with__seq_extend(DREF DeeObject **__restrict p_lhs, DeeObject *rhs);
#define default__seq_operator_inplace_add__none (*(int (DCALL *)(DREF DeeObject **__restrict, DeeObject *))&_DeeNone_reti0_2)
#define default__seq_operator_inplace_add__empty (*(int (DCALL *)(DREF DeeObject **__restrict, DeeObject *))&default__seq_operator_inplace_add__with__seq_operator_add)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_inplace_add__with__seq_operator_add(DREF DeeObject **__restrict p_lhs, DeeObject *rhs);

/* seq_operator_inplace_mul */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_inplace_mul(DREF DeeObject **__restrict p_lhs, DeeObject *repeat);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_inplace_mul__with_callattr___seq_inplace_mul__(DREF DeeObject **__restrict p_lhs, DeeObject *repeat);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_inplace_mul__with_callobjectcache___seq_inplace_mul__(DREF DeeObject **__restrict p_lhs, DeeObject *repeat);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__seq_operator_inplace_mul__with_callobjectcache___seq_inplace_mul__(DeeTypeObject *tp_self, DREF DeeObject **p_lhs, DeeObject *repeat);
#define default__seq_operator_inplace_mul__unsupported (*(int (DCALL *)(DREF DeeObject **__restrict, DeeObject *))&default__seq_operator_inplace_mul__with__seq_operator_mul)
#define default__seq_operator_inplace_mul__empty (*(int (DCALL *)(DREF DeeObject **__restrict, DeeObject *))&_DeeNone_reti0_2)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_inplace_mul__with__seq_clear__and__seq_extend(DREF DeeObject **__restrict p_lhs, DeeObject *repeat);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_inplace_mul__with__seq_operator_mul(DREF DeeObject **__restrict p_lhs, DeeObject *repeat);

/* seq_enumerate */
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_enumerate__with_callattr___seq_enumerate__(DeeObject *__restrict self, Dee_seq_enumerate_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_enumerate__with_callobjectcache___seq_enumerate__(DeeObject *__restrict self, Dee_seq_enumerate_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL tdefault__seq_enumerate__with_callobjectcache___seq_enumerate__(DeeTypeObject *tp_self, DeeObject *self, Dee_seq_enumerate_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_enumerate__unsupported(DeeObject *__restrict self, Dee_seq_enumerate_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_enumerate__with__seq_enumerate_index(DeeObject *__restrict self, Dee_seq_enumerate_t cb, void *arg);
#define default__seq_enumerate__empty (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_seq_enumerate_t, void *))&_DeeNone_rets0_3)
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_enumerate__with__seq_operator_size__and__operator_getitem_index_fast(DeeObject *__restrict self, Dee_seq_enumerate_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_enumerate__with__seq_operator_size__and__seq_operator_getitem_index(DeeObject *__restrict self, Dee_seq_enumerate_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_enumerate__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *__restrict self, Dee_seq_enumerate_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_enumerate__with__seq_operator_sizeob__and__seq_operator_getitem(DeeObject *__restrict self, Dee_seq_enumerate_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_enumerate__with__seq_operator_getitem_index(DeeObject *__restrict self, Dee_seq_enumerate_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_enumerate__with__seq_operator_getitem(DeeObject *__restrict self, Dee_seq_enumerate_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_enumerate__with__seq_operator_foreach__and__counter(DeeObject *__restrict self, Dee_seq_enumerate_t cb, void *arg);

/* seq_enumerate_index */
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_enumerate_index__with_callattr___seq_enumerate__(DeeObject *__restrict self, Dee_seq_enumerate_index_t cb, void *arg, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_enumerate_index__with_callobjectcache___seq_enumerate__(DeeObject *__restrict self, Dee_seq_enumerate_index_t cb, void *arg, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL tdefault__seq_enumerate_index__with_callobjectcache___seq_enumerate__(DeeTypeObject *tp_self, DeeObject *self, Dee_seq_enumerate_index_t cb, void *arg, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_enumerate_index__unsupported(DeeObject *__restrict self, Dee_seq_enumerate_index_t cb, void *arg, size_t start, size_t end);
#define default__seq_enumerate_index__empty (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_seq_enumerate_index_t, void *, size_t, size_t))&_DeeNone_rets0_5)
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_enumerate_index__with__seq_operator_size__and__operator_getitem_index_fast(DeeObject *__restrict self, Dee_seq_enumerate_index_t cb, void *arg, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_enumerate_index__with__seq_operator_size__and__seq_operator_getitem_index(DeeObject *__restrict self, Dee_seq_enumerate_index_t cb, void *arg, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_enumerate_index__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *__restrict self, Dee_seq_enumerate_index_t cb, void *arg, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_enumerate_index__with__seq_operator_getitem_index(DeeObject *__restrict self, Dee_seq_enumerate_index_t cb, void *arg, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_enumerate_index__with__seq_enumerate(DeeObject *__restrict self, Dee_seq_enumerate_index_t cb, void *arg, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_enumerate_index__with__seq_operator_iter(DeeObject *__restrict self, Dee_seq_enumerate_index_t cb, void *arg, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_enumerate_index__with__seq_operator_foreach__and__counter(DeeObject *__restrict self, Dee_seq_enumerate_index_t cb, void *arg, size_t start, size_t end);

/* seq_makeenumeration */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_makeenumeration__with_callattr___seq_enumerate_items__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_makeenumeration__with_callobjectcache___seq_enumerate_items__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__seq_makeenumeration__with_callobjectcache___seq_enumerate_items__(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_makeenumeration__unsupported(DeeObject *__restrict self);
#define default__seq_makeenumeration__none (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&_DeeNone_NewRef1)
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_makeenumeration__empty(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_makeenumeration__with__seq_operator_size__and__operator_getitem_index_fast(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_makeenumeration__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_makeenumeration__with__seq_operator_size__and__seq_operator_getitem_index(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_makeenumeration__with__seq_operator_sizeob__and__seq_operator_getitem(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_makeenumeration__with__seq_operator_getitem_index(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_makeenumeration__with__seq_operator_getitem(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_makeenumeration__with__seq_operator_iter__and__counter(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_makeenumeration__with__seq_enumerate(DeeObject *__restrict self);

/* seq_makeenumeration_with_range */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_makeenumeration_with_range__with_callattr___seq_enumerate_items__(DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_makeenumeration_with_range__with_callobjectcache___seq_enumerate_items__(DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *DCALL tdefault__seq_makeenumeration_with_range__with_callobjectcache___seq_enumerate_items__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_makeenumeration_with_range__unsupported(DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_makeenumeration_with_range__with__seq_makeenumeration_with_intrange(DeeObject *self, DeeObject *start, DeeObject *end);
#define default__seq_makeenumeration_with_range__none (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *))&_DeeNone_NewRef3)
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_makeenumeration_with_range__empty(DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_makeenumeration_with_range__with__seq_operator_sizeob__and__seq_operator_getitem(DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_makeenumeration_with_range__with__seq_operator_getitem(DeeObject *self, DeeObject *start, DeeObject *end);

/* seq_makeenumeration_with_intrange */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_makeenumeration_with_intrange__with_callattr___seq_enumerate_items__(DeeObject *__restrict self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_makeenumeration_with_intrange__with_callobjectcache___seq_enumerate_items__(DeeObject *__restrict self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__seq_makeenumeration_with_intrange__with_callobjectcache___seq_enumerate_items__(DeeTypeObject *tp_self, DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_makeenumeration_with_intrange__unsupported(DeeObject *__restrict self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_makeenumeration_with_intrange__with__seq_makeenumeration_with_range(DeeObject *__restrict self, size_t start, size_t end);
#define default__seq_makeenumeration_with_intrange__none (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict, size_t, size_t))&_DeeNone_NewRef3)
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_makeenumeration_with_intrange__empty(DeeObject *__restrict self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_makeenumeration_with_intrange__with__seq_operator_size__and__operator_getitem_index_fast(DeeObject *__restrict self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_makeenumeration_with_intrange__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *__restrict self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_makeenumeration_with_intrange__with__seq_operator_size__and__seq_operator_getitem_index(DeeObject *__restrict self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_makeenumeration_with_intrange__with__seq_operator_getitem_index(DeeObject *__restrict self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_makeenumeration_with_intrange__with__seq_operator_iter__and__counter(DeeObject *__restrict self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_makeenumeration_with_intrange__with__seq_enumerate_index(DeeObject *__restrict self, size_t start, size_t end);

/* seq_foreach_reverse */
#define default__seq_foreach_reverse__empty (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&_DeeNone_rets0_3)
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_foreach_reverse__with__seq_operator_size__and__operator_getitem_index_fast(DeeObject *__restrict self, Dee_foreach_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_foreach_reverse__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *__restrict self, Dee_foreach_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_foreach_reverse__with__seq_operator_size__and__seq_operator_getitem_index(DeeObject *__restrict self, Dee_foreach_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_foreach_reverse__with__seq_operator_sizeob__and__seq_operator_getitem(DeeObject *__restrict self, Dee_foreach_t cb, void *arg);

/* seq_enumerate_index_reverse */
#define default__seq_enumerate_index_reverse__empty (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_seq_enumerate_index_t, void *, size_t, size_t))&_DeeNone_rets0_5)
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_enumerate_index_reverse__with__seq_operator_size__and__operator_getitem_index_fast(DeeObject *__restrict self, Dee_seq_enumerate_index_t cb, void *arg, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_enumerate_index_reverse__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *__restrict self, Dee_seq_enumerate_index_t cb, void *arg, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_enumerate_index_reverse__with__seq_operator_size__and__seq_operator_getitem_index(DeeObject *__restrict self, Dee_seq_enumerate_index_t cb, void *arg, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_enumerate_index_reverse__with__seq_operator_sizeob__and__seq_operator_getitem(DeeObject *__restrict self, Dee_seq_enumerate_index_t cb, void *arg, size_t start, size_t end);

/* seq_unpack */
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_unpack__with_callattr_unpack(DeeObject *__restrict self, size_t count, DREF DeeObject *result[]);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_unpack__with_callattr___seq_unpack__(DeeObject *__restrict self, size_t count, DREF DeeObject *result[]);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_unpack__with_callobjectcache___seq_unpack__(DeeObject *__restrict self, size_t count, DREF DeeObject *result[]);
INTDEF WUNUSED NONNULL((1, 2, 4)) int DCALL tdefault__seq_unpack__with_callobjectcache___seq_unpack__(DeeTypeObject *tp_self, DeeObject *self, size_t count, DREF DeeObject *result[]);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_unpack__unsupported(DeeObject *__restrict self, size_t count, DREF DeeObject *result[]);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_unpack__with__seq_unpack_ex(DeeObject *__restrict self, size_t count, DREF DeeObject *result[]);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_unpack__none(DeeObject *__restrict self, size_t count, DREF DeeObject *result[]);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_unpack__empty(DeeObject *__restrict self, size_t count, DREF DeeObject *result[]);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_unpack__with__tp_asvector(DeeObject *__restrict self, size_t count, DREF DeeObject *result[]);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_unpack__with__seq_operator_size__and__operator_getitem_index_fast(DeeObject *__restrict self, size_t count, DREF DeeObject *result[]);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_unpack__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *__restrict self, size_t count, DREF DeeObject *result[]);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_unpack__with__seq_operator_size__and__seq_operator_getitem_index(DeeObject *__restrict self, size_t count, DREF DeeObject *result[]);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_unpack__with__seq_operator_foreach(DeeObject *__restrict self, size_t count, DREF DeeObject *result[]);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_unpack__with__seq_operator_iter(DeeObject *__restrict self, size_t count, DREF DeeObject *result[]);

/* seq_unpack_ex */
INTDEF WUNUSED NONNULL((1, 4)) size_t DCALL default__seq_unpack_ex__with_callattr_unpack(DeeObject *__restrict self, size_t min_count, size_t max_count, DREF DeeObject *result[]);
INTDEF WUNUSED NONNULL((1, 4)) size_t DCALL default__seq_unpack_ex__with_callattr___seq_unpack__(DeeObject *__restrict self, size_t min_count, size_t max_count, DREF DeeObject *result[]);
INTDEF WUNUSED NONNULL((1, 4)) size_t DCALL default__seq_unpack_ex__with_callobjectcache___seq_unpack__(DeeObject *__restrict self, size_t min_count, size_t max_count, DREF DeeObject *result[]);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL tdefault__seq_unpack_ex__with_callobjectcache___seq_unpack__(DeeTypeObject *tp_self, DeeObject *self, size_t min_count, size_t max_count, DREF DeeObject *result[]);
INTDEF WUNUSED NONNULL((1, 4)) size_t DCALL default__seq_unpack_ex__unsupported(DeeObject *__restrict self, size_t min_count, size_t max_count, DREF DeeObject *result[]);
INTDEF WUNUSED NONNULL((1, 4)) size_t DCALL default__seq_unpack_ex__none(DeeObject *__restrict self, size_t min_count, size_t max_count, DREF DeeObject *result[]);
INTDEF WUNUSED NONNULL((1, 4)) size_t DCALL default__seq_unpack_ex__empty(DeeObject *__restrict self, size_t min_count, size_t max_count, DREF DeeObject *result[]);
INTDEF WUNUSED NONNULL((1, 4)) size_t DCALL default__seq_unpack_ex__with__tp_asvector(DeeObject *__restrict self, size_t min_count, size_t max_count, DREF DeeObject *result[]);
INTDEF WUNUSED NONNULL((1, 4)) size_t DCALL default__seq_unpack_ex__with__seq_operator_size__and__operator_getitem_index_fast(DeeObject *__restrict self, size_t min_count, size_t max_count, DREF DeeObject *result[]);
INTDEF WUNUSED NONNULL((1, 4)) size_t DCALL default__seq_unpack_ex__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *__restrict self, size_t min_count, size_t max_count, DREF DeeObject *result[]);
INTDEF WUNUSED NONNULL((1, 4)) size_t DCALL default__seq_unpack_ex__with__seq_operator_size__and__seq_operator_getitem_index(DeeObject *__restrict self, size_t min_count, size_t max_count, DREF DeeObject *result[]);
INTDEF WUNUSED NONNULL((1, 4)) size_t DCALL default__seq_unpack_ex__with__seq_operator_foreach(DeeObject *__restrict self, size_t min_count, size_t max_count, DREF DeeObject *result[]);
INTDEF WUNUSED NONNULL((1, 4)) size_t DCALL default__seq_unpack_ex__with__seq_operator_iter(DeeObject *__restrict self, size_t min_count, size_t max_count, DREF DeeObject *result[]);

/* seq_unpack_ub */
INTDEF WUNUSED NONNULL((1, 4)) size_t DCALL default__seq_unpack_ub__with_callattr_unpackub(DeeObject *__restrict self, size_t min_count, size_t max_count, DREF DeeObject *result[]);
INTDEF WUNUSED NONNULL((1, 4)) size_t DCALL default__seq_unpack_ub__with_callattr___seq_unpackub__(DeeObject *__restrict self, size_t min_count, size_t max_count, DREF DeeObject *result[]);
INTDEF WUNUSED NONNULL((1, 4)) size_t DCALL default__seq_unpack_ub__with_callobjectcache___seq_unpackub__(DeeObject *__restrict self, size_t min_count, size_t max_count, DREF DeeObject *result[]);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL tdefault__seq_unpack_ub__with_callobjectcache___seq_unpackub__(DeeTypeObject *tp_self, DeeObject *self, size_t min_count, size_t max_count, DREF DeeObject *result[]);
INTDEF WUNUSED NONNULL((1, 4)) size_t DCALL default__seq_unpack_ub__unsupported(DeeObject *__restrict self, size_t min_count, size_t max_count, DREF DeeObject *result[]);
#define default__seq_unpack_ub__none (*(size_t (DCALL *)(DeeObject *__restrict, size_t, size_t, DREF DeeObject **))&default__seq_unpack_ex__none)
INTDEF WUNUSED NONNULL((1, 4)) size_t DCALL default__seq_unpack_ub__empty(DeeObject *__restrict self, size_t min_count, size_t max_count, DREF DeeObject *result[]);
INTDEF WUNUSED NONNULL((1, 4)) size_t DCALL default__seq_unpack_ub__with__seq_operator_size__and__operator_getitem_index_fast(DeeObject *__restrict self, size_t min_count, size_t max_count, DREF DeeObject *result[]);
INTDEF WUNUSED NONNULL((1, 4)) size_t DCALL default__seq_unpack_ub__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *__restrict self, size_t min_count, size_t max_count, DREF DeeObject *result[]);
INTDEF WUNUSED NONNULL((1, 4)) size_t DCALL default__seq_unpack_ub__with__seq_operator_size__and__seq_operator_getitem_index(DeeObject *__restrict self, size_t min_count, size_t max_count, DREF DeeObject *result[]);

/* seq_trygetfirst */
#define default__seq_trygetfirst__with_callattr_first (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&default__seq_trygetfirst__with__seq_getfirst)
#define default__seq_trygetfirst__with_callattr___seq_first__ (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&default__seq_trygetfirst__with__seq_getfirst)
#define default__seq_trygetfirst__with_callobjectcache___seq_first__ (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&default__seq_trygetfirst__with__seq_getfirst)
#define default__seq_trygetfirst__unsupported (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&default__seq_getfirst__unsupported)
#define default__seq_trygetfirst__none (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&_DeeNone_NewRef1)
#define default__seq_trygetfirst__empty (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&_DeeNone_retsm1_1)
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_trygetfirst__with__seq_getfirst(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_trygetfirst__with__seq_operator_size__and__operator_getitem_index_fast(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_trygetfirst__with__seq_operator_trygetitem_index(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_trygetfirst__with__seq_operator_foreach(DeeObject *__restrict self);

/* seq_getfirst */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_getfirst(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_getfirst__with_callattr_first(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_getfirst__with_callattr___seq_first__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_getfirst__with_callobjectcache___seq_first__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__seq_getfirst__with_callobjectcache___seq_first__(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_getfirst__unsupported(DeeObject *__restrict self);
#define default__seq_getfirst__none (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&_DeeNone_NewRef1)
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_getfirst__empty(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_getfirst__with__seq_operator_getitem_index(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_getfirst__with__seq_trygetfirst(DeeObject *__restrict self);

/* seq_boundfirst */
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_boundfirst(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_boundfirst__with_callattr_first(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_boundfirst__with_callattr___seq_first__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_boundfirst__with_callobjectcache___seq_first__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__seq_boundfirst__with_callobjectcache___seq_first__(DeeTypeObject *tp_self, DeeObject *self);
#define default__seq_boundfirst__unsupported (*(int (DCALL *)(DeeObject *__restrict))&default__seq_boundfirst__empty)
#define default__seq_boundfirst__none (*(int (DCALL *)(DeeObject *__restrict))&_DeeNone_reti1_1)
#define default__seq_boundfirst__empty (*(int (DCALL *)(DeeObject *__restrict))&_DeeNone_reti0_1)
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_boundfirst__with__seq_operator_bounditem_index(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_boundfirst__with__seq_trygetfirst(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_boundfirst__with__set_operator_bool(DeeObject *__restrict self);

/* seq_delfirst */
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_delfirst(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_delfirst__with_callattr_first(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_delfirst__with_callattr___seq_first__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_delfirst__with_callobjectcache___seq_first__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__seq_delfirst__with_callobjectcache___seq_first__(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_delfirst__unsupported(DeeObject *__restrict self);
#define default__seq_delfirst__empty (*(int (DCALL *)(DeeObject *__restrict))&_DeeNone_reti0_1)
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_delfirst__with__seq_operator_delitem_index(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_delfirst__with__seq_trygetfirst__set_remove(DeeObject *__restrict self);

/* seq_setfirst */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_setfirst(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_setfirst__with_callattr_first(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_setfirst__with_callattr___seq_first__(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_setfirst__with_callobjectcache___seq_first__(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__seq_setfirst__with_callobjectcache___seq_first__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_setfirst__unsupported(DeeObject *self, DeeObject *value);
#define default__seq_setfirst__none (*(int (DCALL *)(DeeObject *, DeeObject *))&_DeeNone_reti0_2)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_setfirst__empty(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_setfirst__with__seq_operator_setitem_index(DeeObject *self, DeeObject *value);

/* seq_trygetlast */
#define default__seq_trygetlast__with_callattr_last (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&default__seq_trygetlast__with__seq_getlast)
#define default__seq_trygetlast__with_callattr___seq_last__ (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&default__seq_trygetlast__with__seq_getlast)
#define default__seq_trygetlast__with_callobjectcache___seq_last__ (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&default__seq_trygetlast__with__seq_getlast)
#define default__seq_trygetlast__unsupported (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&default__seq_getlast__unsupported)
#define default__seq_trygetlast__none (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&_DeeNone_NewRef1)
#define default__seq_trygetlast__empty (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&default__seq_trygetfirst__empty)
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_trygetlast__with__seq_getlast(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_trygetlast__with__seq_operator_size__and__operator_getitem_index_fast(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_trygetlast__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_trygetlast__with__seq_operator_sizeob__and__seq_operator_trygetitem(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_trygetlast__with__seq_operator_foreach(DeeObject *__restrict self);

/* seq_getlast */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_getlast(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_getlast__with_callattr_last(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_getlast__with_callattr___seq_last__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_getlast__with_callobjectcache___seq_last__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__seq_getlast__with_callobjectcache___seq_last__(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_getlast__unsupported(DeeObject *__restrict self);
#define default__seq_getlast__none (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&_DeeNone_NewRef1)
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_getlast__empty(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_getlast__with__seq_operator_size__and__seq_operator_getitem_index(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_getlast__with__seq_operator_sizeob__and__seq_operator_getitem(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_getlast__with__seq_trygetlast(DeeObject *__restrict self);

/* seq_boundlast */
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_boundlast(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_boundlast__with_callattr_last(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_boundlast__with_callattr___seq_last__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_boundlast__with_callobjectcache___seq_last__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__seq_boundlast__with_callobjectcache___seq_last__(DeeTypeObject *tp_self, DeeObject *self);
#define default__seq_boundlast__unsupported (*(int (DCALL *)(DeeObject *__restrict))&default__seq_boundlast__empty)
#define default__seq_boundlast__none (*(int (DCALL *)(DeeObject *__restrict))&_DeeNone_reti1_1)
#define default__seq_boundlast__empty (*(int (DCALL *)(DeeObject *__restrict))&_DeeNone_reti0_1)
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_boundlast__with__seq_operator_size__and__seq_operator_bounditem_index(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_boundlast__with__seq_operator_sizeob__and__seq_operator_bounditem(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_boundlast__with__seq_trygetlast(DeeObject *__restrict self);
#define default__seq_boundlast__with__set_operator_bool (*(int (DCALL *)(DeeObject *__restrict))&default__seq_boundfirst__with__set_operator_bool)

/* seq_dellast */
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_dellast(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_dellast__with_callattr_last(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_dellast__with_callattr___seq_last__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_dellast__with_callobjectcache___seq_last__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__seq_dellast__with_callobjectcache___seq_last__(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_dellast__unsupported(DeeObject *__restrict self);
#define default__seq_dellast__empty (*(int (DCALL *)(DeeObject *__restrict))&_DeeNone_reti0_1)
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_dellast__with__seq_operator_size__and__seq_operator_delitem_index(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_dellast__with__seq_operator_sizeob__and__seq_operator_delitem(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_dellast__with__seq_trygetlast__set_remove(DeeObject *__restrict self);

/* seq_setlast */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_setlast(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_setlast__with_callattr_last(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_setlast__with_callattr___seq_last__(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_setlast__with_callobjectcache___seq_last__(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__seq_setlast__with_callobjectcache___seq_last__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_setlast__unsupported(DeeObject *self, DeeObject *value);
#define default__seq_setlast__none (*(int (DCALL *)(DeeObject *, DeeObject *))&_DeeNone_reti0_2)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_setlast__empty(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_setlast__with__seq_operator_size__and__seq_operator_setitem_index(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_setlast__with__seq_operator_sizeob__and__seq_operator_setitem(DeeObject *self, DeeObject *value);

/* seq_cached */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_cached(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_cached__with_callattr_cached(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_cached__with_callattr___seq_cached__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_cached__with_callobjectcache___seq_cached__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__seq_cached__with_callobjectcache___seq_cached__(DeeTypeObject *tp_self, DeeObject *self);
#define default__seq_cached__unsupported (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&DeeObject_NewRef)
#define default__seq_cached__none (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&_DeeNone_NewRef1)
#define default__seq_cached__empty (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&DeeObject_NewRef)
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_cached__with__seq_operator_iter(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_cached__with__seq_operator_size__and__seq_operator_getitem_index(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_cached__with__seq_operator_sizeob__and__seq_operator_getitem(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_cached__with__seq_operator_getitem(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_cached__with__seq_enumerate(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_cached__with__seq_enumerate_index(DeeObject *__restrict self);

/* seq_frozen */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_frozen(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_frozen__with_callattr_frozen(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_frozen__with_callattr___seq_frozen__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_frozen__with_callobjectcache___seq_frozen__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__seq_frozen__with_callobjectcache___seq_frozen__(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_frozen__unsupported(DeeObject *__restrict self);
#define default__seq_frozen__none (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&_DeeNone_NewRef1)
#define default__seq_frozen__empty (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&DeeObject_NewRef)
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_frozen__with__seq_operator_foreach(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_frozen__with__seq_enumerate_index(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_frozen__with__set_frozen(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_frozen__with__map_frozen(DeeObject *__restrict self);

/* seq_any */
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_any__with_callattr_any(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_any__with_callattr___seq_any__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_any__with_callobjectcache___seq_any__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__seq_any__with_callobjectcache___seq_any__(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_any__unsupported(DeeObject *__restrict self);
#define default__seq_any__empty (*(int (DCALL *)(DeeObject *__restrict))&_DeeNone_reti0_1)
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_any__with__seq_operator_foreach(DeeObject *__restrict self);

/* seq_any_with_key */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_any_with_key__with_callattr_any(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_any_with_key__with_callattr___seq_any__(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_any_with_key__with_callobjectcache___seq_any__(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__seq_any_with_key__with_callobjectcache___seq_any__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_any_with_key__unsupported(DeeObject *self, DeeObject *key);
#define default__seq_any_with_key__empty (*(int (DCALL *)(DeeObject *, DeeObject *))&_DeeNone_reti0_2)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_any_with_key__with__seq_operator_foreach(DeeObject *self, DeeObject *key);

/* seq_any_with_range */
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_any_with_range__with_callattr_any(DeeObject *__restrict self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_any_with_range__with_callattr___seq_any__(DeeObject *__restrict self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_any_with_range__with_callobjectcache___seq_any__(DeeObject *__restrict self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__seq_any_with_range__with_callobjectcache___seq_any__(DeeTypeObject *tp_self, DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_any_with_range__unsupported(DeeObject *__restrict self, size_t start, size_t end);
#define default__seq_any_with_range__empty (*(int (DCALL *)(DeeObject *__restrict, size_t, size_t))&_DeeNone_reti0_3)
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_any_with_range__with__seqclass_map__and__seq_operator_bool__and__map_operator_size(DeeObject *__restrict self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_any_with_range__with__seq_enumerate_index(DeeObject *__restrict self, size_t start, size_t end);

/* seq_any_with_range_and_key */
INTDEF WUNUSED NONNULL((1, 4)) int DCALL default__seq_any_with_range_and_key__with_callattr_any(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL default__seq_any_with_range_and_key__with_callattr___seq_any__(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL default__seq_any_with_range_and_key__with_callobjectcache___seq_any__(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL tdefault__seq_any_with_range_and_key__with_callobjectcache___seq_any__(DeeTypeObject *tp_self, DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL default__seq_any_with_range_and_key__unsupported(DeeObject *self, size_t start, size_t end, DeeObject *key);
#define default__seq_any_with_range_and_key__empty (*(int (DCALL *)(DeeObject *, size_t, size_t, DeeObject *))&_DeeNone_reti0_4)
INTDEF WUNUSED NONNULL((1, 4)) int DCALL default__seq_any_with_range_and_key__with__seq_enumerate_index(DeeObject *self, size_t start, size_t end, DeeObject *key);

/* seq_all */
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_all__with_callattr_all(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_all__with_callattr___seq_all__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_all__with_callobjectcache___seq_all__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__seq_all__with_callobjectcache___seq_all__(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_all__unsupported(DeeObject *__restrict self);
#define default__seq_all__empty (*(int (DCALL *)(DeeObject *__restrict))&_DeeNone_reti1_1)
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_all__with__seq_operator_foreach(DeeObject *__restrict self);

/* seq_all_with_key */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_all_with_key__with_callattr_all(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_all_with_key__with_callattr___seq_all__(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_all_with_key__with_callobjectcache___seq_all__(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__seq_all_with_key__with_callobjectcache___seq_all__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_all_with_key__unsupported(DeeObject *self, DeeObject *key);
#define default__seq_all_with_key__empty (*(int (DCALL *)(DeeObject *, DeeObject *))&_DeeNone_reti1_2)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_all_with_key__with__seq_operator_foreach(DeeObject *self, DeeObject *key);

/* seq_all_with_range */
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_all_with_range__with_callattr_all(DeeObject *__restrict self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_all_with_range__with_callattr___seq_all__(DeeObject *__restrict self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_all_with_range__with_callobjectcache___seq_all__(DeeObject *__restrict self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__seq_all_with_range__with_callobjectcache___seq_all__(DeeTypeObject *tp_self, DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_all_with_range__unsupported(DeeObject *__restrict self, size_t start, size_t end);
#define default__seq_all_with_range__empty (*(int (DCALL *)(DeeObject *__restrict, size_t, size_t))&_DeeNone_reti1_3)
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_all_with_range__with__seq_enumerate_index(DeeObject *__restrict self, size_t start, size_t end);

/* seq_all_with_range_and_key */
INTDEF WUNUSED NONNULL((1, 4)) int DCALL default__seq_all_with_range_and_key__with_callattr_all(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL default__seq_all_with_range_and_key__with_callattr___seq_all__(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL default__seq_all_with_range_and_key__with_callobjectcache___seq_all__(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL tdefault__seq_all_with_range_and_key__with_callobjectcache___seq_all__(DeeTypeObject *tp_self, DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL default__seq_all_with_range_and_key__unsupported(DeeObject *self, size_t start, size_t end, DeeObject *key);
#define default__seq_all_with_range_and_key__empty (*(int (DCALL *)(DeeObject *, size_t, size_t, DeeObject *))&_DeeNone_reti1_4)
INTDEF WUNUSED NONNULL((1, 4)) int DCALL default__seq_all_with_range_and_key__with__seq_enumerate_index(DeeObject *self, size_t start, size_t end, DeeObject *key);

/* seq_parity */
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_parity__with_callattr_parity(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_parity__with_callattr___seq_parity__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_parity__with_callobjectcache___seq_parity__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__seq_parity__with_callobjectcache___seq_parity__(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_parity__unsupported(DeeObject *__restrict self);
#define default__seq_parity__empty (*(int (DCALL *)(DeeObject *__restrict))&_DeeNone_reti0_1)
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_parity__with__seq_count(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_parity__with__seq_operator_foreach(DeeObject *__restrict self);

/* seq_parity_with_key */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_parity_with_key__with_callattr_parity(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_parity_with_key__with_callattr___seq_parity__(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_parity_with_key__with_callobjectcache___seq_parity__(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__seq_parity_with_key__with_callobjectcache___seq_parity__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_parity_with_key__unsupported(DeeObject *self, DeeObject *key);
#define default__seq_parity_with_key__empty (*(int (DCALL *)(DeeObject *, DeeObject *))&_DeeNone_reti0_2)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_parity_with_key__with__seq_operator_foreach(DeeObject *self, DeeObject *key);

/* seq_parity_with_range */
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_parity_with_range__with_callattr_parity(DeeObject *__restrict self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_parity_with_range__with_callattr___seq_parity__(DeeObject *__restrict self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_parity_with_range__with_callobjectcache___seq_parity__(DeeObject *__restrict self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__seq_parity_with_range__with_callobjectcache___seq_parity__(DeeTypeObject *tp_self, DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_parity_with_range__unsupported(DeeObject *__restrict self, size_t start, size_t end);
#define default__seq_parity_with_range__empty (*(int (DCALL *)(DeeObject *__restrict, size_t, size_t))&_DeeNone_reti0_3)
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_parity_with_range__with__seq_count_with_range(DeeObject *__restrict self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_parity_with_range__with__seq_enumerate_index(DeeObject *__restrict self, size_t start, size_t end);

/* seq_parity_with_range_and_key */
INTDEF WUNUSED NONNULL((1, 4)) int DCALL default__seq_parity_with_range_and_key__with_callattr_parity(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL default__seq_parity_with_range_and_key__with_callattr___seq_parity__(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL default__seq_parity_with_range_and_key__with_callobjectcache___seq_parity__(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL tdefault__seq_parity_with_range_and_key__with_callobjectcache___seq_parity__(DeeTypeObject *tp_self, DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL default__seq_parity_with_range_and_key__unsupported(DeeObject *self, size_t start, size_t end, DeeObject *key);
#define default__seq_parity_with_range_and_key__empty (*(int (DCALL *)(DeeObject *, size_t, size_t, DeeObject *))&_DeeNone_reti0_4)
INTDEF WUNUSED NONNULL((1, 4)) int DCALL default__seq_parity_with_range_and_key__with__seq_enumerate_index(DeeObject *self, size_t start, size_t end, DeeObject *key);

/* seq_reduce */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_reduce__with_callattr_reduce(DeeObject *self, DeeObject *combine);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_reduce__with_callattr___seq_reduce__(DeeObject *self, DeeObject *combine);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_reduce__with_callobjectcache___seq_reduce__(DeeObject *self, DeeObject *combine);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__seq_reduce__with_callobjectcache___seq_reduce__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *combine);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_reduce__unsupported(DeeObject *self, DeeObject *combine);
#define default__seq_reduce__none (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&_DeeNone_NewRef2)
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_reduce__empty(DeeObject *self, DeeObject *combine);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_reduce__with__seq_operator_foreach(DeeObject *self, DeeObject *combine);

/* seq_reduce_with_init */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_reduce_with_init__with_callattr_reduce(DeeObject *self, DeeObject *combine, DeeObject *init);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_reduce_with_init__with_callattr___seq_reduce__(DeeObject *self, DeeObject *combine, DeeObject *init);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_reduce_with_init__with_callobjectcache___seq_reduce__(DeeObject *self, DeeObject *combine, DeeObject *init);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *DCALL tdefault__seq_reduce_with_init__with_callobjectcache___seq_reduce__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *combine, DeeObject *init);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_reduce_with_init__unsupported(DeeObject *self, DeeObject *combine, DeeObject *init);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_reduce_with_init__empty(DeeObject *self, DeeObject *combine, DeeObject *init);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_reduce_with_init__with__seq_operator_foreach(DeeObject *self, DeeObject *combine, DeeObject *init);

/* seq_reduce_with_range */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_reduce_with_range__with_callattr_reduce(DeeObject *self, DeeObject *combine, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_reduce_with_range__with_callattr___seq_reduce__(DeeObject *self, DeeObject *combine, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_reduce_with_range__with_callobjectcache___seq_reduce__(DeeObject *self, DeeObject *combine, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__seq_reduce_with_range__with_callobjectcache___seq_reduce__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *combine, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_reduce_with_range__unsupported(DeeObject *self, DeeObject *combine, size_t start, size_t end);
#define default__seq_reduce_with_range__none (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, size_t, size_t))&_DeeNone_NewRef4)
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_reduce_with_range__empty(DeeObject *self, DeeObject *combine, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_reduce_with_range__with__seq_enumerate_index(DeeObject *self, DeeObject *combine, size_t start, size_t end);

/* seq_reduce_with_range_and_init */
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL default__seq_reduce_with_range_and_init__with_callattr_reduce(DeeObject *self, DeeObject *combine, size_t start, size_t end, DeeObject *init);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL default__seq_reduce_with_range_and_init__with_callattr___seq_reduce__(DeeObject *self, DeeObject *combine, size_t start, size_t end, DeeObject *init);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL default__seq_reduce_with_range_and_init__with_callobjectcache___seq_reduce__(DeeObject *self, DeeObject *combine, size_t start, size_t end, DeeObject *init);
INTDEF WUNUSED NONNULL((1, 2, 3, 6)) DREF DeeObject *DCALL tdefault__seq_reduce_with_range_and_init__with_callobjectcache___seq_reduce__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *combine, size_t start, size_t end, DeeObject *init);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL default__seq_reduce_with_range_and_init__unsupported(DeeObject *self, DeeObject *combine, size_t start, size_t end, DeeObject *init);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL default__seq_reduce_with_range_and_init__empty(DeeObject *self, DeeObject *combine, size_t start, size_t end, DeeObject *init);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL default__seq_reduce_with_range_and_init__with__seq_enumerate_index(DeeObject *self, DeeObject *combine, size_t start, size_t end, DeeObject *init);

/* seq_min */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_min__with_callattr_min(DeeObject *self, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_min__with_callattr___seq_min__(DeeObject *self, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_min__with_callobjectcache___seq_min__(DeeObject *self, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__seq_min__with_callobjectcache___seq_min__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_min__unsupported(DeeObject *self, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_min__empty(DeeObject *self, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_min__with__seq_operator_foreach(DeeObject *self, DeeObject *def);

/* seq_min_with_key */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_min_with_key__with_callattr_min(DeeObject *self, DeeObject *def, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_min_with_key__with_callattr___seq_min__(DeeObject *self, DeeObject *def, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_min_with_key__with_callobjectcache___seq_min__(DeeObject *self, DeeObject *def, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *DCALL tdefault__seq_min_with_key__with_callobjectcache___seq_min__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *def, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_min_with_key__unsupported(DeeObject *self, DeeObject *def, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_min_with_key__empty(DeeObject *self, DeeObject *def, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_min_with_key__with__seq_operator_foreach(DeeObject *self, DeeObject *def, DeeObject *key);

/* seq_min_with_range */
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL default__seq_min_with_range__with_callattr_min(DeeObject *self, size_t start, size_t end, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL default__seq_min_with_range__with_callattr___seq_min__(DeeObject *self, size_t start, size_t end, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL default__seq_min_with_range__with_callobjectcache___seq_min__(DeeObject *self, size_t start, size_t end, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL tdefault__seq_min_with_range__with_callobjectcache___seq_min__(DeeTypeObject *tp_self, DeeObject *self, size_t start, size_t end, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL default__seq_min_with_range__unsupported(DeeObject *self, size_t start, size_t end, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL default__seq_min_with_range__empty(DeeObject *self, size_t start, size_t end, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL default__seq_min_with_range__with__seq_enumerate_index(DeeObject *self, size_t start, size_t end, DeeObject *def);

/* seq_min_with_range_and_key */
INTDEF WUNUSED NONNULL((1, 4, 5)) DREF DeeObject *DCALL default__seq_min_with_range_and_key__with_callattr_min(DeeObject *self, size_t start, size_t end, DeeObject *def, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4, 5)) DREF DeeObject *DCALL default__seq_min_with_range_and_key__with_callattr___seq_min__(DeeObject *self, size_t start, size_t end, DeeObject *def, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4, 5)) DREF DeeObject *DCALL default__seq_min_with_range_and_key__with_callobjectcache___seq_min__(DeeObject *self, size_t start, size_t end, DeeObject *def, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5, 6)) DREF DeeObject *DCALL tdefault__seq_min_with_range_and_key__with_callobjectcache___seq_min__(DeeTypeObject *tp_self, DeeObject *self, size_t start, size_t end, DeeObject *def, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4, 5)) DREF DeeObject *DCALL default__seq_min_with_range_and_key__unsupported(DeeObject *self, size_t start, size_t end, DeeObject *def, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4, 5)) DREF DeeObject *DCALL default__seq_min_with_range_and_key__empty(DeeObject *self, size_t start, size_t end, DeeObject *def, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4, 5)) DREF DeeObject *DCALL default__seq_min_with_range_and_key__with__seq_enumerate_index(DeeObject *self, size_t start, size_t end, DeeObject *def, DeeObject *key);

/* seq_max */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_max__with_callattr_max(DeeObject *self, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_max__with_callattr___seq_max__(DeeObject *self, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_max__with_callobjectcache___seq_max__(DeeObject *self, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__seq_max__with_callobjectcache___seq_max__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_max__unsupported(DeeObject *self, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_max__empty(DeeObject *self, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_max__with__seq_operator_foreach(DeeObject *self, DeeObject *def);

/* seq_max_with_key */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_max_with_key__with_callattr_max(DeeObject *self, DeeObject *def, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_max_with_key__with_callattr___seq_max__(DeeObject *self, DeeObject *def, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_max_with_key__with_callobjectcache___seq_max__(DeeObject *self, DeeObject *def, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *DCALL tdefault__seq_max_with_key__with_callobjectcache___seq_max__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *def, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_max_with_key__unsupported(DeeObject *self, DeeObject *def, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_max_with_key__empty(DeeObject *self, DeeObject *def, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_max_with_key__with__seq_operator_foreach(DeeObject *self, DeeObject *def, DeeObject *key);

/* seq_max_with_range */
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL default__seq_max_with_range__with_callattr_max(DeeObject *self, size_t start, size_t end, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL default__seq_max_with_range__with_callattr___seq_max__(DeeObject *self, size_t start, size_t end, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL default__seq_max_with_range__with_callobjectcache___seq_max__(DeeObject *self, size_t start, size_t end, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL tdefault__seq_max_with_range__with_callobjectcache___seq_max__(DeeTypeObject *tp_self, DeeObject *self, size_t start, size_t end, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL default__seq_max_with_range__unsupported(DeeObject *self, size_t start, size_t end, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL default__seq_max_with_range__empty(DeeObject *self, size_t start, size_t end, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL default__seq_max_with_range__with__seq_enumerate_index(DeeObject *self, size_t start, size_t end, DeeObject *def);

/* seq_max_with_range_and_key */
INTDEF WUNUSED NONNULL((1, 4, 5)) DREF DeeObject *DCALL default__seq_max_with_range_and_key__with_callattr_max(DeeObject *self, size_t start, size_t end, DeeObject *def, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4, 5)) DREF DeeObject *DCALL default__seq_max_with_range_and_key__with_callattr___seq_max__(DeeObject *self, size_t start, size_t end, DeeObject *def, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4, 5)) DREF DeeObject *DCALL default__seq_max_with_range_and_key__with_callobjectcache___seq_max__(DeeObject *self, size_t start, size_t end, DeeObject *def, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5, 6)) DREF DeeObject *DCALL tdefault__seq_max_with_range_and_key__with_callobjectcache___seq_max__(DeeTypeObject *tp_self, DeeObject *self, size_t start, size_t end, DeeObject *def, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4, 5)) DREF DeeObject *DCALL default__seq_max_with_range_and_key__unsupported(DeeObject *self, size_t start, size_t end, DeeObject *def, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4, 5)) DREF DeeObject *DCALL default__seq_max_with_range_and_key__empty(DeeObject *self, size_t start, size_t end, DeeObject *def, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4, 5)) DREF DeeObject *DCALL default__seq_max_with_range_and_key__with__seq_enumerate_index(DeeObject *self, size_t start, size_t end, DeeObject *def, DeeObject *key);

/* seq_sum */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_sum__with_callattr_sum(DeeObject *self, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_sum__with_callattr___seq_sum__(DeeObject *self, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_sum__with_callobjectcache___seq_sum__(DeeObject *self, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__seq_sum__with_callobjectcache___seq_sum__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_sum__unsupported(DeeObject *self, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_sum__empty(DeeObject *self, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_sum__with__seq_operator_foreach(DeeObject *self, DeeObject *def);

/* seq_sum_with_key */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_sum_with_key__with_callattr_sum(DeeObject *self, DeeObject *def, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_sum_with_key__with_callattr___seq_sum__(DeeObject *self, DeeObject *def, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_sum_with_key__with_callobjectcache___seq_sum__(DeeObject *self, DeeObject *def, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *DCALL tdefault__seq_sum_with_key__with_callobjectcache___seq_sum__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *def, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_sum_with_key__unsupported(DeeObject *self, DeeObject *def, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_sum_with_key__empty(DeeObject *self, DeeObject *def, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_sum_with_key__with__seq_operator_foreach(DeeObject *self, DeeObject *def, DeeObject *key);

/* seq_sum_with_range */
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL default__seq_sum_with_range__with_callattr_sum(DeeObject *self, size_t start, size_t end, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL default__seq_sum_with_range__with_callattr___seq_sum__(DeeObject *self, size_t start, size_t end, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL default__seq_sum_with_range__with_callobjectcache___seq_sum__(DeeObject *self, size_t start, size_t end, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL tdefault__seq_sum_with_range__with_callobjectcache___seq_sum__(DeeTypeObject *tp_self, DeeObject *self, size_t start, size_t end, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL default__seq_sum_with_range__unsupported(DeeObject *self, size_t start, size_t end, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL default__seq_sum_with_range__empty(DeeObject *self, size_t start, size_t end, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL default__seq_sum_with_range__with__seq_enumerate_index(DeeObject *self, size_t start, size_t end, DeeObject *def);

/* seq_sum_with_range_and_key */
INTDEF WUNUSED NONNULL((1, 4, 5)) DREF DeeObject *DCALL default__seq_sum_with_range_and_key__with_callattr_sum(DeeObject *self, size_t start, size_t end, DeeObject *def, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4, 5)) DREF DeeObject *DCALL default__seq_sum_with_range_and_key__with_callattr___seq_sum__(DeeObject *self, size_t start, size_t end, DeeObject *def, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4, 5)) DREF DeeObject *DCALL default__seq_sum_with_range_and_key__with_callobjectcache___seq_sum__(DeeObject *self, size_t start, size_t end, DeeObject *def, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5, 6)) DREF DeeObject *DCALL tdefault__seq_sum_with_range_and_key__with_callobjectcache___seq_sum__(DeeTypeObject *tp_self, DeeObject *self, size_t start, size_t end, DeeObject *def, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4, 5)) DREF DeeObject *DCALL default__seq_sum_with_range_and_key__unsupported(DeeObject *self, size_t start, size_t end, DeeObject *def, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4, 5)) DREF DeeObject *DCALL default__seq_sum_with_range_and_key__empty(DeeObject *self, size_t start, size_t end, DeeObject *def, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4, 5)) DREF DeeObject *DCALL default__seq_sum_with_range_and_key__with__seq_enumerate_index(DeeObject *self, size_t start, size_t end, DeeObject *def, DeeObject *key);

/* seq_count */
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_count__with_callattr_count(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_count__with_callattr___seq_count__(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_count__with_callobjectcache___seq_count__(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2, 3)) size_t DCALL tdefault__seq_count__with_callobjectcache___seq_count__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_count__unsupported(DeeObject *self, DeeObject *item);
#define default__seq_count__empty (*(size_t (DCALL *)(DeeObject *, DeeObject *))&_DeeNone_rets0_2)
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_count__with__set_operator_contains(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_count__with__seq_operator_foreach(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_count__with__seq_find(DeeObject *self, DeeObject *item);

/* seq_count_with_key */
INTDEF WUNUSED NONNULL((1, 2, 3)) size_t DCALL default__seq_count_with_key__with_callattr_count(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) size_t DCALL default__seq_count_with_key__with_callattr___seq_count__(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) size_t DCALL default__seq_count_with_key__with_callobjectcache___seq_count__(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) size_t DCALL tdefault__seq_count_with_key__with_callobjectcache___seq_count__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) size_t DCALL default__seq_count_with_key__unsupported(DeeObject *self, DeeObject *item, DeeObject *key);
#define default__seq_count_with_key__empty (*(size_t (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&_DeeNone_rets0_3)
INTDEF WUNUSED NONNULL((1, 2, 3)) size_t DCALL default__seq_count_with_key__with__seq_operator_foreach(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) size_t DCALL default__seq_count_with_key__with__seq_find_with_key(DeeObject *self, DeeObject *item, DeeObject *key);

/* seq_count_with_range */
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_count_with_range__with_callattr_count(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_count_with_range__with_callattr___seq_count__(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_count_with_range__with_callobjectcache___seq_count__(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 3)) size_t DCALL tdefault__seq_count_with_range__with_callobjectcache___seq_count__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_count_with_range__unsupported(DeeObject *self, DeeObject *item, size_t start, size_t end);
#define default__seq_count_with_range__empty (*(size_t (DCALL *)(DeeObject *, DeeObject *, size_t, size_t))&_DeeNone_rets0_4)
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_count_with_range__with__seq_enumerate_index(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_count_with_range__with__seq_find(DeeObject *self, DeeObject *item, size_t start, size_t end);

/* seq_count_with_range_and_key */
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL default__seq_count_with_range_and_key__with_callattr_count(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL default__seq_count_with_range_and_key__with_callattr___seq_count__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL default__seq_count_with_range_and_key__with_callobjectcache___seq_count__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3, 6)) size_t DCALL tdefault__seq_count_with_range_and_key__with_callobjectcache___seq_count__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL default__seq_count_with_range_and_key__unsupported(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
#define default__seq_count_with_range_and_key__empty (*(size_t (DCALL *)(DeeObject *, DeeObject *, size_t, size_t, DeeObject *))&_DeeNone_rets0_5)
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL default__seq_count_with_range_and_key__with__seq_enumerate_index(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL default__seq_count_with_range_and_key__with__seq_find_with_key(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);

/* seq_contains */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_contains__with_callattr_contains(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_contains__with_callattr___seq_contains__(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_contains__with_callobjectcache___seq_contains__(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__seq_contains__with_callobjectcache___seq_contains__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_contains__unsupported(DeeObject *self, DeeObject *item);
#define default__seq_contains__empty (*(int (DCALL *)(DeeObject *, DeeObject *))&_DeeNone_reti0_2)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_contains__with__seq_operator_contains(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_contains__with__map_operator_trygetitem(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_contains__with__seq_operator_foreach(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_contains__with__seq_find(DeeObject *self, DeeObject *item);

/* seq_contains_with_key */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_contains_with_key__with_callattr_contains(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_contains_with_key__with_callattr___seq_contains__(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_contains_with_key__with_callobjectcache___seq_contains__(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL tdefault__seq_contains_with_key__with_callobjectcache___seq_contains__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_contains_with_key__unsupported(DeeObject *self, DeeObject *item, DeeObject *key);
#define default__seq_contains_with_key__empty (*(int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&_DeeNone_reti0_3)
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_contains_with_key__with__seq_operator_foreach(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_contains_with_key__with__seq_find_with_key(DeeObject *self, DeeObject *item, DeeObject *key);

/* seq_contains_with_range */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_contains_with_range__with_callattr_contains(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_contains_with_range__with_callattr___seq_contains__(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_contains_with_range__with_callobjectcache___seq_contains__(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__seq_contains_with_range__with_callobjectcache___seq_contains__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_contains_with_range__unsupported(DeeObject *self, DeeObject *item, size_t start, size_t end);
#define default__seq_contains_with_range__empty (*(int (DCALL *)(DeeObject *, DeeObject *, size_t, size_t))&_DeeNone_reti0_4)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_contains_with_range__with__seq_enumerate_index(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_contains_with_range__with__seq_find(DeeObject *self, DeeObject *item, size_t start, size_t end);

/* seq_contains_with_range_and_key */
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL default__seq_contains_with_range_and_key__with_callattr_contains(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL default__seq_contains_with_range_and_key__with_callattr___seq_contains__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL default__seq_contains_with_range_and_key__with_callobjectcache___seq_contains__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3, 6)) int DCALL tdefault__seq_contains_with_range_and_key__with_callobjectcache___seq_contains__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL default__seq_contains_with_range_and_key__unsupported(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
#define default__seq_contains_with_range_and_key__empty (*(int (DCALL *)(DeeObject *, DeeObject *, size_t, size_t, DeeObject *))&_DeeNone_reti0_5)
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL default__seq_contains_with_range_and_key__with__seq_enumerate_index(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL default__seq_contains_with_range_and_key__with__seq_find_with_key(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);

/* seq_operator_contains */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_contains(DeeObject *self, DeeObject *item);
#define default__seq_operator_contains__with_callattr_contains (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&default__seq_operator_contains__with__seq_contains)
#define default__seq_operator_contains__with_callattr___seq_contains__ (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&default__seq_operator_contains__with__seq_contains)
#define default__seq_operator_contains__with_callobjectcache___seq_contains__ (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&default__seq_operator_contains__with__seq_contains)
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_contains__unsupported(DeeObject *self, DeeObject *item);
#define default__seq_operator_contains__none (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&_DeeNone_NewRef2)
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_contains__empty(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_contains__with__seq_contains(DeeObject *self, DeeObject *item);

/* seq_locate */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_locate__with_callattr_locate(DeeObject *self, DeeObject *match, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_locate__with_callattr___seq_locate__(DeeObject *self, DeeObject *match, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_locate__with_callobjectcache___seq_locate__(DeeObject *self, DeeObject *match, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *DCALL tdefault__seq_locate__with_callobjectcache___seq_locate__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *match, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_locate__unsupported(DeeObject *self, DeeObject *match, DeeObject *def);
#define default__seq_locate__none (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *))&_DeeNone_NewRef3)
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_locate__empty(DeeObject *self, DeeObject *match, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_locate__with__seq_operator_foreach(DeeObject *self, DeeObject *match, DeeObject *def);

/* seq_locate_with_range */
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL default__seq_locate_with_range__with_callattr_locate(DeeObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL default__seq_locate_with_range__with_callattr___seq_locate__(DeeObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL default__seq_locate_with_range__with_callobjectcache___seq_locate__(DeeObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 3, 6)) DREF DeeObject *DCALL tdefault__seq_locate_with_range__with_callobjectcache___seq_locate__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL default__seq_locate_with_range__unsupported(DeeObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def);
#define default__seq_locate_with_range__none (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, size_t, size_t, DeeObject *))&_DeeNone_NewRef5)
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL default__seq_locate_with_range__empty(DeeObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL default__seq_locate_with_range__with__seq_enumerate_index(DeeObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def);

/* seq_rlocate */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_rlocate__with_callattr_rlocate(DeeObject *self, DeeObject *match, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_rlocate__with_callattr___seq_rlocate__(DeeObject *self, DeeObject *match, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_rlocate__with_callobjectcache___seq_rlocate__(DeeObject *self, DeeObject *match, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *DCALL tdefault__seq_rlocate__with_callobjectcache___seq_rlocate__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *match, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_rlocate__unsupported(DeeObject *self, DeeObject *match, DeeObject *def);
#define default__seq_rlocate__none (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *))&_DeeNone_NewRef3)
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_rlocate__empty(DeeObject *self, DeeObject *match, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_rlocate__with__seq_foreach_reverse(DeeObject *self, DeeObject *match, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_rlocate__with__seq_operator_foreach(DeeObject *self, DeeObject *match, DeeObject *def);

/* seq_rlocate_with_range */
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL default__seq_rlocate_with_range__with_callattr_rlocate(DeeObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL default__seq_rlocate_with_range__with_callattr___seq_rlocate__(DeeObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL default__seq_rlocate_with_range__with_callobjectcache___seq_rlocate__(DeeObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 3, 6)) DREF DeeObject *DCALL tdefault__seq_rlocate_with_range__with_callobjectcache___seq_rlocate__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL default__seq_rlocate_with_range__unsupported(DeeObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def);
#define default__seq_rlocate_with_range__none (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, size_t, size_t, DeeObject *))&_DeeNone_NewRef5)
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL default__seq_rlocate_with_range__empty(DeeObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL default__seq_rlocate_with_range__with__seq_enumerate_index_reverse(DeeObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL default__seq_rlocate_with_range__with__seq_enumerate_index(DeeObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def);

/* seq_startswith */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_startswith__with_callattr_startswith(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_startswith__with_callattr___seq_startswith__(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_startswith__with_callobjectcache___seq_startswith__(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__seq_startswith__with_callobjectcache___seq_startswith__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_startswith__unsupported(DeeObject *self, DeeObject *item);
#define default__seq_startswith__empty (*(int (DCALL *)(DeeObject *, DeeObject *))&_DeeNone_reti0_2)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_startswith__with__seq_trygetfirst(DeeObject *self, DeeObject *item);

/* seq_startswith_with_key */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_startswith_with_key__with_callattr_startswith(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_startswith_with_key__with_callattr___seq_startswith__(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_startswith_with_key__with_callobjectcache___seq_startswith__(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL tdefault__seq_startswith_with_key__with_callobjectcache___seq_startswith__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_startswith_with_key__unsupported(DeeObject *self, DeeObject *item, DeeObject *key);
#define default__seq_startswith_with_key__empty (*(int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&_DeeNone_reti0_3)
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_startswith_with_key__with__seq_trygetfirst(DeeObject *self, DeeObject *item, DeeObject *key);

/* seq_startswith_with_range */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_startswith_with_range__with_callattr_startswith(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_startswith_with_range__with_callattr___seq_startswith__(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_startswith_with_range__with_callobjectcache___seq_startswith__(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__seq_startswith_with_range__with_callobjectcache___seq_startswith__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_startswith_with_range__unsupported(DeeObject *self, DeeObject *item, size_t start, size_t end);
#define default__seq_startswith_with_range__empty (*(int (DCALL *)(DeeObject *, DeeObject *, size_t, size_t))&_DeeNone_reti0_4)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_startswith_with_range__with__seq_operator_trygetitem_index(DeeObject *self, DeeObject *item, size_t start, size_t end);

/* seq_startswith_with_range_and_key */
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL default__seq_startswith_with_range_and_key__with_callattr_startswith(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL default__seq_startswith_with_range_and_key__with_callattr___seq_startswith__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL default__seq_startswith_with_range_and_key__with_callobjectcache___seq_startswith__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3, 6)) int DCALL tdefault__seq_startswith_with_range_and_key__with_callobjectcache___seq_startswith__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL default__seq_startswith_with_range_and_key__unsupported(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
#define default__seq_startswith_with_range_and_key__empty (*(int (DCALL *)(DeeObject *, DeeObject *, size_t, size_t, DeeObject *))&_DeeNone_reti0_5)
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL default__seq_startswith_with_range_and_key__with__seq_operator_trygetitem_index(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);

/* seq_endswith */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_endswith__with_callattr_endswith(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_endswith__with_callattr___seq_endswith__(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_endswith__with_callobjectcache___seq_endswith__(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__seq_endswith__with_callobjectcache___seq_endswith__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_endswith__unsupported(DeeObject *self, DeeObject *item);
#define default__seq_endswith__empty (*(int (DCALL *)(DeeObject *, DeeObject *))&_DeeNone_reti0_2)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_endswith__with__seq_trygetlast(DeeObject *self, DeeObject *item);

/* seq_endswith_with_key */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_endswith_with_key__with_callattr_endswith(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_endswith_with_key__with_callattr___seq_endswith__(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_endswith_with_key__with_callobjectcache___seq_endswith__(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL tdefault__seq_endswith_with_key__with_callobjectcache___seq_endswith__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_endswith_with_key__unsupported(DeeObject *self, DeeObject *item, DeeObject *key);
#define default__seq_endswith_with_key__empty (*(int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&_DeeNone_reti0_3)
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_endswith_with_key__with__seq_trygetlast(DeeObject *self, DeeObject *item, DeeObject *key);

/* seq_endswith_with_range */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_endswith_with_range__with_callattr_endswith(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_endswith_with_range__with_callattr___seq_endswith__(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_endswith_with_range__with_callobjectcache___seq_endswith__(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__seq_endswith_with_range__with_callobjectcache___seq_endswith__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_endswith_with_range__unsupported(DeeObject *self, DeeObject *item, size_t start, size_t end);
#define default__seq_endswith_with_range__empty (*(int (DCALL *)(DeeObject *, DeeObject *, size_t, size_t))&_DeeNone_reti0_4)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_endswith_with_range__with__seq_operator_size__and__operator_trygetitem_index(DeeObject *self, DeeObject *item, size_t start, size_t end);

/* seq_endswith_with_range_and_key */
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL default__seq_endswith_with_range_and_key__with_callattr_endswith(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL default__seq_endswith_with_range_and_key__with_callattr___seq_endswith__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL default__seq_endswith_with_range_and_key__with_callobjectcache___seq_endswith__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3, 6)) int DCALL tdefault__seq_endswith_with_range_and_key__with_callobjectcache___seq_endswith__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL default__seq_endswith_with_range_and_key__unsupported(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
#define default__seq_endswith_with_range_and_key__empty (*(int (DCALL *)(DeeObject *, DeeObject *, size_t, size_t, DeeObject *))&_DeeNone_reti0_5)
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL default__seq_endswith_with_range_and_key__with__seq_operator_size__and__operator_trygetitem_index(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);

/* seq_find */
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_find__with_callattr_find(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_find__with_callattr___seq_find__(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_find__with_callobjectcache___seq_find__(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 3)) size_t DCALL tdefault__seq_find__with_callobjectcache___seq_find__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_find__unsupported(DeeObject *self, DeeObject *item, size_t start, size_t end);
#define default__seq_find__empty (*(size_t (DCALL *)(DeeObject *, DeeObject *, size_t, size_t))&_DeeNone_retsm1_4)
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_find__with__seq_enumerate_index(DeeObject *self, DeeObject *item, size_t start, size_t end);

/* seq_find_with_key */
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL default__seq_find_with_key__with_callattr_find(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL default__seq_find_with_key__with_callattr___seq_find__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL default__seq_find_with_key__with_callobjectcache___seq_find__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3, 6)) size_t DCALL tdefault__seq_find_with_key__with_callobjectcache___seq_find__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL default__seq_find_with_key__unsupported(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
#define default__seq_find_with_key__empty (*(size_t (DCALL *)(DeeObject *, DeeObject *, size_t, size_t, DeeObject *))&_DeeNone_retsm1_5)
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL default__seq_find_with_key__with__seq_enumerate_index(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);

/* seq_rfind */
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_rfind__with_callattr_rfind(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_rfind__with_callattr___seq_rfind__(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_rfind__with_callobjectcache___seq_rfind__(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 3)) size_t DCALL tdefault__seq_rfind__with_callobjectcache___seq_rfind__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_rfind__unsupported(DeeObject *self, DeeObject *item, size_t start, size_t end);
#define default__seq_rfind__empty (*(size_t (DCALL *)(DeeObject *, DeeObject *, size_t, size_t))&default__seq_find__empty)
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_rfind__with__seq_enumerate_index_reverse(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_rfind__with__seq_enumerate_index(DeeObject *self, DeeObject *item, size_t start, size_t end);

/* seq_rfind_with_key */
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL default__seq_rfind_with_key__with_callattr_rfind(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL default__seq_rfind_with_key__with_callattr___seq_rfind__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL default__seq_rfind_with_key__with_callobjectcache___seq_rfind__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3, 6)) size_t DCALL tdefault__seq_rfind_with_key__with_callobjectcache___seq_rfind__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL default__seq_rfind_with_key__unsupported(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
#define default__seq_rfind_with_key__empty (*(size_t (DCALL *)(DeeObject *, DeeObject *, size_t, size_t, DeeObject *))&default__seq_find_with_key__empty)
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL default__seq_rfind_with_key__with__seq_enumerate_index_reverse(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL default__seq_rfind_with_key__with__seq_enumerate_index(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);

/* seq_erase */
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_erase__with_callattr_erase(DeeObject *__restrict self, size_t index, size_t count);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_erase__with_callattr___seq_erase__(DeeObject *__restrict self, size_t index, size_t count);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_erase__with_callobjectcache___seq_erase__(DeeObject *__restrict self, size_t index, size_t count);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__seq_erase__with_callobjectcache___seq_erase__(DeeTypeObject *tp_self, DeeObject *self, size_t index, size_t count);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_erase__unsupported(DeeObject *__restrict self, size_t index, size_t count);
#define default__seq_erase__empty (*(int (DCALL *)(DeeObject *__restrict, size_t, size_t))&_DeeNone_reti0_3)
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_erase__with__seq_operator_delrange_index(DeeObject *__restrict self, size_t index, size_t count);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_erase__with__seq_pop(DeeObject *__restrict self, size_t index, size_t count);

/* seq_insert */
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_insert__with_callattr_insert(DeeObject *self, size_t index, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_insert__with_callattr___seq_insert__(DeeObject *self, size_t index, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_insert__with_callobjectcache___seq_insert__(DeeObject *self, size_t index, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2, 4)) int DCALL tdefault__seq_insert__with_callobjectcache___seq_insert__(DeeTypeObject *tp_self, DeeObject *self, size_t index, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_insert__unsupported(DeeObject *self, size_t index, DeeObject *item);
#define default__seq_insert__none (*(int (DCALL *)(DeeObject *, size_t, DeeObject *))&_DeeNone_reti0_3)
#define default__seq_insert__empty (*(int (DCALL *)(DeeObject *, size_t, DeeObject *))&default__seq_insert__unsupported)
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_insert__with__seq_insertall(DeeObject *self, size_t index, DeeObject *item);

/* seq_insertall */
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_insertall__with_callattr_insertall(DeeObject *self, size_t index, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_insertall__with_callattr___seq_insertall__(DeeObject *self, size_t index, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_insertall__with_callobjectcache___seq_insertall__(DeeObject *self, size_t index, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2, 4)) int DCALL tdefault__seq_insertall__with_callobjectcache___seq_insertall__(DeeTypeObject *tp_self, DeeObject *self, size_t index, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_insertall__unsupported(DeeObject *self, size_t index, DeeObject *items);
#define default__seq_insertall__none (*(int (DCALL *)(DeeObject *, size_t, DeeObject *))&_DeeNone_reti0_3)
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_insertall__empty(DeeObject *self, size_t index, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_insertall__with__seq_operator_setrange_index(DeeObject *self, size_t index, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_insertall__with__seq_insert(DeeObject *self, size_t index, DeeObject *items);

/* seq_pushfront */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_pushfront__with_callattr_pushfront(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_pushfront__with_callattr___seq_pushfront__(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_pushfront__with_callobjectcache___seq_pushfront__(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__seq_pushfront__with_callobjectcache___seq_pushfront__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_pushfront__unsupported(DeeObject *self, DeeObject *item);
#define default__seq_pushfront__none (*(int (DCALL *)(DeeObject *, DeeObject *))&_DeeNone_reti0_2)
#define default__seq_pushfront__empty (*(int (DCALL *)(DeeObject *, DeeObject *))&default__seq_pushfront__unsupported)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_pushfront__with__seq_insert(DeeObject *self, DeeObject *item);

/* seq_append */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_append__with_callattr_append(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_append__with_callattr_pushback(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_append__with_callattr___seq_append__(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_append__with_callobjectcache___seq_append__(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__seq_append__with_callobjectcache___seq_append__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_append__unsupported(DeeObject *self, DeeObject *item);
#define default__seq_append__none (*(int (DCALL *)(DeeObject *, DeeObject *))&_DeeNone_reti0_2)
#define default__seq_append__empty (*(int (DCALL *)(DeeObject *, DeeObject *))&default__seq_append__unsupported)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_append__with__seq_extend(DeeObject *self, DeeObject *item);

/* seq_extend */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_extend__with_callattr_extend(DeeObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_extend__with_callattr___seq_extend__(DeeObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_extend__with_callobjectcache___seq_extend__(DeeObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__seq_extend__with_callobjectcache___seq_extend__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_extend__unsupported(DeeObject *self, DeeObject *items);
#define default__seq_extend__none (*(int (DCALL *)(DeeObject *, DeeObject *))&_DeeNone_reti0_2)
#define default__seq_extend__empty (*(int (DCALL *)(DeeObject *, DeeObject *))&default__seq_extend__unsupported)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_extend__with__seq_operator_size__and__seq_operator_setrange_index(DeeObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_extend__with__seq_operator_size__and__seq_insertall(DeeObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_extend__with__seq_append(DeeObject *self, DeeObject *items);

/* seq_xchitem_index */
INTDEF WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL default__seq_xchitem_index__with_callattr_xchitem(DeeObject *self, size_t index, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL default__seq_xchitem_index__with_callattr___seq_xchitem__(DeeObject *self, size_t index, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL default__seq_xchitem_index__with_callobjectcache___seq_xchitem__(DeeObject *self, size_t index, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2, 4)) DREF DeeObject *DCALL tdefault__seq_xchitem_index__with_callobjectcache___seq_xchitem__(DeeTypeObject *tp_self, DeeObject *self, size_t index, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL default__seq_xchitem_index__unsupported(DeeObject *self, size_t index, DeeObject *item);
#define default__seq_xchitem_index__none (*(DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *))&_DeeNone_NewRef3)
#define default__seq_xchitem_index__empty (*(DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *))&default__seq_xchitem_index__unsupported)
INTDEF WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL default__seq_xchitem_index__with__seq_operator_getitem_index__and__seq_operator_setitem_index(DeeObject *self, size_t index, DeeObject *item);

/* seq_clear */
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_clear__with_callattr_clear(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_clear__with_callattr___seq_clear__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_clear__with_callobjectcache___seq_clear__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__seq_clear__with_callobjectcache___seq_clear__(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_clear__unsupported(DeeObject *__restrict self);
#define default__seq_clear__empty (*(int (DCALL *)(DeeObject *__restrict))&_DeeNone_reti0_1)
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_clear__with__seq_operator_delrange(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_clear__with__seq_operator_delrange_index_n(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_clear__with__seq_erase(DeeObject *__restrict self);

/* seq_pop */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_pop__with_callattr_pop(DeeObject *self, Dee_ssize_t index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_pop__with_callattr___seq_pop__(DeeObject *self, Dee_ssize_t index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_pop__with_callobjectcache___seq_pop__(DeeObject *self, Dee_ssize_t index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__seq_pop__with_callobjectcache___seq_pop__(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_pop__unsupported(DeeObject *self, Dee_ssize_t index);
#define default__seq_pop__none (*(DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t))&_DeeNone_NewRef2)
#define default__seq_pop__empty (*(DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t))&default__seq_pop__unsupported)
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_pop__with__seq_operator_size__and__seq_operator_getitem_index__and__seq_erase(DeeObject *self, Dee_ssize_t index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_pop__with__seq_operator_size__and__seq_operator_getitem_index__and__seq_operator_delitem_index(DeeObject *self, Dee_ssize_t index);

/* seq_remove */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_remove__with_callattr_remove(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_remove__with_callattr___seq_remove__(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_remove__with_callobjectcache___seq_remove__(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__seq_remove__with_callobjectcache___seq_remove__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_remove__unsupported(DeeObject *self, DeeObject *item, size_t start, size_t end);
#define default__seq_remove__empty (*(int (DCALL *)(DeeObject *, DeeObject *, size_t, size_t))&_DeeNone_reti0_4)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_remove__with__seq_removeall(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_remove__with__seq_enumerate_index__and__seq_operator_delitem_index(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_remove__with__seq_find__and__seq_operator_delitem_index(DeeObject *self, DeeObject *item, size_t start, size_t end);

/* seq_remove_with_key */
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL default__seq_remove_with_key__with_callattr_remove(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL default__seq_remove_with_key__with_callattr___seq_remove__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL default__seq_remove_with_key__with_callobjectcache___seq_remove__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3, 6)) int DCALL tdefault__seq_remove_with_key__with_callobjectcache___seq_remove__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL default__seq_remove_with_key__unsupported(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
#define default__seq_remove_with_key__empty (*(int (DCALL *)(DeeObject *, DeeObject *, size_t, size_t, DeeObject *))&_DeeNone_reti0_5)
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL default__seq_remove_with_key__with__seq_removeall(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL default__seq_remove_with_key__with__seq_enumerate_index__and__seq_operator_delitem_index(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL default__seq_remove_with_key__with__seq_find_with_key__and__seq_operator_delitem_index(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);

/* seq_rremove */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_rremove__with_callattr_rremove(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_rremove__with_callattr___seq_rremove__(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_rremove__with_callobjectcache___seq_rremove__(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__seq_rremove__with_callobjectcache___seq_rremove__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_rremove__unsupported(DeeObject *self, DeeObject *item, size_t start, size_t end);
#define default__seq_rremove__empty (*(int (DCALL *)(DeeObject *, DeeObject *, size_t, size_t))&_DeeNone_reti0_4)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_rremove__with__seq_enumerate_index_reverse__and__seq_operator_delitem_index(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_rremove__with__seq_rfind__and__seq_operator_delitem_index(DeeObject *self, DeeObject *item, size_t start, size_t end);

/* seq_rremove_with_key */
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL default__seq_rremove_with_key__with_callattr_rremove(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL default__seq_rremove_with_key__with_callattr___seq_rremove__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL default__seq_rremove_with_key__with_callobjectcache___seq_rremove__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3, 6)) int DCALL tdefault__seq_rremove_with_key__with_callobjectcache___seq_rremove__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL default__seq_rremove_with_key__unsupported(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
#define default__seq_rremove_with_key__empty (*(int (DCALL *)(DeeObject *, DeeObject *, size_t, size_t, DeeObject *))&_DeeNone_reti0_5)
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL default__seq_rremove_with_key__with__seq_enumerate_index_reverse__and__seq_operator_delitem_index(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL default__seq_rremove_with_key__with__seq_rfind_with_key__and__seq_operator_delitem_index(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);

/* seq_removeall */
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_removeall__with_callattr_removeall(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_removeall__with_callattr___seq_removeall__(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_removeall__with_callobjectcache___seq_removeall__(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max);
INTDEF WUNUSED NONNULL((1, 2, 3)) size_t DCALL tdefault__seq_removeall__with_callobjectcache___seq_removeall__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_removeall__unsupported(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max);
#define default__seq_removeall__empty (*(size_t (DCALL *)(DeeObject *, DeeObject *, size_t, size_t, size_t))&_DeeNone_rets0_5)
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_removeall__with__seq_removeif(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_removeall__with__seq_operator_size__and__seq_remove(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_removeall__with__seq_remove__once(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_removeall__with__seq_operator_size__and__seq_operator_trygetitem_index__and__seq_operator_delitem_index(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max);

/* seq_removeall_with_key */
INTDEF WUNUSED NONNULL((1, 2, 6)) size_t DCALL default__seq_removeall_with_key__with_callattr_removeall(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 6)) size_t DCALL default__seq_removeall_with_key__with_callattr___seq_removeall__(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 6)) size_t DCALL default__seq_removeall_with_key__with_callobjectcache___seq_removeall__(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3, 7)) size_t DCALL tdefault__seq_removeall_with_key__with_callobjectcache___seq_removeall__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 6)) size_t DCALL default__seq_removeall_with_key__unsupported(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max, DeeObject *key);
#define default__seq_removeall_with_key__empty (*(size_t (DCALL *)(DeeObject *, DeeObject *, size_t, size_t, size_t, DeeObject *))&_DeeNone_rets0_6)
INTDEF WUNUSED NONNULL((1, 2, 6)) size_t DCALL default__seq_removeall_with_key__with__seq_removeif(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 6)) size_t DCALL default__seq_removeall_with_key__with__seq_operator_size__and__seq_remove_with_key(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 6)) size_t DCALL default__seq_removeall_with_key__with__seq_remove_with_key__once(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 6)) size_t DCALL default__seq_removeall_with_key__with__seq_operator_size__and__seq_operator_trygetitem_index__and__seq_operator_delitem_index(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max, DeeObject *key);

/* seq_removeif */
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_removeif__with_callattr_removeif(DeeObject *self, DeeObject *should, size_t start, size_t end, size_t max);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_removeif__with_callattr___seq_removeif__(DeeObject *self, DeeObject *should, size_t start, size_t end, size_t max);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_removeif__with_callobjectcache___seq_removeif__(DeeObject *self, DeeObject *should, size_t start, size_t end, size_t max);
INTDEF WUNUSED NONNULL((1, 2, 3)) size_t DCALL tdefault__seq_removeif__with_callobjectcache___seq_removeif__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *should, size_t start, size_t end, size_t max);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_removeif__unsupported(DeeObject *self, DeeObject *should, size_t start, size_t end, size_t max);
#define default__seq_removeif__empty (*(size_t (DCALL *)(DeeObject *, DeeObject *, size_t, size_t, size_t))&_DeeNone_rets0_5)
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_removeif__with__seq_removeall_with_key(DeeObject *self, DeeObject *should, size_t start, size_t end, size_t max);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_removeif__with__seq_operator_size__and__seq_operator_trygetitem_index__and__seq_operator_delitem_index(DeeObject *self, DeeObject *should, size_t start, size_t end, size_t max);

/* seq_resize */
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_resize__with_callattr_resize(DeeObject *self, size_t newsize, DeeObject *filler);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_resize__with_callattr___seq_resize__(DeeObject *self, size_t newsize, DeeObject *filler);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_resize__with_callobjectcache___seq_resize__(DeeObject *self, size_t newsize, DeeObject *filler);
INTDEF WUNUSED NONNULL((1, 2, 4)) int DCALL tdefault__seq_resize__with_callobjectcache___seq_resize__(DeeTypeObject *tp_self, DeeObject *self, size_t newsize, DeeObject *filler);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_resize__unsupported(DeeObject *self, size_t newsize, DeeObject *filler);
#define default__seq_resize__none (*(int (DCALL *)(DeeObject *, size_t, DeeObject *))&_DeeNone_reti0_3)
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_resize__empty(DeeObject *self, size_t newsize, DeeObject *filler);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_resize__with__seq_operator_size__and__seq_operator_setrange_index__and__seq_operator_delrange_index(DeeObject *self, size_t newsize, DeeObject *filler);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_resize__with__seq_operator_size__and__seq_operator_setrange_index(DeeObject *self, size_t newsize, DeeObject *filler);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_resize__with__seq_operator_size__and__seq_erase__and__seq_extend(DeeObject *self, size_t newsize, DeeObject *filler);

/* seq_fill */
INTDEF WUNUSED NONNULL((1, 4)) int DCALL default__seq_fill__with_callattr_fill(DeeObject *self, size_t start, size_t end, DeeObject *filler);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL default__seq_fill__with_callattr___seq_fill__(DeeObject *self, size_t start, size_t end, DeeObject *filler);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL default__seq_fill__with_callobjectcache___seq_fill__(DeeObject *self, size_t start, size_t end, DeeObject *filler);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL tdefault__seq_fill__with_callobjectcache___seq_fill__(DeeTypeObject *tp_self, DeeObject *self, size_t start, size_t end, DeeObject *filler);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL default__seq_fill__unsupported(DeeObject *self, size_t start, size_t end, DeeObject *filler);
#define default__seq_fill__empty (*(int (DCALL *)(DeeObject *, size_t, size_t, DeeObject *))&_DeeNone_reti0_4)
INTDEF WUNUSED NONNULL((1, 4)) int DCALL default__seq_fill__with__seq_operator_size__and__seq_operator_setrange_index(DeeObject *self, size_t start, size_t end, DeeObject *filler);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL default__seq_fill__with__seq_enumerate_index__and__seq_operator_setitem_index(DeeObject *self, size_t start, size_t end, DeeObject *filler);

/* seq_reverse */
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_reverse__with_callattr_reverse(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_reverse__with_callattr___seq_reverse__(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_reverse__with_callobjectcache___seq_reverse__(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__seq_reverse__with_callobjectcache___seq_reverse__(DeeTypeObject *tp_self, DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_reverse__unsupported(DeeObject *self, size_t start, size_t end);
#define default__seq_reverse__empty (*(int (DCALL *)(DeeObject *, size_t, size_t))&_DeeNone_reti0_3)
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_reverse__with__seq_reversed__and__seq_operator_setrange_index(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_reverse__with__seq_operator_size__and__seq_operator_getitem_index__and__seq_operator_setitem_index__and__seq_operator_delitem_index(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_reverse__with__seq_operator_size__and__seq_operator_getitem_index__and__seq_operator_setitem_index(DeeObject *self, size_t start, size_t end);

/* seq_reversed */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_reversed__with_callattr_reversed(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_reversed__with_callattr___seq_reversed__(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_reversed__with_callobjectcache___seq_reversed__(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__seq_reversed__with_callobjectcache___seq_reversed__(DeeTypeObject *tp_self, DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_reversed__unsupported(DeeObject *self, size_t start, size_t end);
#define default__seq_reversed__none (*(DREF DeeObject *(DCALL *)(DeeObject *, size_t, size_t))&_DeeNone_NewRef3)
#define default__seq_reversed__empty (*(DREF DeeObject *(DCALL *)(DeeObject *, size_t, size_t))&default__seq_operator_getrange_index__empty)
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_reversed__with__seq_operator_size__and__operator_getitem_index_fast(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_reversed__with__seq_operator_size__and__seq_operator_getitem_index(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_reversed__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_reversed__with__seq_operator_foreach(DeeObject *self, size_t start, size_t end);

/* seq_sort */
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_sort__with_callattr_sort(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_sort__with_callattr___seq_sort__(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_sort__with_callobjectcache___seq_sort__(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__seq_sort__with_callobjectcache___seq_sort__(DeeTypeObject *tp_self, DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_sort__unsupported(DeeObject *self, size_t start, size_t end);
#define default__seq_sort__empty (*(int (DCALL *)(DeeObject *, size_t, size_t))&_DeeNone_reti0_3)
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_sort__with__seq_sorted__and__seq_operator_setrange_index(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_sort__with__seq_operator_size__and__seq_operator_getitem_index__and__seq_operator_setitem_index(DeeObject *self, size_t start, size_t end);

/* seq_sort_with_key */
INTDEF WUNUSED NONNULL((1, 4)) int DCALL default__seq_sort_with_key__with_callattr_sort(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL default__seq_sort_with_key__with_callattr___seq_sort__(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL default__seq_sort_with_key__with_callobjectcache___seq_sort__(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL tdefault__seq_sort_with_key__with_callobjectcache___seq_sort__(DeeTypeObject *tp_self, DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL default__seq_sort_with_key__unsupported(DeeObject *self, size_t start, size_t end, DeeObject *key);
#define default__seq_sort_with_key__empty (*(int (DCALL *)(DeeObject *, size_t, size_t, DeeObject *))&_DeeNone_reti0_4)
INTDEF WUNUSED NONNULL((1, 4)) int DCALL default__seq_sort_with_key__with__seq_sorted_with_key__and__seq_operator_setrange_index(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL default__seq_sort_with_key__with__seq_operator_size__and__seq_operator_getitem_index__and__seq_operator_setitem_index(DeeObject *self, size_t start, size_t end, DeeObject *key);

/* seq_sorted */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_sorted__with_callattr_sorted(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_sorted__with_callattr___seq_sorted__(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_sorted__with_callobjectcache___seq_sorted__(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__seq_sorted__with_callobjectcache___seq_sorted__(DeeTypeObject *tp_self, DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_sorted__unsupported(DeeObject *self, size_t start, size_t end);
#define default__seq_sorted__none (*(DREF DeeObject *(DCALL *)(DeeObject *, size_t, size_t))&_DeeNone_NewRef3)
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_sorted__empty(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_sorted__with__seq_operator_size__and__operator_getitem_index_fast(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_sorted__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_sorted__with__seq_operator_foreach(DeeObject *self, size_t start, size_t end);

/* seq_sorted_with_key */
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL default__seq_sorted_with_key__with_callattr_sorted(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL default__seq_sorted_with_key__with_callattr___seq_sorted__(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL default__seq_sorted_with_key__with_callobjectcache___seq_sorted__(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL tdefault__seq_sorted_with_key__with_callobjectcache___seq_sorted__(DeeTypeObject *tp_self, DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL default__seq_sorted_with_key__unsupported(DeeObject *self, size_t start, size_t end, DeeObject *key);
#define default__seq_sorted_with_key__none (*(DREF DeeObject *(DCALL *)(DeeObject *, size_t, size_t, DeeObject *))&_DeeNone_NewRef4)
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL default__seq_sorted_with_key__empty(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL default__seq_sorted_with_key__with__seq_operator_size__and__operator_getitem_index_fast(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL default__seq_sorted_with_key__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL default__seq_sorted_with_key__with__seq_operator_foreach(DeeObject *self, size_t start, size_t end, DeeObject *key);

/* seq_bfind */
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_bfind__with_callattr_bfind(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_bfind__with_callattr___seq_bfind__(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_bfind__with_callobjectcache___seq_bfind__(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 3)) size_t DCALL tdefault__seq_bfind__with_callobjectcache___seq_bfind__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_bfind__unsupported(DeeObject *self, DeeObject *item, size_t start, size_t end);
#define default__seq_bfind__empty (*(size_t (DCALL *)(DeeObject *, DeeObject *, size_t, size_t))&_DeeNone_retsm1_4)
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_bfind__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *self, DeeObject *item, size_t start, size_t end);

/* seq_bfind_with_key */
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL default__seq_bfind_with_key__with_callattr_bfind(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL default__seq_bfind_with_key__with_callattr___seq_bfind__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL default__seq_bfind_with_key__with_callobjectcache___seq_bfind__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3, 6)) size_t DCALL tdefault__seq_bfind_with_key__with_callobjectcache___seq_bfind__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL default__seq_bfind_with_key__unsupported(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
#define default__seq_bfind_with_key__empty (*(size_t (DCALL *)(DeeObject *, DeeObject *, size_t, size_t, DeeObject *))&_DeeNone_retsm1_5)
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL default__seq_bfind_with_key__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);

/* seq_bposition */
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_bposition__with_callattr_bposition(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_bposition__with_callattr___seq_bposition__(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_bposition__with_callobjectcache___seq_bposition__(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 3)) size_t DCALL tdefault__seq_bposition__with_callobjectcache___seq_bposition__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_bposition__unsupported(DeeObject *self, DeeObject *item, size_t start, size_t end);
#define default__seq_bposition__empty (*(size_t (DCALL *)(DeeObject *, DeeObject *, size_t, size_t))&_DeeNone_rets0_4)
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_bposition__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *self, DeeObject *item, size_t start, size_t end);

/* seq_bposition_with_key */
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL default__seq_bposition_with_key__with_callattr_bposition(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL default__seq_bposition_with_key__with_callattr___seq_bposition__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL default__seq_bposition_with_key__with_callobjectcache___seq_bposition__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3, 6)) size_t DCALL tdefault__seq_bposition_with_key__with_callobjectcache___seq_bposition__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL default__seq_bposition_with_key__unsupported(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
#define default__seq_bposition_with_key__empty (*(size_t (DCALL *)(DeeObject *, DeeObject *, size_t, size_t, DeeObject *))&_DeeNone_rets0_5)
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL default__seq_bposition_with_key__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);

/* seq_brange */
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL default__seq_brange__with_callattr_brange(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t result_range[2]);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL default__seq_brange__with_callattr___seq_brange__(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t result_range[2]);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL default__seq_brange__with_callobjectcache___seq_brange__(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t result_range[2]);
INTDEF WUNUSED NONNULL((1, 2, 3, 6)) int DCALL tdefault__seq_brange__with_callobjectcache___seq_brange__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item, size_t start, size_t end, size_t result_range[2]);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL default__seq_brange__unsupported(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t result_range[2]);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL default__seq_brange__empty(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t result_range[2]);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL default__seq_brange__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t result_range[2]);

/* seq_brange_with_key */
INTDEF WUNUSED NONNULL((1, 2, 5, 6)) int DCALL default__seq_brange_with_key__with_callattr_brange(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key, size_t result_range[2]);
INTDEF WUNUSED NONNULL((1, 2, 5, 6)) int DCALL default__seq_brange_with_key__with_callattr___seq_brange__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key, size_t result_range[2]);
INTDEF WUNUSED NONNULL((1, 2, 5, 6)) int DCALL default__seq_brange_with_key__with_callobjectcache___seq_brange__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key, size_t result_range[2]);
INTDEF WUNUSED NONNULL((1, 2, 3, 6, 7)) int DCALL tdefault__seq_brange_with_key__with_callobjectcache___seq_brange__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key, size_t result_range[2]);
INTDEF WUNUSED NONNULL((1, 2, 5, 6)) int DCALL default__seq_brange_with_key__unsupported(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key, size_t result_range[2]);
INTDEF WUNUSED NONNULL((1, 2, 5, 6)) int DCALL default__seq_brange_with_key__empty(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key, size_t result_range[2]);
INTDEF WUNUSED NONNULL((1, 2, 5, 6)) int DCALL default__seq_brange_with_key__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key, size_t result_range[2]);

/* set_operator_iter */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__set_operator_iter(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__set_operator_iter__with_callattr___set_iter__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__set_operator_iter__with_callobjectcache___set_iter__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__set_operator_iter__with_callobjectcache___set_iter__(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__set_operator_iter__unsupported(DeeObject *__restrict self);
#define default__set_operator_iter__none (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&_DeeNone_NewRef1)
#define default__set_operator_iter__empty (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&default__seq_operator_iter__empty)
#define default__set_operator_iter__with__map_enumerate (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&default__seq_operator_iter__with__map_enumerate)
#define default__set_operator_iter__with__map_iterkeys__and__map_operator_trygetitem (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&default__seq_operator_iter__with__map_iterkeys__and__map_operator_trygetitem)
#define default__set_operator_iter__with__map_iterkeys__and__map_operator_getitem (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&default__seq_operator_iter__with__map_iterkeys__and__map_operator_getitem)
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__set_operator_iter__with__seq_operator_iter(DeeObject *__restrict self);

/* set_operator_foreach */
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__set_operator_foreach(DeeObject *__restrict self, Dee_foreach_t cb, void *arg);
#define default__set_operator_foreach__with_callattr___set_iter__ (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&default__set_operator_foreach__with__set_operator_iter)
#define default__set_operator_foreach__with_callobjectcache___set_iter__ (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&default__set_operator_foreach__with__set_operator_iter)
#define default__set_operator_foreach__unsupported (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&default__set_operator_foreach__with__set_operator_iter)
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__set_operator_foreach__with__set_operator_iter(DeeObject *__restrict self, Dee_foreach_t cb, void *arg);
#define default__set_operator_foreach__empty (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&_DeeNone_rets0_3)
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__set_operator_foreach__with__seq_operator_foreach(DeeObject *__restrict self, Dee_foreach_t cb, void *arg);
#define default__set_operator_foreach__with__map_operator_foreach_pair (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&default__seq_operator_foreach__with__seq_operator_foreach_pair)

/* set_operator_sizeob */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__set_operator_sizeob(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__set_operator_sizeob__with_callattr___set_size__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__set_operator_sizeob__with_callobjectcache___set_size__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__set_operator_sizeob__with_callobjectcache___set_size__(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__set_operator_sizeob__unsupported(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__set_operator_sizeob__with__set_operator_size(DeeObject *__restrict self);
#define default__set_operator_sizeob__none (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&_DeeNone_NewRef1)
#define default__set_operator_sizeob__empty (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&default__seq_operator_sizeob__empty)

/* set_operator_size */
INTDEF WUNUSED NONNULL((1)) size_t DCALL default__set_operator_size(DeeObject *__restrict self);
#define default__set_operator_size__with_callattr___set_size__ (*(size_t (DCALL *)(DeeObject *__restrict))&default__set_operator_size__with__set_operator_sizeob)
#define default__set_operator_size__with_callobjectcache___set_size__ (*(size_t (DCALL *)(DeeObject *__restrict))&default__set_operator_size__with__set_operator_sizeob)
INTDEF WUNUSED NONNULL((1)) size_t DCALL default__set_operator_size__unsupported(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) size_t DCALL default__set_operator_size__with__set_operator_sizeob(DeeObject *__restrict self);
#define default__set_operator_size__empty (*(size_t (DCALL *)(DeeObject *__restrict))&default__seq_operator_size__empty)
INTDEF WUNUSED NONNULL((1)) size_t DCALL default__set_operator_size__with__set_operator_foreach(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) size_t DCALL default__set_operator_size__with__set_operator_iter(DeeObject *__restrict self);

/* set_operator_hash */
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL default__set_operator_hash(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL default__set_operator_hash__with_callattr___set_hash__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL default__set_operator_hash__with_callobjectcache___set_hash__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) Dee_hash_t DCALL tdefault__set_operator_hash__with_callobjectcache___set_hash__(DeeTypeObject *tp_self, DeeObject *self);
#define default__set_operator_hash__unsupported (*(Dee_hash_t (DCALL *)(DeeObject *__restrict))&default__seq_operator_hash__unsupported)
#define default__set_operator_hash__none (*(Dee_hash_t (DCALL *)(DeeObject *__restrict))&_DeeNone_rets0_1)
#define default__set_operator_hash__empty (*(Dee_hash_t (DCALL *)(DeeObject *__restrict))&_DeeNone_rets0_1)
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL default__set_operator_hash__with__set_operator_foreach(DeeObject *__restrict self);
#define default__set_operator_hash__with__map_operator_foreach_pair (*(Dee_hash_t (DCALL *)(DeeObject *__restrict))&default__map_operator_hash__with__map_operator_foreach_pair)

/* set_operator_compare_eq */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__set_operator_compare_eq(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__set_operator_compare_eq__with_callattr_equals(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__set_operator_compare_eq__with_callattr___set_compare_eq__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__set_operator_compare_eq__with_callobjectcache___set_compare_eq__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__set_operator_compare_eq__with_callobjectcache___set_compare_eq__(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__set_operator_compare_eq__unsupported(DeeObject *lhs, DeeObject *rhs);
#define default__set_operator_compare_eq__empty (*(int (DCALL *)(DeeObject *, DeeObject *))&default__seq_operator_compare_eq__empty)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__set_operator_compare_eq__with__set_operator_eq(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__set_operator_compare_eq__with__set_operator_ne(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__set_operator_compare_eq__with__set_operator_foreach(DeeObject *lhs, DeeObject *rhs);

/* set_operator_trycompare_eq */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__set_operator_trycompare_eq(DeeObject *lhs, DeeObject *rhs);
#define default__set_operator_trycompare_eq__with_callattr_equals (*(int (DCALL *)(DeeObject *, DeeObject *))&default__set_operator_trycompare_eq__with__set_operator_compare_eq)
#define default__set_operator_trycompare_eq__with_callattr___set_compare_eq__ (*(int (DCALL *)(DeeObject *, DeeObject *))&default__set_operator_trycompare_eq__with__set_operator_compare_eq)
#define default__set_operator_trycompare_eq__with_callobjectcache___set_compare_eq__ (*(int (DCALL *)(DeeObject *, DeeObject *))&default__set_operator_trycompare_eq__with__set_operator_compare_eq)
#define default__set_operator_trycompare_eq__unsupported (*(int (DCALL *)(DeeObject *, DeeObject *))&_DeeNone_reti1_2)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__set_operator_trycompare_eq__with__set_operator_compare_eq(DeeObject *lhs, DeeObject *rhs);
#define default__set_operator_trycompare_eq__empty (*(int (DCALL *)(DeeObject *, DeeObject *))&default__seq_operator_trycompare_eq__empty)

/* set_operator_eq */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_operator_eq(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_operator_eq__with_callattr___set_eq__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_operator_eq__with_callobjectcache___set_eq__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__set_operator_eq__with_callobjectcache___set_eq__(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_operator_eq__unsupported(DeeObject *lhs, DeeObject *rhs);
#define default__set_operator_eq__empty (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&default__set_operator_eq__with__set_operator_compare_eq)
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_operator_eq__with__set_operator_compare_eq(DeeObject *lhs, DeeObject *rhs);

/* set_operator_ne */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_operator_ne(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_operator_ne__with_callattr___set_ne__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_operator_ne__with_callobjectcache___set_ne__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__set_operator_ne__with_callobjectcache___set_ne__(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_operator_ne__unsupported(DeeObject *lhs, DeeObject *rhs);
#define default__set_operator_ne__empty (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&default__set_operator_ne__with__set_operator_compare_eq)
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_operator_ne__with__set_operator_compare_eq(DeeObject *lhs, DeeObject *rhs);

/* set_operator_lo */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_operator_lo(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_operator_lo__with_callattr___set_lo__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_operator_lo__with_callobjectcache___set_lo__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__set_operator_lo__with_callobjectcache___set_lo__(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_operator_lo__unsupported(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_operator_lo__empty(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_operator_lo__with__set_operator_ge(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_operator_lo__with__set_operator_foreach(DeeObject *lhs, DeeObject *rhs);

/* set_operator_le */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_operator_le(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_operator_le__with_callattr_issubset(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_operator_le__with_callattr___set_le__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_operator_le__with_callobjectcache___set_le__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__set_operator_le__with_callobjectcache___set_le__(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_operator_le__unsupported(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_operator_le__empty(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_operator_le__with__set_operator_gr(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_operator_le__with__set_operator_foreach(DeeObject *lhs, DeeObject *rhs);

/* set_operator_gr */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_operator_gr(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_operator_gr__with_callattr___set_gr__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_operator_gr__with_callobjectcache___set_gr__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__set_operator_gr__with_callobjectcache___set_gr__(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_operator_gr__unsupported(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_operator_gr__empty(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_operator_gr__with__set_operator_le(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_operator_gr__with__set_operator_foreach(DeeObject *lhs, DeeObject *rhs);

/* set_operator_ge */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_operator_ge(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_operator_ge__with_callattr_issuperset(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_operator_ge__with_callattr___set_ge__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_operator_ge__with_callobjectcache___set_ge__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__set_operator_ge__with_callobjectcache___set_ge__(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_operator_ge__unsupported(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_operator_ge__empty(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_operator_ge__with__set_operator_lo(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_operator_ge__with__set_operator_foreach(DeeObject *lhs, DeeObject *rhs);

/* set_operator_bool */
INTDEF WUNUSED NONNULL((1)) int DCALL default__set_operator_bool(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__set_operator_bool__with_callattr___set_bool__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__set_operator_bool__with_callobjectcache___set_bool__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__set_operator_bool__with_callobjectcache___set_bool__(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__set_operator_bool__unsupported(DeeObject *__restrict self);
#define default__set_operator_bool__empty (*(int (DCALL *)(DeeObject *__restrict))&default__seq_operator_bool__empty)
#define default__set_operator_bool__with__set_boundfirst (*(int (DCALL *)(DeeObject *__restrict))&default__seq_operator_bool__with__set_boundfirst)
#define default__set_operator_bool__with__set_boundlast (*(int (DCALL *)(DeeObject *__restrict))&default__seq_operator_bool__with__set_boundlast)
#define default__set_operator_bool__with__set_trygetfirst (*(int (DCALL *)(DeeObject *__restrict))&default__seq_operator_bool__with__set_trygetfirst)
#define default__set_operator_bool__with__set_trygetlast (*(int (DCALL *)(DeeObject *__restrict))&default__seq_operator_bool__with__set_trygetlast)
#define default__set_operator_bool__with__seq_operator_foreach (*(int (DCALL *)(DeeObject *__restrict))&default__seq_operator_bool__with__seq_operator_foreach)
#define default__set_operator_bool__with__seq_operator_foreach_pair (*(int (DCALL *)(DeeObject *__restrict))&default__seq_operator_bool__with__seq_operator_foreach_pair)
#define default__set_operator_bool__with__seq_operator_iter (*(int (DCALL *)(DeeObject *__restrict))&default__seq_operator_bool__with__seq_operator_iter)
#define default__set_operator_bool__with__seq_operator_size (*(int (DCALL *)(DeeObject *__restrict))&default__seq_operator_bool__with__seq_operator_size)
#define default__set_operator_bool__with__seq_operator_sizeob (*(int (DCALL *)(DeeObject *__restrict))&default__seq_operator_bool__with__seq_operator_sizeob)
#define default__set_operator_bool__with__seq_operator_compare_eq (*(int (DCALL *)(DeeObject *__restrict))&default__seq_operator_bool__with__seq_operator_compare_eq)
#define default__set_operator_bool__with__set_operator_compare_eq (*(int (DCALL *)(DeeObject *__restrict))&default__seq_operator_bool__with__set_operator_compare_eq)
#define default__set_operator_bool__with__map_operator_compare_eq (*(int (DCALL *)(DeeObject *__restrict))&default__seq_operator_bool__with__map_operator_compare_eq)
INTDEF WUNUSED NONNULL((1)) int DCALL default__set_operator_bool__with__set_operator_foreach(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__set_operator_bool__with__set_operator_size(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__set_operator_bool__with__set_operator_sizeob(DeeObject *__restrict self);

/* set_operator_inv */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__set_operator_inv(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__set_operator_inv__with_callattr___set_inv__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__set_operator_inv__with_callobjectcache___set_inv__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__set_operator_inv__with_callobjectcache___set_inv__(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__set_operator_inv__unsupported(DeeObject *__restrict self);
#define default__set_operator_inv__none (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&_DeeNone_NewRef1)
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__set_operator_inv__empty(DeeObject *__restrict self);

/* set_operator_add */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_operator_add(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_operator_add__with_callattr_union(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_operator_add__with_callattr___set_add__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_operator_add__with_callobjectcache___set_add__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__set_operator_add__with_callobjectcache___set_add__(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_operator_add__unsupported(DeeObject *lhs, DeeObject *rhs);
#define default__set_operator_add__none (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&_DeeNone_NewRef2)
#define default__set_operator_add__empty (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&default__set_operator_add__unsupported)

/* set_operator_sub */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_operator_sub(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_operator_sub__with_callattr_difference(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_operator_sub__with_callattr___set_sub__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_operator_sub__with_callobjectcache___set_sub__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__set_operator_sub__with_callobjectcache___set_sub__(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_operator_sub__unsupported(DeeObject *lhs, DeeObject *rhs);
#define default__set_operator_sub__none (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&_DeeNone_NewRef2)
#define default__set_operator_sub__empty (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&default__set_operator_sub__unsupported)

/* set_operator_and */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_operator_and(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_operator_and__with_callattr_intersection(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_operator_and__with_callattr___set_and__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_operator_and__with_callobjectcache___set_and__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__set_operator_and__with_callobjectcache___set_and__(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_operator_and__unsupported(DeeObject *lhs, DeeObject *rhs);
#define default__set_operator_and__none (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&_DeeNone_NewRef2)
#define default__set_operator_and__empty (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&default__set_operator_and__unsupported)

/* set_operator_xor */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_operator_xor(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_operator_xor__with_callattr_symmetric_difference(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_operator_xor__with_callattr___set_xor__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_operator_xor__with_callobjectcache___set_xor__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__set_operator_xor__with_callobjectcache___set_xor__(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_operator_xor__unsupported(DeeObject *lhs, DeeObject *rhs);
#define default__set_operator_xor__none (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&_DeeNone_NewRef2)
#define default__set_operator_xor__empty (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&default__set_operator_xor__unsupported)

/* set_operator_inplace_add */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__set_operator_inplace_add(DREF DeeObject **__restrict p_self, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__set_operator_inplace_add__with_callattr___set_inplace_add__(DREF DeeObject **__restrict p_self, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__set_operator_inplace_add__with_callobjectcache___set_inplace_add__(DREF DeeObject **__restrict p_self, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__set_operator_inplace_add__with_callobjectcache___set_inplace_add__(DeeTypeObject *tp_self, DREF DeeObject **p_self, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__set_operator_inplace_add__unsupported(DREF DeeObject **__restrict p_self, DeeObject *rhs);
#define default__set_operator_inplace_add__none (*(int (DCALL *)(DREF DeeObject **__restrict, DeeObject *))&_DeeNone_reti0_2)
#define default__set_operator_inplace_add__empty (*(int (DCALL *)(DREF DeeObject **__restrict, DeeObject *))&default__set_operator_inplace_add__unsupported)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__set_operator_inplace_add__with__set_insertall(DREF DeeObject **__restrict p_self, DeeObject *rhs);

/* set_operator_inplace_sub */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__set_operator_inplace_sub(DREF DeeObject **__restrict p_self, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__set_operator_inplace_sub__with_callattr___set_inplace_sub__(DREF DeeObject **__restrict p_self, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__set_operator_inplace_sub__with_callobjectcache___set_inplace_sub__(DREF DeeObject **__restrict p_self, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__set_operator_inplace_sub__with_callobjectcache___set_inplace_sub__(DeeTypeObject *tp_self, DREF DeeObject **p_self, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__set_operator_inplace_sub__unsupported(DREF DeeObject **__restrict p_self, DeeObject *rhs);
#define default__set_operator_inplace_sub__none (*(int (DCALL *)(DREF DeeObject **__restrict, DeeObject *))&_DeeNone_reti0_2)
#define default__set_operator_inplace_sub__empty (*(int (DCALL *)(DREF DeeObject **__restrict, DeeObject *))&default__set_operator_inplace_sub__unsupported)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__set_operator_inplace_sub__with__set_operator_foreach__and__set_removeall(DREF DeeObject **__restrict p_self, DeeObject *rhs);

/* set_operator_inplace_and */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__set_operator_inplace_and(DREF DeeObject **__restrict p_self, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__set_operator_inplace_and__with_callattr___set_inplace_and__(DREF DeeObject **__restrict p_self, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__set_operator_inplace_and__with_callobjectcache___set_inplace_and__(DREF DeeObject **__restrict p_self, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__set_operator_inplace_and__with_callobjectcache___set_inplace_and__(DeeTypeObject *tp_self, DREF DeeObject **p_self, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__set_operator_inplace_and__unsupported(DREF DeeObject **__restrict p_self, DeeObject *rhs);
#define default__set_operator_inplace_and__none (*(int (DCALL *)(DREF DeeObject **__restrict, DeeObject *))&_DeeNone_reti0_2)
#define default__set_operator_inplace_and__empty (*(int (DCALL *)(DREF DeeObject **__restrict, DeeObject *))&default__set_operator_inplace_and__unsupported)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__set_operator_inplace_and__with__set_operator_foreach__and__set_removeall(DREF DeeObject **__restrict p_self, DeeObject *rhs);

/* set_operator_inplace_xor */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__set_operator_inplace_xor(DREF DeeObject **__restrict p_self, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__set_operator_inplace_xor__with_callattr___set_inplace_xor__(DREF DeeObject **__restrict p_self, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__set_operator_inplace_xor__with_callobjectcache___set_inplace_xor__(DREF DeeObject **__restrict p_self, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__set_operator_inplace_xor__with_callobjectcache___set_inplace_xor__(DeeTypeObject *tp_self, DREF DeeObject **p_self, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__set_operator_inplace_xor__unsupported(DREF DeeObject **__restrict p_self, DeeObject *rhs);
#define default__set_operator_inplace_xor__none (*(int (DCALL *)(DREF DeeObject **__restrict, DeeObject *))&_DeeNone_reti0_2)
#define default__set_operator_inplace_xor__empty (*(int (DCALL *)(DREF DeeObject **__restrict, DeeObject *))&default__set_operator_inplace_xor__unsupported)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__set_operator_inplace_xor__with__set_operator_foreach__and__set_insertall__and__set_removeall(DREF DeeObject **__restrict p_self, DeeObject *rhs);

/* set_frozen */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__set_frozen(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__set_frozen__with_callattr_frozen(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__set_frozen__with_callattr___set_frozen__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__set_frozen__with_callobjectcache___set_frozen__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__set_frozen__with_callobjectcache___set_frozen__(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__set_frozen__unsupported(DeeObject *__restrict self);
#define default__set_frozen__none (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&_DeeNone_NewRef1)
#define default__set_frozen__empty (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&DeeObject_NewRef)
#define default__set_frozen__with__set_operator_foreach (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&DeeRoSet_FromSequence)
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__set_frozen__with__map_frozen(DeeObject *__restrict self);

/* set_unify */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_unify__with_callattr_unify(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_unify__with_callattr___set_unify__(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_unify__with_callobjectcache___set_unify__(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__set_unify__with_callobjectcache___set_unify__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_unify__unsupported(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_unify__none(DeeObject *self, DeeObject *key);
#define default__set_unify__empty (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&default__set_unify__unsupported)
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_unify__with__seq_operator_foreach__and__set_insert(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_unify__with__seq_operator_foreach__and__seq_append(DeeObject *self, DeeObject *key);

/* set_insert */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__set_insert__with_callattr_insert(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__set_insert__with_callattr___set_insert__(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__set_insert__with_callobjectcache___set_insert__(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__set_insert__with_callobjectcache___set_insert__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__set_insert__unsupported(DeeObject *self, DeeObject *key);
#define default__set_insert__none (*(int (DCALL *)(DeeObject *, DeeObject *))&_DeeNone_reti0_2)
#define default__set_insert__empty (*(int (DCALL *)(DeeObject *, DeeObject *))&default__set_insert__unsupported)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__set_insert__with__map_setnew(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__set_insert__with__seq_operator_size__and__set_insertall(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__set_insert__with__seq_contains__and__seq_append(DeeObject *self, DeeObject *key);

/* set_insertall */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__set_insertall__with_callattr_insertall(DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__set_insertall__with_callattr___set_insertall__(DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__set_insertall__with_callobjectcache___set_insertall__(DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__set_insertall__with_callobjectcache___set_insertall__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__set_insertall__unsupported(DeeObject *self, DeeObject *keys);
#define default__set_insertall__none (*(int (DCALL *)(DeeObject *, DeeObject *))&_DeeNone_reti0_2)
#define default__set_insertall__empty (*(int (DCALL *)(DeeObject *, DeeObject *))&default__set_insertall__unsupported)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__set_insertall__with__set_operator_inplace_add(DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__set_insertall__with__set_insert(DeeObject *self, DeeObject *keys);

/* set_remove */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__set_remove__with_callattr_remove(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__set_remove__with_callattr___set_remove__(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__set_remove__with_callobjectcache___set_remove__(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__set_remove__with_callobjectcache___set_remove__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__set_remove__unsupported(DeeObject *self, DeeObject *key);
#define default__set_remove__none (*(int (DCALL *)(DeeObject *, DeeObject *))&_DeeNone_reti0_2)
#define default__set_remove__empty (*(int (DCALL *)(DeeObject *, DeeObject *))&default__set_remove__unsupported)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__set_remove__with__map_operator_trygetitem__and__map_operator_delitem(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__set_remove__with__seq_operator_size__and__set_removeall(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__set_remove__with__seq_removeall(DeeObject *self, DeeObject *key);

/* set_removeall */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__set_removeall__with_callattr_removeall(DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__set_removeall__with_callattr___set_removeall__(DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__set_removeall__with_callobjectcache___set_removeall__(DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__set_removeall__with_callobjectcache___set_removeall__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__set_removeall__unsupported(DeeObject *self, DeeObject *keys);
#define default__set_removeall__none (*(int (DCALL *)(DeeObject *, DeeObject *))&_DeeNone_reti0_2)
#define default__set_removeall__empty (*(int (DCALL *)(DeeObject *, DeeObject *))&default__set_removeall__unsupported)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__set_removeall__with__set_operator_inplace_sub(DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__set_removeall__with__set_remove(DeeObject *self, DeeObject *keys);

/* set_pop */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__set_pop__with_callattr_pop(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__set_pop__with_callattr___set_pop__(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__set_pop__with_callobjectcache___set_pop__(DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__set_pop__with_callobjectcache___set_pop__(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__set_pop__unsupported(DeeObject *self);
#define default__set_pop__none (*(DREF DeeObject *(DCALL *)(DeeObject *))&_DeeNone_NewRef1)
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__set_pop__empty(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__set_pop__with__seq_trygetfirst__and__set_remove(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__set_pop__with__map_popitem(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__set_pop__with__seq_pop(DeeObject *self);

/* set_pop_with_default */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_pop_with_default__with_callattr_pop(DeeObject *self, DeeObject *default_);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_pop_with_default__with_callattr___set_pop__(DeeObject *self, DeeObject *default_);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_pop_with_default__with_callobjectcache___set_pop__(DeeObject *self, DeeObject *default_);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__set_pop_with_default__with_callobjectcache___set_pop__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *default_);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_pop_with_default__unsupported(DeeObject *self, DeeObject *default_);
#define default__set_pop_with_default__none (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&_DeeNone_NewRef2)
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_pop_with_default__empty(DeeObject *self, DeeObject *default_);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_pop_with_default__with__seq_trygetfirst__and__set_remove(DeeObject *self, DeeObject *default_);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_pop_with_default__with__map_popitem(DeeObject *self, DeeObject *default_);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__set_pop_with_default__with__seq_pop(DeeObject *self, DeeObject *default_);

/* set_trygetfirst */
#define default__set_trygetfirst__with_callattr_first (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&default__set_trygetfirst__with__set_getfirst)
#define default__set_trygetfirst__with_callattr___set_first__ (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&default__set_trygetfirst__with__set_getfirst)
#define default__set_trygetfirst__with_callobjectcache___set_first__ (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&default__set_trygetfirst__with__set_getfirst)
#define default__set_trygetfirst__unsupported (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&default__set_getfirst__unsupported)
#define default__set_trygetfirst__none (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&_DeeNone_NewRef1)
#define default__set_trygetfirst__empty (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&_DeeNone_retsm1_1)
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__set_trygetfirst__with__set_getfirst(DeeObject *__restrict self);
#define default__set_trygetfirst__with__seq_operator_size__and__operator_getitem_index_fast_ab (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&default__seq_trygetfirst__with__seq_operator_size__and__operator_getitem_index_fast)
#define default__set_trygetfirst__with__seq_operator_trygetitem_index_ab (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&default__seq_trygetfirst__with__seq_operator_trygetitem_index)
#define default__set_trygetfirst__with__seq_operator_foreach (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&default__seq_trygetfirst__with__seq_operator_foreach)

/* set_getfirst */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__set_getfirst(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__set_getfirst__with_callattr_first(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__set_getfirst__with_callattr___set_first__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__set_getfirst__with_callobjectcache___set_first__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__set_getfirst__with_callobjectcache___set_first__(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__set_getfirst__unsupported(DeeObject *__restrict self);
#define default__set_getfirst__none (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&_DeeNone_NewRef1)
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__set_getfirst__empty(DeeObject *__restrict self);
#define default__set_getfirst__with__seq_operator_getitem_index_ab (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&default__seq_getfirst__with__seq_operator_getitem_index)
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__set_getfirst__with__set_trygetfirst(DeeObject *__restrict self);

/* set_boundfirst */
INTDEF WUNUSED NONNULL((1)) int DCALL default__set_boundfirst(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__set_boundfirst__with_callattr_first(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__set_boundfirst__with_callattr___set_first__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__set_boundfirst__with_callobjectcache___set_first__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__set_boundfirst__with_callobjectcache___set_first__(DeeTypeObject *tp_self, DeeObject *self);
#define default__set_boundfirst__unsupported (*(int (DCALL *)(DeeObject *__restrict))&default__set_boundfirst__empty)
#define default__set_boundfirst__none (*(int (DCALL *)(DeeObject *__restrict))&_DeeNone_reti1_1)
#define default__set_boundfirst__empty (*(int (DCALL *)(DeeObject *__restrict))&_DeeNone_reti0_1)
#define default__set_boundfirst__with__set_operator_bool (*(int (DCALL *)(DeeObject *__restrict))&default__seq_boundfirst__with__set_operator_bool)

/* set_delfirst */
INTDEF WUNUSED NONNULL((1)) int DCALL default__set_delfirst(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__set_delfirst__with_callattr_first(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__set_delfirst__with_callattr___set_first__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__set_delfirst__with_callobjectcache___set_first__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__set_delfirst__with_callobjectcache___set_first__(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__set_delfirst__unsupported(DeeObject *__restrict self);
#define default__set_delfirst__empty (*(int (DCALL *)(DeeObject *__restrict))&_DeeNone_reti0_1)
#define default__set_delfirst__with__seq_operator_delitem_index (*(int (DCALL *)(DeeObject *__restrict))&default__seq_delfirst__with__seq_operator_delitem_index)
INTDEF WUNUSED NONNULL((1)) int DCALL default__set_delfirst__with__seq_operator_size__and__seq_operator_bounditem_index__seq_operator_delitem_index(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__set_delfirst__with__seq_operator_bounditem_index__seq_operator_delitem_index(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__set_delfirst__with__set_trygetfirst__set_remove(DeeObject *__restrict self);

/* set_setfirst */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__set_setfirst(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__set_setfirst__with_callattr_first(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__set_setfirst__with_callattr___set_first__(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__set_setfirst__with_callobjectcache___set_first__(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__set_setfirst__with_callobjectcache___set_first__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__set_setfirst__unsupported(DeeObject *self, DeeObject *value);
#define default__set_setfirst__none (*(int (DCALL *)(DeeObject *, DeeObject *))&_DeeNone_reti0_2)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__set_setfirst__empty(DeeObject *self, DeeObject *value);
#define default__set_setfirst__with__seq_operator_setitem_index (*(int (DCALL *)(DeeObject *, DeeObject *))&default__seq_setfirst__with__seq_operator_setitem_index)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__set_setfirst__with__seq_operator_size__and__seq_operator_bounditem_index__seq_operator_setitem_index(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__set_setfirst__with__seq_operator_bounditem_index__seq_operator_setitem_index(DeeObject *self, DeeObject *value);

/* set_trygetlast */
#define default__set_trygetlast__with_callattr_last (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&default__set_trygetlast__with__set_getlast)
#define default__set_trygetlast__with_callattr___set_last__ (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&default__set_trygetlast__with__set_getlast)
#define default__set_trygetlast__with_callobjectcache___set_last__ (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&default__set_trygetlast__with__set_getlast)
#define default__set_trygetlast__unsupported (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&default__set_getlast__unsupported)
#define default__set_trygetlast__none (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&_DeeNone_NewRef1)
#define default__set_trygetlast__empty (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&_DeeNone_retsm1_1)
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__set_trygetlast__with__set_getlast(DeeObject *__restrict self);
#define default__set_trygetlast__with__seq_operator_size__and__operator_getitem_index_fast_ab (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&default__seq_trygetlast__with__seq_operator_size__and__operator_getitem_index_fast)
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__set_trygetlast__with__seq_operator_size__and__operator_getitem_index_fast(DeeObject *__restrict self);
#define default__set_trygetlast__with__seq_operator_size__and__seq_operator_trygetitem_index_ab (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&default__seq_trygetlast__with__seq_operator_size__and__seq_operator_trygetitem_index)
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__set_trygetlast__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *__restrict self);
#define default__set_trygetlast__with__seq_operator_sizeob__and__seq_operator_trygetitem_ab (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&default__seq_trygetlast__with__seq_operator_sizeob__and__seq_operator_trygetitem)
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__set_trygetlast__with__seq_operator_sizeob__and__seq_operator_trygetitem(DeeObject *__restrict self);
#define default__set_trygetlast__with__seq_operator_foreach (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&default__seq_trygetlast__with__seq_operator_foreach)

/* set_getlast */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__set_getlast(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__set_getlast__with_callattr_last(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__set_getlast__with_callattr___set_last__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__set_getlast__with_callobjectcache___set_last__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__set_getlast__with_callobjectcache___set_last__(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__set_getlast__unsupported(DeeObject *__restrict self);
#define default__set_getlast__none (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&_DeeNone_NewRef1)
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__set_getlast__empty(DeeObject *__restrict self);
#define default__set_getlast__with__seq_operator_size__and__seq_operator_getitem_index_ab (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&default__seq_getlast__with__seq_operator_size__and__seq_operator_getitem_index)
#define default__set_getlast__with__seq_operator_sizeob__and__seq_operator_getitem_ab (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&default__seq_getlast__with__seq_operator_sizeob__and__seq_operator_getitem)
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__set_getlast__with__set_trygetlast(DeeObject *__restrict self);

/* set_boundlast */
INTDEF WUNUSED NONNULL((1)) int DCALL default__set_boundlast(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__set_boundlast__with_callattr_last(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__set_boundlast__with_callattr___set_last__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__set_boundlast__with_callobjectcache___set_last__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__set_boundlast__with_callobjectcache___set_last__(DeeTypeObject *tp_self, DeeObject *self);
#define default__set_boundlast__unsupported (*(int (DCALL *)(DeeObject *__restrict))&default__set_boundlast__empty)
#define default__set_boundlast__none (*(int (DCALL *)(DeeObject *__restrict))&_DeeNone_reti1_1)
#define default__set_boundlast__empty (*(int (DCALL *)(DeeObject *__restrict))&_DeeNone_reti0_1)
#define default__set_boundlast__with__set_operator_bool (*(int (DCALL *)(DeeObject *__restrict))&default__seq_boundlast__with__set_operator_bool)

/* set_dellast */
INTDEF WUNUSED NONNULL((1)) int DCALL default__set_dellast(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__set_dellast__with_callattr_last(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__set_dellast__with_callattr___set_last__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__set_dellast__with_callobjectcache___set_last__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__set_dellast__with_callobjectcache___set_last__(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__set_dellast__unsupported(DeeObject *__restrict self);
#define default__set_dellast__empty (*(int (DCALL *)(DeeObject *__restrict))&_DeeNone_reti0_1)
#define default__set_dellast__with__seq_operator_size__and__seq_operator_delitem_index_ab (*(int (DCALL *)(DeeObject *__restrict))&default__seq_dellast__with__seq_operator_size__and__seq_operator_delitem_index)
INTDEF WUNUSED NONNULL((1)) int DCALL default__set_dellast__with__seq_operator_size__and__seq_operator_bounditem_index__and__seq_operator_delitem_index(DeeObject *__restrict self);
#define default__set_dellast__with__seq_operator_sizeob__and__seq_operator_delitem_ab (*(int (DCALL *)(DeeObject *__restrict))&default__seq_dellast__with__seq_operator_sizeob__and__seq_operator_delitem)
INTDEF WUNUSED NONNULL((1)) int DCALL default__set_dellast__with__seq_operator_sizeob__and__seq_operator_bounditem__and__seq_operator_delitem(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__set_dellast__with__set_trygetlast__set_remove(DeeObject *__restrict self);

/* set_setlast */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__set_setlast(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__set_setlast__with_callattr_last(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__set_setlast__with_callattr___set_last__(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__set_setlast__with_callobjectcache___set_last__(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__set_setlast__with_callobjectcache___set_last__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__set_setlast__unsupported(DeeObject *self, DeeObject *value);
#define default__set_setlast__none (*(int (DCALL *)(DeeObject *, DeeObject *))&_DeeNone_reti0_2)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__set_setlast__empty(DeeObject *self, DeeObject *value);
#define default__set_setlast__with__seq_operator_size__and__seq_operator_setitem_index_ab (*(int (DCALL *)(DeeObject *, DeeObject *))&default__seq_setlast__with__seq_operator_size__and__seq_operator_setitem_index)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__set_setlast__with__seq_operator_size__and__seq_operator_bounditem_index__and__seq_operator_setitem_index(DeeObject *self, DeeObject *value);
#define default__set_setlast__with__seq_operator_sizeob__and__seq_operator_setitem_ab (*(int (DCALL *)(DeeObject *, DeeObject *))&default__seq_setlast__with__seq_operator_sizeob__and__seq_operator_setitem)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__set_setlast__with__seq_operator_sizeob__and__seq_operator_bounditem__and__seq_operator_setitem(DeeObject *self, DeeObject *value);

/* map_operator_iter */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_operator_iter(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_operator_iter__with_callattr___map_iter__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_operator_iter__with_callobjectcache___map_iter__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__map_operator_iter__with_callobjectcache___map_iter__(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_operator_iter__unsupported(DeeObject *__restrict self);
#define default__map_operator_iter__none (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&_DeeNone_NewRef1)
#define default__map_operator_iter__empty (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&default__seq_operator_iter__empty)
#define default__map_operator_iter__with__map_enumerate (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&default__seq_operator_iter__with__map_enumerate)
#define default__map_operator_iter__with__map_iterkeys__and__map_operator_trygetitem (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&default__seq_operator_iter__with__map_iterkeys__and__map_operator_trygetitem)
#define default__map_operator_iter__with__map_iterkeys__and__map_operator_getitem (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&default__seq_operator_iter__with__map_iterkeys__and__map_operator_getitem)
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_operator_iter__with__seq_operator_iter(DeeObject *__restrict self);

/* map_operator_foreach_pair */
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__map_operator_foreach_pair(DeeObject *__restrict self, Dee_foreach_pair_t cb, void *arg);
#define default__map_operator_foreach_pair__with_callattr___map_iter__ (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&default__map_operator_foreach_pair__with__map_operator_iter)
#define default__map_operator_foreach_pair__with_callobjectcache___map_iter__ (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&default__map_operator_foreach_pair__with__map_operator_iter)
#define default__map_operator_foreach_pair__unsupported (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&default__map_operator_foreach_pair__with__map_operator_iter)
#define default__map_operator_foreach_pair__empty (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&_DeeNone_rets0_3)
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__map_operator_foreach_pair__with__map_operator_iter(DeeObject *__restrict self, Dee_foreach_pair_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__map_operator_foreach_pair__with__seq_operator_foreach_pair(DeeObject *__restrict self, Dee_foreach_pair_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__map_operator_foreach_pair__with__map_enumerate(DeeObject *__restrict self, Dee_foreach_pair_t cb, void *arg);

/* map_operator_sizeob */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_operator_sizeob(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_operator_sizeob__with_callattr___map_size__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_operator_sizeob__with_callobjectcache___map_size__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__map_operator_sizeob__with_callobjectcache___map_size__(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_operator_sizeob__unsupported(DeeObject *__restrict self);
#define default__map_operator_sizeob__none (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&_DeeNone_NewRef1)
#define default__map_operator_sizeob__empty (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&default__seq_operator_sizeob__empty)
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_operator_sizeob__with__map_operator_size(DeeObject *__restrict self);

/* map_operator_size */
INTDEF WUNUSED NONNULL((1)) size_t DCALL default__map_operator_size(DeeObject *__restrict self);
#define default__map_operator_size__with_callattr___map_size__ (*(size_t (DCALL *)(DeeObject *__restrict))&default__map_operator_size__with__map_operator_sizeob)
#define default__map_operator_size__with_callobjectcache___map_size__ (*(size_t (DCALL *)(DeeObject *__restrict))&default__map_operator_size__with__map_operator_sizeob)
INTDEF WUNUSED NONNULL((1)) size_t DCALL default__map_operator_size__unsupported(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) size_t DCALL default__map_operator_size__with__map_operator_sizeob(DeeObject *__restrict self);
#define default__map_operator_size__none (*(size_t (DCALL *)(DeeObject *__restrict))&_DeeNone_rets0_1)
#define default__map_operator_size__empty (*(size_t (DCALL *)(DeeObject *__restrict))&default__seq_operator_size__empty)
INTDEF WUNUSED NONNULL((1)) size_t DCALL default__map_operator_size__with__map_operator_foreach_pair(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) size_t DCALL default__map_operator_size__with__map_operator_iter(DeeObject *__restrict self);

/* map_operator_hash */
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL default__map_operator_hash(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL default__map_operator_hash__with_callattr___map_hash__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL default__map_operator_hash__with_callobjectcache___map_hash__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) Dee_hash_t DCALL tdefault__map_operator_hash__with_callobjectcache___map_hash__(DeeTypeObject *tp_self, DeeObject *self);
#define default__map_operator_hash__unsupported (*(Dee_hash_t (DCALL *)(DeeObject *__restrict))&default__seq_operator_hash__unsupported)
#define default__map_operator_hash__none (*(Dee_hash_t (DCALL *)(DeeObject *__restrict))&_DeeNone_rets0_1)
#define default__map_operator_hash__empty (*(Dee_hash_t (DCALL *)(DeeObject *__restrict))&_DeeNone_rets0_1)
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL default__map_operator_hash__with__map_operator_foreach_pair(DeeObject *__restrict self);

/* map_operator_getitem */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_getitem(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_getitem__with_callattr___map_getitem__(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_getitem__with_callobjectcache___map_getitem__(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__map_operator_getitem__with_callobjectcache___map_getitem__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_getitem__unsupported(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_getitem__with__map_operator_getitem_index__and__map_operator_getitem_string_len_hash(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_getitem__with__map_operator_getitem_index__and__map_operator_getitem_string_hash(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_getitem__with__map_operator_getitem_string_len_hash(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_getitem__with__map_operator_getitem_string_hash(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_getitem__with__map_operator_getitem_index(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_getitem__with__map_operator_trygetitem__and__map_operator_hasitem(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_getitem__with__map_operator_trygetitem(DeeObject *self, DeeObject *key);
#define default__map_operator_getitem__none (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&_DeeNone_NewRef2)
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_getitem__empty(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_getitem__with__map_enumerate(DeeObject *self, DeeObject *key);

/* map_operator_trygetitem */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_trygetitem(DeeObject *self, DeeObject *key);
#define default__map_operator_trygetitem__with_callattr___map_getitem__ (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&default__map_operator_trygetitem__with__map_operator_getitem)
#define default__map_operator_trygetitem__with_callobjectcache___map_getitem__ (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&default__map_operator_trygetitem__with__map_operator_getitem)
#define default__map_operator_trygetitem__unsupported (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&default__map_operator_getitem__unsupported)
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_trygetitem__with__map_operator_trygetitem_index__and__map_operator_trygetitem_string_len_hash(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_trygetitem__with__map_operator_trygetitem_index__and__map_operator_trygetitem_string_hash(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_trygetitem__with__map_operator_trygetitem_string_len_hash(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_trygetitem__with__map_operator_trygetitem_string_hash(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_trygetitem__with__map_operator_trygetitem_index(DeeObject *self, DeeObject *key);
#define default__map_operator_trygetitem__none (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&_DeeNone_NewRef2)
#define default__map_operator_trygetitem__empty (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&_DeeNone_retsm1_2)
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_trygetitem__with__map_enumerate(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_trygetitem__with__map_operator_getitem(DeeObject *self, DeeObject *key);

/* map_operator_getitem_index */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_operator_getitem_index(DeeObject *self, size_t key);
#define default__map_operator_getitem_index__with_callattr___map_getitem__ (*(DREF DeeObject *(DCALL *)(DeeObject *, size_t))&default__map_operator_getitem_index__with__map_operator_getitem)
#define default__map_operator_getitem_index__with_callobjectcache___map_getitem__ (*(DREF DeeObject *(DCALL *)(DeeObject *, size_t))&default__map_operator_getitem_index__with__map_operator_getitem)
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_operator_getitem_index__unsupported(DeeObject *self, size_t key);
#define default__map_operator_getitem_index__none (*(DREF DeeObject *(DCALL *)(DeeObject *, size_t))&_DeeNone_NewRef2)
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_operator_getitem_index__empty(DeeObject *self, size_t key);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_operator_getitem_index__with__map_operator_getitem(DeeObject *self, size_t key);

/* map_operator_trygetitem_index */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_operator_trygetitem_index(DeeObject *self, size_t key);
#define default__map_operator_trygetitem_index__with_callattr___map_getitem__ (*(DREF DeeObject *(DCALL *)(DeeObject *, size_t))&default__map_operator_trygetitem_index__with__map_operator_trygetitem)
#define default__map_operator_trygetitem_index__with_callobjectcache___map_getitem__ (*(DREF DeeObject *(DCALL *)(DeeObject *, size_t))&default__map_operator_trygetitem_index__with__map_operator_trygetitem)
#define default__map_operator_trygetitem_index__unsupported (*(DREF DeeObject *(DCALL *)(DeeObject *, size_t))&default__map_operator_getitem_index__unsupported)
#define default__map_operator_trygetitem_index__none (*(DREF DeeObject *(DCALL *)(DeeObject *, size_t))&_DeeNone_NewRef2)
#define default__map_operator_trygetitem_index__empty (*(DREF DeeObject *(DCALL *)(DeeObject *, size_t))&_DeeNone_retsm1_2)
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_operator_trygetitem_index__with__map_operator_trygetitem(DeeObject *self, size_t key);

/* map_operator_getitem_string_hash */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_getitem_string_hash(DeeObject *self, char const *key, Dee_hash_t hash);
#define default__map_operator_getitem_string_hash__with_callattr___map_getitem__ (*(DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&default__map_operator_getitem_string_hash__with__map_operator_getitem)
#define default__map_operator_getitem_string_hash__with_callobjectcache___map_getitem__ (*(DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&default__map_operator_getitem_string_hash__with__map_operator_getitem)
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_getitem_string_hash__unsupported(DeeObject *self, char const *key, Dee_hash_t hash);
#define default__map_operator_getitem_string_hash__none (*(DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&_DeeNone_NewRef3)
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_getitem_string_hash__empty(DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_getitem_string_hash__with__map_enumerate(DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_getitem_string_hash__with__map_operator_getitem(DeeObject *self, char const *key, Dee_hash_t hash);

/* map_operator_trygetitem_string_hash */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_trygetitem_string_hash(DeeObject *self, char const *key, Dee_hash_t hash);
#define default__map_operator_trygetitem_string_hash__with_callattr___map_getitem__ (*(DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&default__map_operator_trygetitem_string_hash__with__map_operator_trygetitem)
#define default__map_operator_trygetitem_string_hash__with_callobjectcache___map_getitem__ (*(DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&default__map_operator_trygetitem_string_hash__with__map_operator_trygetitem)
#define default__map_operator_trygetitem_string_hash__unsupported (*(DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&default__map_operator_getitem_string_hash__unsupported)
#define default__map_operator_trygetitem_string_hash__none (*(DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&_DeeNone_NewRef3)
#define default__map_operator_trygetitem_string_hash__empty (*(DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&_DeeNone_retsm1_3)
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_trygetitem_string_hash__with__map_enumerate(DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_trygetitem_string_hash__with__map_operator_trygetitem(DeeObject *self, char const *key, Dee_hash_t hash);

/* map_operator_getitem_string_len_hash */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_operator_getitem_string_len_hash(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
#define default__map_operator_getitem_string_len_hash__with_callattr___map_getitem__ (*(DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&default__map_operator_getitem_string_len_hash__with__map_operator_getitem)
#define default__map_operator_getitem_string_len_hash__with_callobjectcache___map_getitem__ (*(DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&default__map_operator_getitem_string_len_hash__with__map_operator_getitem)
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_operator_getitem_string_len_hash__unsupported(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
#define default__map_operator_getitem_string_len_hash__none (*(DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&_DeeNone_NewRef4)
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_operator_getitem_string_len_hash__empty(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_operator_getitem_string_len_hash__with__map_enumerate(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_operator_getitem_string_len_hash__with__map_operator_getitem(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);

/* map_operator_trygetitem_string_len_hash */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_operator_trygetitem_string_len_hash(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
#define default__map_operator_trygetitem_string_len_hash__with_callattr___map_getitem__ (*(DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&default__map_operator_trygetitem_string_len_hash__with__map_operator_trygetitem)
#define default__map_operator_trygetitem_string_len_hash__with_callobjectcache___map_getitem__ (*(DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&default__map_operator_trygetitem_string_len_hash__with__map_operator_trygetitem)
#define default__map_operator_trygetitem_string_len_hash__unsupported (*(DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&default__map_operator_getitem_string_len_hash__unsupported)
#define default__map_operator_trygetitem_string_len_hash__none (*(DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&_DeeNone_NewRef4)
#define default__map_operator_trygetitem_string_len_hash__empty (*(DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&_DeeNone_retsm1_4)
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_operator_trygetitem_string_len_hash__with__map_enumerate(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_operator_trygetitem_string_len_hash__with__map_operator_trygetitem(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);

/* map_operator_bounditem */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_bounditem(DeeObject *self, DeeObject *key);
#define default__map_operator_bounditem__with_callattr___map_getitem__ (*(int (DCALL *)(DeeObject *, DeeObject *))&default__map_operator_bounditem__with__map_operator_getitem)
#define default__map_operator_bounditem__with_callobjectcache___map_getitem__ (*(int (DCALL *)(DeeObject *, DeeObject *))&default__map_operator_bounditem__with__map_operator_getitem)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_bounditem__unsupported(DeeObject *self, DeeObject *key);
#define default__map_operator_bounditem__none (*(int (DCALL *)(DeeObject *, DeeObject *))&_DeeNone_reti1_2)
#define default__map_operator_bounditem__empty (*(int (DCALL *)(DeeObject *, DeeObject *))&_DeeNone_reti0_2)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_bounditem__with__map_enumerate(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_bounditem__with__map_operator_contains(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_bounditem__with__map_operator_getitem(DeeObject *self, DeeObject *key);

/* map_operator_bounditem_index */
INTDEF WUNUSED NONNULL((1)) int DCALL default__map_operator_bounditem_index(DeeObject *self, size_t key);
#define default__map_operator_bounditem_index__with_callattr___map_getitem__ (*(int (DCALL *)(DeeObject *, size_t))&default__map_operator_bounditem_index__with__map_operator_getitem_index)
#define default__map_operator_bounditem_index__with_callobjectcache___map_getitem__ (*(int (DCALL *)(DeeObject *, size_t))&default__map_operator_bounditem_index__with__map_operator_getitem_index)
INTDEF WUNUSED NONNULL((1)) int DCALL default__map_operator_bounditem_index__unsupported(DeeObject *self, size_t key);
#define default__map_operator_bounditem_index__none (*(int (DCALL *)(DeeObject *, size_t))&_DeeNone_reti1_2)
#define default__map_operator_bounditem_index__empty (*(int (DCALL *)(DeeObject *, size_t))&_DeeNone_reti0_2)
INTDEF WUNUSED NONNULL((1)) int DCALL default__map_operator_bounditem_index__with__map_operator_bounditem(DeeObject *self, size_t key);
INTDEF WUNUSED NONNULL((1)) int DCALL default__map_operator_bounditem_index__with__map_operator_getitem_index(DeeObject *self, size_t key);

/* map_operator_bounditem_string_hash */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_bounditem_string_hash(DeeObject *self, char const *key, Dee_hash_t hash);
#define default__map_operator_bounditem_string_hash__with_callattr___map_getitem__ (*(int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&default__map_operator_bounditem_string_hash__with__map_operator_getitem_string_hash)
#define default__map_operator_bounditem_string_hash__with_callobjectcache___map_getitem__ (*(int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&default__map_operator_bounditem_string_hash__with__map_operator_getitem_string_hash)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_bounditem_string_hash__unsupported(DeeObject *self, char const *key, Dee_hash_t hash);
#define default__map_operator_bounditem_string_hash__none (*(int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&_DeeNone_reti1_3)
#define default__map_operator_bounditem_string_hash__empty (*(int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&_DeeNone_reti0_3)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_bounditem_string_hash__with__map_operator_bounditem(DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_bounditem_string_hash__with__map_operator_getitem_string_hash(DeeObject *self, char const *key, Dee_hash_t hash);

/* map_operator_bounditem_string_len_hash */
INTDEF WUNUSED NONNULL((1)) int DCALL default__map_operator_bounditem_string_len_hash(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
#define default__map_operator_bounditem_string_len_hash__with_callattr___map_getitem__ (*(int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&default__map_operator_bounditem_string_len_hash__with__map_operator_getitem_string_len_hash)
#define default__map_operator_bounditem_string_len_hash__with_callobjectcache___map_getitem__ (*(int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&default__map_operator_bounditem_string_len_hash__with__map_operator_getitem_string_len_hash)
INTDEF WUNUSED NONNULL((1)) int DCALL default__map_operator_bounditem_string_len_hash__unsupported(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
#define default__map_operator_bounditem_string_len_hash__none (*(int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&_DeeNone_reti1_4)
#define default__map_operator_bounditem_string_len_hash__empty (*(int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&_DeeNone_reti0_4)
INTDEF WUNUSED NONNULL((1)) int DCALL default__map_operator_bounditem_string_len_hash__with__map_operator_bounditem(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1)) int DCALL default__map_operator_bounditem_string_len_hash__with__map_operator_getitem_string_len_hash(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);

/* map_operator_hasitem */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_hasitem(DeeObject *self, DeeObject *key);
#define default__map_operator_hasitem__with_callattr___map_getitem__ (*(int (DCALL *)(DeeObject *, DeeObject *))&default__map_operator_bounditem__with__map_operator_getitem)
#define default__map_operator_hasitem__with_callobjectcache___map_getitem__ (*(int (DCALL *)(DeeObject *, DeeObject *))&default__map_operator_bounditem__with__map_operator_getitem)
#define default__map_operator_hasitem__unsupported (*(int (DCALL *)(DeeObject *, DeeObject *))&default__map_operator_bounditem__with__map_operator_getitem)
#define default__map_operator_hasitem__none (*(int (DCALL *)(DeeObject *, DeeObject *))&_DeeNone_reti1_2)
#define default__map_operator_hasitem__empty (*(int (DCALL *)(DeeObject *, DeeObject *))&_DeeNone_reti0_2)

/* map_operator_hasitem_index */
INTDEF WUNUSED NONNULL((1)) int DCALL default__map_operator_hasitem_index(DeeObject *self, size_t key);
#define default__map_operator_hasitem_index__with_callattr___map_getitem__ (*(int (DCALL *)(DeeObject *, size_t))&default__map_operator_bounditem_index__with__map_operator_getitem_index)
#define default__map_operator_hasitem_index__with_callobjectcache___map_getitem__ (*(int (DCALL *)(DeeObject *, size_t))&default__map_operator_bounditem_index__with__map_operator_getitem_index)
#define default__map_operator_hasitem_index__unsupported (*(int (DCALL *)(DeeObject *, size_t))&default__map_operator_bounditem_index__with__map_operator_getitem_index)
#define default__map_operator_hasitem_index__none (*(int (DCALL *)(DeeObject *, size_t))&_DeeNone_reti1_2)
#define default__map_operator_hasitem_index__empty (*(int (DCALL *)(DeeObject *, size_t))&_DeeNone_reti0_2)

/* map_operator_hasitem_string_hash */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_hasitem_string_hash(DeeObject *self, char const *key, Dee_hash_t hash);
#define default__map_operator_hasitem_string_hash__with_callattr___map_getitem__ (*(int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&default__map_operator_bounditem_string_hash__with__map_operator_getitem_string_hash)
#define default__map_operator_hasitem_string_hash__with_callobjectcache___map_getitem__ (*(int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&default__map_operator_bounditem_string_hash__with__map_operator_getitem_string_hash)
#define default__map_operator_hasitem_string_hash__unsupported (*(int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&default__map_operator_bounditem_string_hash__with__map_operator_getitem_string_hash)
#define default__map_operator_hasitem_string_hash__none (*(int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&_DeeNone_reti1_3)
#define default__map_operator_hasitem_string_hash__empty (*(int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&_DeeNone_reti0_3)

/* map_operator_hasitem_string_len_hash */
INTDEF WUNUSED NONNULL((1)) int DCALL default__map_operator_hasitem_string_len_hash(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
#define default__map_operator_hasitem_string_len_hash__with_callattr___map_getitem__ (*(int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&default__map_operator_bounditem_string_len_hash__with__map_operator_getitem_string_len_hash)
#define default__map_operator_hasitem_string_len_hash__with_callobjectcache___map_getitem__ (*(int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&default__map_operator_bounditem_string_len_hash__with__map_operator_getitem_string_len_hash)
#define default__map_operator_hasitem_string_len_hash__unsupported (*(int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&default__map_operator_bounditem_string_len_hash__with__map_operator_getitem_string_len_hash)
#define default__map_operator_hasitem_string_len_hash__none (*(int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&_DeeNone_reti1_4)
#define default__map_operator_hasitem_string_len_hash__empty (*(int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&_DeeNone_reti0_4)

/* map_operator_delitem */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_delitem(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_delitem__with_callattr___map_delitem__(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_delitem__with_callobjectcache___map_delitem__(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__map_operator_delitem__with_callobjectcache___map_delitem__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_delitem__unsupported(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_delitem__with__map_operator_delitem_index__and__map_operator_delitem_string_len_hash(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_delitem__with__map_operator_delitem_index__and__map_operator_delitem_string_hash(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_delitem__with__map_operator_delitem_string_len_hash(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_delitem__with__map_operator_delitem_string_hash(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_delitem__with__map_operator_delitem_index(DeeObject *self, DeeObject *key);
#define default__map_operator_delitem__empty (*(int (DCALL *)(DeeObject *, DeeObject *))&_DeeNone_reti0_2)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_delitem__with__map_remove(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_delitem__with__map_pop_with_default(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_delitem__with__map_removekeys(DeeObject *self, DeeObject *key);

/* map_operator_delitem_index */
INTDEF WUNUSED NONNULL((1)) int DCALL default__map_operator_delitem_index(DeeObject *self, size_t key);
#define default__map_operator_delitem_index__with_callattr___map_delitem__ (*(int (DCALL *)(DeeObject *, size_t))&default__map_operator_delitem_index__with__map_operator_delitem)
#define default__map_operator_delitem_index__with_callobjectcache___map_delitem__ (*(int (DCALL *)(DeeObject *, size_t))&default__map_operator_delitem_index__with__map_operator_delitem)
INTDEF WUNUSED NONNULL((1)) int DCALL default__map_operator_delitem_index__unsupported(DeeObject *self, size_t key);
#define default__map_operator_delitem_index__empty (*(int (DCALL *)(DeeObject *, size_t))&_DeeNone_reti0_2)
INTDEF WUNUSED NONNULL((1)) int DCALL default__map_operator_delitem_index__with__map_operator_delitem(DeeObject *self, size_t key);

/* map_operator_delitem_string_hash */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_delitem_string_hash(DeeObject *self, char const *key, Dee_hash_t hash);
#define default__map_operator_delitem_string_hash__with_callattr___map_delitem__ (*(int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&default__map_operator_delitem_string_hash__with__map_operator_delitem)
#define default__map_operator_delitem_string_hash__with_callobjectcache___map_delitem__ (*(int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&default__map_operator_delitem_string_hash__with__map_operator_delitem)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_delitem_string_hash__unsupported(DeeObject *self, char const *key, Dee_hash_t hash);
#define default__map_operator_delitem_string_hash__empty (*(int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&_DeeNone_reti0_3)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_delitem_string_hash__with__map_operator_delitem(DeeObject *self, char const *key, Dee_hash_t hash);

/* map_operator_delitem_string_len_hash */
INTDEF WUNUSED NONNULL((1)) int DCALL default__map_operator_delitem_string_len_hash(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
#define default__map_operator_delitem_string_len_hash__with_callattr___map_delitem__ (*(int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&default__map_operator_delitem_string_len_hash__with__map_operator_delitem)
#define default__map_operator_delitem_string_len_hash__with_callobjectcache___map_delitem__ (*(int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&default__map_operator_delitem_string_len_hash__with__map_operator_delitem)
INTDEF WUNUSED NONNULL((1)) int DCALL default__map_operator_delitem_string_len_hash__unsupported(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
#define default__map_operator_delitem_string_len_hash__empty (*(int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&_DeeNone_reti0_4)
INTDEF WUNUSED NONNULL((1)) int DCALL default__map_operator_delitem_string_len_hash__with__map_operator_delitem(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);

/* map_operator_setitem */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__map_operator_setitem(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__map_operator_setitem__with_callattr___map_setitem__(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__map_operator_setitem__with_callobjectcache___map_setitem__(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL tdefault__map_operator_setitem__with_callobjectcache___map_setitem__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__map_operator_setitem__unsupported(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__map_operator_setitem__with__map_operator_setitem_index__and__map_operator_setitem_string_len_hash(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__map_operator_setitem__with__map_operator_setitem_index__and__map_operator_setitem_string_hash(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__map_operator_setitem__with__map_operator_setitem_string_len_hash(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__map_operator_setitem__with__map_operator_setitem_string_hash(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__map_operator_setitem__with__map_operator_setitem_index(DeeObject *self, DeeObject *key, DeeObject *value);
#define default__map_operator_setitem__none (*(int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&_DeeNone_reti0_3)
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__map_operator_setitem__empty(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__map_operator_setitem__with__seq_enumerate__and__seq_operator_setitem__and__seq_append(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__map_operator_setitem__with__seq_enumerate_index__and__seq_operator_setitem_index__and__seq_append(DeeObject *self, DeeObject *key, DeeObject *value);

/* map_operator_setitem_index */
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__map_operator_setitem_index(DeeObject *self, size_t key, DeeObject *value);
#define default__map_operator_setitem_index__with_callattr___map_setitem__ (*(int (DCALL *)(DeeObject *, size_t, DeeObject *))&default__map_operator_setitem_index__with__map_operator_setitem)
#define default__map_operator_setitem_index__with_callobjectcache___map_setitem__ (*(int (DCALL *)(DeeObject *, size_t, DeeObject *))&default__map_operator_setitem_index__with__map_operator_setitem)
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__map_operator_setitem_index__unsupported(DeeObject *self, size_t key, DeeObject *value);
#define default__map_operator_setitem_index__none (*(int (DCALL *)(DeeObject *, size_t, DeeObject *))&_DeeNone_reti0_3)
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__map_operator_setitem_index__empty(DeeObject *self, size_t key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__map_operator_setitem_index__with__map_operator_setitem(DeeObject *self, size_t key, DeeObject *value);

/* map_operator_setitem_string_hash */
INTDEF WUNUSED NONNULL((1, 2, 4)) int DCALL default__map_operator_setitem_string_hash(DeeObject *self, char const *key, Dee_hash_t hash, DeeObject *value);
#define default__map_operator_setitem_string_hash__with_callattr___map_setitem__ (*(int (DCALL *)(DeeObject *, char const *, Dee_hash_t, DeeObject *))&default__map_operator_setitem_string_hash__with__map_operator_setitem)
#define default__map_operator_setitem_string_hash__with_callobjectcache___map_setitem__ (*(int (DCALL *)(DeeObject *, char const *, Dee_hash_t, DeeObject *))&default__map_operator_setitem_string_hash__with__map_operator_setitem)
INTDEF WUNUSED NONNULL((1, 2, 4)) int DCALL default__map_operator_setitem_string_hash__unsupported(DeeObject *self, char const *key, Dee_hash_t hash, DeeObject *value);
#define default__map_operator_setitem_string_hash__none (*(int (DCALL *)(DeeObject *, char const *, Dee_hash_t, DeeObject *))&_DeeNone_reti0_4)
INTDEF WUNUSED NONNULL((1, 2, 4)) int DCALL default__map_operator_setitem_string_hash__empty(DeeObject *self, char const *key, Dee_hash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 4)) int DCALL default__map_operator_setitem_string_hash__with__map_operator_setitem(DeeObject *self, char const *key, Dee_hash_t hash, DeeObject *value);

/* map_operator_setitem_string_len_hash */
INTDEF WUNUSED NONNULL((1, 5)) int DCALL default__map_operator_setitem_string_len_hash(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash, DeeObject *value);
#define default__map_operator_setitem_string_len_hash__with_callattr___map_setitem__ (*(int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t, DeeObject *))&default__map_operator_setitem_string_len_hash__with__map_operator_setitem)
#define default__map_operator_setitem_string_len_hash__with_callobjectcache___map_setitem__ (*(int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t, DeeObject *))&default__map_operator_setitem_string_len_hash__with__map_operator_setitem)
INTDEF WUNUSED NONNULL((1, 5)) int DCALL default__map_operator_setitem_string_len_hash__unsupported(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash, DeeObject *value);
#define default__map_operator_setitem_string_len_hash__none (*(int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t, DeeObject *))&_DeeNone_reti0_5)
INTDEF WUNUSED NONNULL((1, 5)) int DCALL default__map_operator_setitem_string_len_hash__empty(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 5)) int DCALL default__map_operator_setitem_string_len_hash__with__map_operator_setitem(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash, DeeObject *value);

/* map_operator_contains */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_contains(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_contains__with_callattr___map_contains__(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_contains__with_callobjectcache___map_contains__(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__map_operator_contains__with_callobjectcache___map_contains__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_contains__unsupported(DeeObject *self, DeeObject *key);
#define default__map_operator_contains__none (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&_DeeNone_NewRef2)
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_contains__empty(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_contains__with__map_operator_trygetitem(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_contains__with__map_operator_bounditem(DeeObject *self, DeeObject *key);

/* map_keys */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_keys(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_keys__with_callattr_keys(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_keys__with_callattr___map_keys__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_keys__with_callobjectcache___map_keys__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__map_keys__with_callobjectcache___map_keys__(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_keys__unsupported(DeeObject *__restrict self);
#define default__map_keys__none (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&_DeeNone_NewRef1)
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_keys__empty(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_keys__with__map_iterkeys(DeeObject *__restrict self);

/* map_iterkeys */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_iterkeys(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_iterkeys__with_callattr_iterkeys(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_iterkeys__with_callattr___map_iterkeys__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_iterkeys__with_callobjectcache___map_iterkeys__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__map_iterkeys__with_callobjectcache___map_iterkeys__(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_iterkeys__unsupported(DeeObject *__restrict self);
#define default__map_iterkeys__none (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&_DeeNone_NewRef1)
#define default__map_iterkeys__empty (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&default__seq_operator_iter__empty)
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_iterkeys__with__map_keys(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_iterkeys__with__map_enumerate(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_iterkeys__with__map_operator_iter(DeeObject *__restrict self);

/* map_values */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_values(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_values__with_callattr_values(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_values__with_callattr___map_values__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_values__with_callobjectcache___map_values__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__map_values__with_callobjectcache___map_values__(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_values__unsupported(DeeObject *__restrict self);
#define default__map_values__none (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&_DeeNone_NewRef1)
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_values__empty(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_values__with__map_itervalues(DeeObject *__restrict self);

/* map_itervalues */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_itervalues(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_itervalues__with_callattr_itervalues(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_itervalues__with_callattr___map_itervalues__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_itervalues__with_callobjectcache___map_itervalues__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__map_itervalues__with_callobjectcache___map_itervalues__(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_itervalues__unsupported(DeeObject *__restrict self);
#define default__map_itervalues__none (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&_DeeNone_NewRef1)
#define default__map_itervalues__empty (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&default__seq_operator_iter__empty)
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_itervalues__with__map_values(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_itervalues__with__map_operator_iter(DeeObject *__restrict self);

/* map_enumerate */
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__map_enumerate__with_callattr___map_enumerate__(DeeObject *__restrict self, Dee_seq_enumerate_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__map_enumerate__with_callobjectcache___map_enumerate__(DeeObject *__restrict self, Dee_seq_enumerate_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL tdefault__map_enumerate__with_callobjectcache___map_enumerate__(DeeTypeObject *tp_self, DeeObject *self, Dee_seq_enumerate_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__map_enumerate__unsupported(DeeObject *__restrict self, Dee_seq_enumerate_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__map_enumerate__with__map_enumerate_range(DeeObject *__restrict self, Dee_seq_enumerate_t cb, void *arg);
#define default__map_enumerate__empty (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_seq_enumerate_t, void *))&default__map_operator_foreach_pair__empty)
#define default__map_enumerate__with__seq_operator_foreach_pair (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_seq_enumerate_t, void *))&default__map_operator_foreach_pair__with__seq_operator_foreach_pair)
#define default__map_enumerate__with__map_operator_iter (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_seq_enumerate_t, void *))&default__map_operator_foreach_pair__with__map_operator_iter)
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__map_enumerate__with__map_iterkeys__and__map_operator_trygetitem(DeeObject *__restrict self, Dee_seq_enumerate_t cb, void *arg);

/* map_enumerate_range */
INTDEF WUNUSED NONNULL((1, 2, 4, 5)) Dee_ssize_t DCALL default__map_enumerate_range__with_callattr___map_enumerate__(DeeObject *self, Dee_seq_enumerate_t cb, void *arg, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 4, 5)) Dee_ssize_t DCALL default__map_enumerate_range__with_callobjectcache___map_enumerate__(DeeObject *self, Dee_seq_enumerate_t cb, void *arg, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3, 5, 6)) Dee_ssize_t DCALL tdefault__map_enumerate_range__with_callobjectcache___map_enumerate__(DeeTypeObject *tp_self, DeeObject *self, Dee_seq_enumerate_t cb, void *arg, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 4, 5)) Dee_ssize_t DCALL default__map_enumerate_range__unsupported(DeeObject *self, Dee_seq_enumerate_t cb, void *arg, DeeObject *start, DeeObject *end);
#define default__map_enumerate_range__empty (*(Dee_ssize_t (DCALL *)(DeeObject *, Dee_seq_enumerate_t, void *, DeeObject *, DeeObject *))&_DeeNone_rets0_5)
INTDEF WUNUSED NONNULL((1, 2, 4, 5)) Dee_ssize_t DCALL default__map_enumerate_range__with__map_enumerate(DeeObject *self, Dee_seq_enumerate_t cb, void *arg, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 4, 5)) Dee_ssize_t DCALL default__map_enumerate_range__with__map_iterkeys__and__map_operator_trygetitem(DeeObject *self, Dee_seq_enumerate_t cb, void *arg, DeeObject *start, DeeObject *end);

/* map_makeenumeration */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_makeenumeration__with_callattr___map_enumerate_items__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_makeenumeration__with_callobjectcache___map_enumerate_items__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__map_makeenumeration__with_callobjectcache___map_enumerate_items__(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_makeenumeration__unsupported(DeeObject *__restrict self);
#define default__map_makeenumeration__none (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&default__seq_makeenumeration__none)
#define default__map_makeenumeration__empty (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&default__seq_makeenumeration__empty)
#define default__map_makeenumeration__with__operator_iter (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&DeeObject_NewRef)
#define default__map_makeenumeration__with__map_operator_iter (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&generic_obj__asmap)
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_makeenumeration__with__map_iterkeys__and__map_operator_getitem(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_makeenumeration__with__map_iterkeys__and__map_operator_trygetitem(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_makeenumeration__with__map_enumerate(DeeObject *__restrict self);

/* map_makeenumeration_with_range */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__map_makeenumeration_with_range__with_callattr___map_enumerate_items__(DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__map_makeenumeration_with_range__with_callobjectcache___map_enumerate_items__(DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *DCALL tdefault__map_makeenumeration_with_range__with_callobjectcache___map_enumerate_items__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__map_makeenumeration_with_range__unsupported(DeeObject *self, DeeObject *start, DeeObject *end);
#define default__map_makeenumeration_with_range__none (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *))&default__seq_makeenumeration_with_range__none)
#define default__map_makeenumeration_with_range__empty (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *))&default__seq_makeenumeration_with_range__empty)
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__map_makeenumeration_with_range__with__map_operator_iter(DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__map_makeenumeration_with_range__with__map_iterkeys__and__map_operator_getitem(DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__map_makeenumeration_with_range__with__map_iterkeys__and__map_operator_trygetitem(DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__map_makeenumeration_with_range__with__map_enumerate_range(DeeObject *self, DeeObject *start, DeeObject *end);

/* map_operator_compare_eq */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_compare_eq(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_compare_eq__with_callattr_equals(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_compare_eq__with_callattr___map_compare_eq__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_compare_eq__with_callobjectcache___map_compare_eq__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__map_operator_compare_eq__with_callobjectcache___map_compare_eq__(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_compare_eq__unsupported(DeeObject *lhs, DeeObject *rhs);
#define default__map_operator_compare_eq__empty (*(int (DCALL *)(DeeObject *, DeeObject *))&default__seq_operator_compare_eq__empty)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_compare_eq__with__map_operator_eq(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_compare_eq__with__map_operator_ne(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_compare_eq__with__map_operator_foreach_pair(DeeObject *lhs, DeeObject *rhs);

/* map_operator_trycompare_eq */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_trycompare_eq(DeeObject *lhs, DeeObject *rhs);
#define default__map_operator_trycompare_eq__with_callattr_equals (*(int (DCALL *)(DeeObject *, DeeObject *))&default__map_operator_trycompare_eq__with__map_operator_compare_eq)
#define default__map_operator_trycompare_eq__with_callattr___map_compare_eq__ (*(int (DCALL *)(DeeObject *, DeeObject *))&default__map_operator_trycompare_eq__with__map_operator_compare_eq)
#define default__map_operator_trycompare_eq__with_callobjectcache___map_compare_eq__ (*(int (DCALL *)(DeeObject *, DeeObject *))&default__map_operator_trycompare_eq__with__map_operator_compare_eq)
#define default__map_operator_trycompare_eq__unsupported (*(int (DCALL *)(DeeObject *, DeeObject *))&_DeeNone_reti1_2)
#define default__map_operator_trycompare_eq__empty (*(int (DCALL *)(DeeObject *, DeeObject *))&default__seq_operator_trycompare_eq__empty)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_trycompare_eq__with__map_operator_compare_eq(DeeObject *lhs, DeeObject *rhs);

/* map_operator_eq */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_eq(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_eq__with_callattr___map_eq__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_eq__with_callobjectcache___map_eq__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__map_operator_eq__with_callobjectcache___map_eq__(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_eq__unsupported(DeeObject *lhs, DeeObject *rhs);
#define default__map_operator_eq__empty (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&default__map_operator_eq__with__map_operator_compare_eq)
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_eq__with__map_operator_compare_eq(DeeObject *lhs, DeeObject *rhs);

/* map_operator_ne */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_ne(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_ne__with_callattr___map_ne__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_ne__with_callobjectcache___map_ne__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__map_operator_ne__with_callobjectcache___map_ne__(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_ne__unsupported(DeeObject *lhs, DeeObject *rhs);
#define default__map_operator_ne__empty (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&default__map_operator_ne__with__map_operator_compare_eq)
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_ne__with__map_operator_compare_eq(DeeObject *lhs, DeeObject *rhs);

/* map_operator_lo */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_lo(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_lo__with_callattr___map_lo__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_lo__with_callobjectcache___map_lo__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__map_operator_lo__with_callobjectcache___map_lo__(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_lo__unsupported(DeeObject *lhs, DeeObject *rhs);
#define default__map_operator_lo__empty (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&default__set_operator_lo__empty)
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_lo__with__map_operator_ge(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_lo__with__map_operator_foreach_pair(DeeObject *lhs, DeeObject *rhs);

/* map_operator_le */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_le(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_le__with_callattr___map_le__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_le__with_callobjectcache___map_le__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__map_operator_le__with_callobjectcache___map_le__(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_le__unsupported(DeeObject *lhs, DeeObject *rhs);
#define default__map_operator_le__empty (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&default__set_operator_le__empty)
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_le__with__map_operator_gr(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_le__with__map_operator_foreach_pair(DeeObject *lhs, DeeObject *rhs);

/* map_operator_gr */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_gr(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_gr__with_callattr___map_gr__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_gr__with_callobjectcache___map_gr__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__map_operator_gr__with_callobjectcache___map_gr__(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_gr__unsupported(DeeObject *lhs, DeeObject *rhs);
#define default__map_operator_gr__empty (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&default__set_operator_gr__empty)
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_gr__with__map_operator_le(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_gr__with__map_operator_foreach_pair(DeeObject *lhs, DeeObject *rhs);

/* map_operator_ge */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_ge(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_ge__with_callattr___map_ge__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_ge__with_callobjectcache___map_ge__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__map_operator_ge__with_callobjectcache___map_ge__(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_ge__unsupported(DeeObject *lhs, DeeObject *rhs);
#define default__map_operator_ge__empty (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&default__set_operator_ge__empty)
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_ge__with__map_operator_lo(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_ge__with__map_operator_foreach_pair(DeeObject *lhs, DeeObject *rhs);

/* map_operator_add */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_add(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_add__with_callattr_union(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_add__with_callattr___map_add__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_add__with_callobjectcache___map_add__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__map_operator_add__with_callobjectcache___map_add__(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_add__unsupported(DeeObject *lhs, DeeObject *rhs);
#define default__map_operator_add__none (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&_DeeNone_NewRef2)
#define default__map_operator_add__empty (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&default__map_operator_add__unsupported)

/* map_operator_sub */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_sub(DeeObject *lhs, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_sub__with_callattr_difference(DeeObject *lhs, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_sub__with_callattr___map_sub__(DeeObject *lhs, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_sub__with_callobjectcache___map_sub__(DeeObject *lhs, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__map_operator_sub__with_callobjectcache___map_sub__(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_sub__unsupported(DeeObject *lhs, DeeObject *keys);
#define default__map_operator_sub__none (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&_DeeNone_NewRef2)
#define default__map_operator_sub__empty (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&default__map_operator_sub__unsupported)

/* map_operator_and */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_and(DeeObject *lhs, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_and__with_callattr_intersection(DeeObject *lhs, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_and__with_callattr___map_and__(DeeObject *lhs, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_and__with_callobjectcache___map_and__(DeeObject *lhs, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__map_operator_and__with_callobjectcache___map_and__(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_and__unsupported(DeeObject *lhs, DeeObject *keys);
#define default__map_operator_and__none (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&_DeeNone_NewRef2)
#define default__map_operator_and__empty (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&default__map_operator_and__unsupported)

/* map_operator_xor */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_xor(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_xor__with_callattr_symmetric_difference(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_xor__with_callattr___map_xor__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_xor__with_callobjectcache___map_xor__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__map_operator_xor__with_callobjectcache___map_xor__(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_xor__unsupported(DeeObject *lhs, DeeObject *rhs);
#define default__map_operator_xor__none (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&_DeeNone_NewRef2)
#define default__map_operator_xor__empty (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&default__map_operator_xor__unsupported)

/* map_operator_inplace_add */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_inplace_add(DREF DeeObject **__restrict p_self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_inplace_add__with_callattr___map_inplace_add__(DREF DeeObject **__restrict p_self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_inplace_add__with_callobjectcache___map_inplace_add__(DREF DeeObject **__restrict p_self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__map_operator_inplace_add__with_callobjectcache___map_inplace_add__(DeeTypeObject *tp_self, DREF DeeObject **p_self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_inplace_add__unsupported(DREF DeeObject **__restrict p_self, DeeObject *items);
#define default__map_operator_inplace_add__none (*(int (DCALL *)(DREF DeeObject **__restrict, DeeObject *))&_DeeNone_reti0_2)
#define default__map_operator_inplace_add__empty (*(int (DCALL *)(DREF DeeObject **__restrict, DeeObject *))&default__map_operator_inplace_add__unsupported)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_inplace_add__with__map_update(DREF DeeObject **__restrict p_self, DeeObject *items);

/* map_operator_inplace_sub */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_inplace_sub(DREF DeeObject **__restrict p_self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_inplace_sub__with_callattr___map_inplace_sub__(DREF DeeObject **__restrict p_self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_inplace_sub__with_callobjectcache___map_inplace_sub__(DREF DeeObject **__restrict p_self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__map_operator_inplace_sub__with_callobjectcache___map_inplace_sub__(DeeTypeObject *tp_self, DREF DeeObject **p_self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_inplace_sub__unsupported(DREF DeeObject **__restrict p_self, DeeObject *keys);
#define default__map_operator_inplace_sub__none (*(int (DCALL *)(DREF DeeObject **__restrict, DeeObject *))&_DeeNone_reti0_2)
#define default__map_operator_inplace_sub__empty (*(int (DCALL *)(DREF DeeObject **__restrict, DeeObject *))&default__map_operator_inplace_sub__unsupported)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_inplace_sub__with__map_removekeys(DREF DeeObject **__restrict p_self, DeeObject *keys);

/* map_operator_inplace_and */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_inplace_and(DREF DeeObject **__restrict p_self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_inplace_and__with_callattr___map_inplace_and__(DREF DeeObject **__restrict p_self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_inplace_and__with_callobjectcache___map_inplace_and__(DREF DeeObject **__restrict p_self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__map_operator_inplace_and__with_callobjectcache___map_inplace_and__(DeeTypeObject *tp_self, DREF DeeObject **p_self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_inplace_and__unsupported(DREF DeeObject **__restrict p_self, DeeObject *keys);
#define default__map_operator_inplace_and__none (*(int (DCALL *)(DREF DeeObject **__restrict, DeeObject *))&_DeeNone_reti0_2)
#define default__map_operator_inplace_and__empty (*(int (DCALL *)(DREF DeeObject **__restrict, DeeObject *))&default__map_operator_inplace_and__unsupported)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_inplace_and__with__map_operator_foreach_pair__and__map_removekeys(DREF DeeObject **__restrict p_self, DeeObject *keys);

/* map_operator_inplace_xor */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_inplace_xor(DREF DeeObject **__restrict p_self, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_inplace_xor__with_callattr___map_inplace_xor__(DREF DeeObject **__restrict p_self, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_inplace_xor__with_callobjectcache___map_inplace_xor__(DREF DeeObject **__restrict p_self, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__map_operator_inplace_xor__with_callobjectcache___map_inplace_xor__(DeeTypeObject *tp_self, DREF DeeObject **p_self, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_inplace_xor__unsupported(DREF DeeObject **__restrict p_self, DeeObject *rhs);
#define default__map_operator_inplace_xor__none (*(int (DCALL *)(DREF DeeObject **__restrict, DeeObject *))&_DeeNone_reti0_2)
#define default__map_operator_inplace_xor__empty (*(int (DCALL *)(DREF DeeObject **__restrict, DeeObject *))&default__map_operator_inplace_xor__unsupported)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_inplace_xor__with__map_operator_foreach_pair__and__map_update__and__map_removekeys(DREF DeeObject **__restrict p_self, DeeObject *rhs);

/* map_frozen */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_frozen(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_frozen__with_callattr_frozen(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_frozen__with_callattr___map_frozen__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_frozen__with_callobjectcache___map_frozen__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__map_frozen__with_callobjectcache___map_frozen__(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_frozen__unsupported(DeeObject *__restrict self);
#define default__map_frozen__none (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&_DeeNone_NewRef1)
#define default__map_frozen__empty (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&DeeObject_NewRef)
#define default__map_frozen__with__map_operator_foreach_pair (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&DeeRoDict_FromSequence)

/* map_setold */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__map_setold__with_callattr_setold(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__map_setold__with_callattr___map_setold__(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__map_setold__with_callobjectcache___map_setold__(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL tdefault__map_setold__with_callobjectcache___map_setold__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__map_setold__unsupported(DeeObject *self, DeeObject *key, DeeObject *value);
#define default__map_setold__none (*(int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&_DeeNone_reti0_3)
#define default__map_setold__empty (*(int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&default__map_setold__unsupported)
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__map_setold__with__map_setold_ex(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__map_setold__with__map_operator_trygetitem__and__map_operator_setitem(DeeObject *self, DeeObject *key, DeeObject *value);

/* map_setold_ex */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__map_setold_ex__with_callattr_setold_ex(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__map_setold_ex__with_callattr___map_setold_ex__(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__map_setold_ex__with_callobjectcache___map_setold_ex__(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *DCALL tdefault__map_setold_ex__with_callobjectcache___map_setold_ex__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__map_setold_ex__unsupported(DeeObject *self, DeeObject *key, DeeObject *value);
#define default__map_setold_ex__none (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *))&_DeeNone_NewRef3)
#define default__map_setold_ex__empty (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *))&default__map_setold_ex__unsupported)
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__map_setold_ex__with__map_operator_trygetitem__and__map_setold(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__map_setold_ex__with__map_operator_trygetitem__and__map_operator_setitem(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__map_setold_ex__with__seq_enumerate__and__seq_operator_setitem(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__map_setold_ex__with__seq_enumerate_index__and__seq_operator_setitem_index(DeeObject *self, DeeObject *key, DeeObject *value);

/* map_setnew */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__map_setnew__with_callattr_setnew(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__map_setnew__with_callattr___map_setnew__(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__map_setnew__with_callobjectcache___map_setnew__(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL tdefault__map_setnew__with_callobjectcache___map_setnew__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__map_setnew__unsupported(DeeObject *self, DeeObject *key, DeeObject *value);
#define default__map_setnew__none (*(int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&_DeeNone_reti0_3)
#define default__map_setnew__empty (*(int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&default__map_setnew__unsupported)
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__map_setnew__with__map_setnew_ex(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__map_setnew__with__map_operator_trygetitem__and__map_setdefault(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__map_setnew__with__map_operator_trygetitem__and__map_operator_setitem(DeeObject *self, DeeObject *key, DeeObject *value);

/* map_setnew_ex */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__map_setnew_ex__with_callattr_setnew_ex(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__map_setnew_ex__with_callattr___map_setnew_ex__(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__map_setnew_ex__with_callobjectcache___map_setnew_ex__(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *DCALL tdefault__map_setnew_ex__with_callobjectcache___map_setnew_ex__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__map_setnew_ex__unsupported(DeeObject *self, DeeObject *key, DeeObject *value);
#define default__map_setnew_ex__none (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *))&_DeeNone_NewRef3)
#define default__map_setnew_ex__empty (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *))&default__map_setnew_ex__unsupported)
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__map_setnew_ex__with__map_operator_trygetitem__and__map_setnew(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__map_setnew_ex__with__map_operator_trygetitem__and__map_setdefault(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__map_setnew_ex__with__map_operator_trygetitem__and__map_operator_setitem(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__map_setnew_ex__with__map_operator_trygetitem__and__seq_append(DeeObject *self, DeeObject *key, DeeObject *value);

/* map_setdefault */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__map_setdefault__with_callattr_setdefault(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__map_setdefault__with_callattr___map_setdefault__(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__map_setdefault__with_callobjectcache___map_setdefault__(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *DCALL tdefault__map_setdefault__with_callobjectcache___map_setdefault__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__map_setdefault__unsupported(DeeObject *self, DeeObject *key, DeeObject *value);
#define default__map_setdefault__none (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *))&_DeeNone_NewRef3)
#define default__map_setdefault__empty (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *))&default__map_setdefault__unsupported)
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__map_setdefault__with__map_setnew_ex(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__map_setdefault__with__map_setnew__and__map_operator_getitem(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__map_setdefault__with__map_operator_trygetitem__and__map_operator_setitem(DeeObject *self, DeeObject *key, DeeObject *value);

/* map_update */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_update__with_callattr_update(DeeObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_update__with_callattr___map_update__(DeeObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_update__with_callobjectcache___map_update__(DeeObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__map_update__with_callobjectcache___map_update__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_update__unsupported(DeeObject *self, DeeObject *items);
#define default__map_update__none (*(int (DCALL *)(DeeObject *, DeeObject *))&_DeeNone_reti0_2)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_update__empty(DeeObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_update__with__map_operator_inplace_add(DeeObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_update__with__map_operator_setitem(DeeObject *self, DeeObject *items);

/* map_remove */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_remove__with_callattr_remove(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_remove__with_callattr___map_remove__(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_remove__with_callobjectcache___map_remove__(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__map_remove__with_callobjectcache___map_remove__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_remove__unsupported(DeeObject *self, DeeObject *key);
#define default__map_remove__none (*(int (DCALL *)(DeeObject *, DeeObject *))&_DeeNone_reti0_2)
#define default__map_remove__empty (*(int (DCALL *)(DeeObject *, DeeObject *))&default__map_remove__unsupported)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_remove__with__map_operator_bounditem__and__map_operator_delitem(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_remove__with__seq_operator_size__and__map_operator_delitem(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_remove__with__seq_enumerate__and__seq_operator_delitem(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_remove__with__seq_enumerate_index__and__seq_operator_delitem_index(DeeObject *self, DeeObject *key);

/* map_removekeys */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_removekeys__with_callattr_removekeys(DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_removekeys__with_callattr___map_removekeys__(DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_removekeys__with_callobjectcache___map_removekeys__(DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__map_removekeys__with_callobjectcache___map_removekeys__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_removekeys__unsupported(DeeObject *self, DeeObject *keys);
#define default__map_removekeys__none (*(int (DCALL *)(DeeObject *, DeeObject *))&_DeeNone_reti0_2)
#define default__map_removekeys__empty (*(int (DCALL *)(DeeObject *, DeeObject *))&default__map_removekeys__unsupported)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_removekeys__with__map_operator_inplace_sub(DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_removekeys__with__map_remove(DeeObject *self, DeeObject *keys);

/* map_pop */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_pop__with_callattr_pop(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_pop__with_callattr___map_pop__(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_pop__with_callobjectcache___map_pop__(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__map_pop__with_callobjectcache___map_pop__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_pop__unsupported(DeeObject *self, DeeObject *key);
#define default__map_pop__none (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&_DeeNone_NewRef2)
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_pop__empty(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_pop__with__map_operator_getitem__and__map_operator_delitem(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_pop__with__seq_enumerate__and__seq_operator_delitem(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_pop__with__seq_enumerate_index__and__seq_operator_delitem_index(DeeObject *self, DeeObject *key);

/* map_pop_with_default */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__map_pop_with_default__with_callattr_pop(DeeObject *self, DeeObject *key, DeeObject *default_);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__map_pop_with_default__with_callattr___map_pop__(DeeObject *self, DeeObject *key, DeeObject *default_);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__map_pop_with_default__with_callobjectcache___map_pop__(DeeObject *self, DeeObject *key, DeeObject *default_);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *DCALL tdefault__map_pop_with_default__with_callobjectcache___map_pop__(DeeTypeObject *tp_self, DeeObject *self, DeeObject *key, DeeObject *default_);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__map_pop_with_default__unsupported(DeeObject *self, DeeObject *key, DeeObject *default_);
#define default__map_pop_with_default__none (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *))&_DeeNone_NewRef3)
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__map_pop_with_default__empty(DeeObject *self, DeeObject *key, DeeObject *default_);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__map_pop_with_default__with__map_operator_trygetitem__and__map_operator_delitem(DeeObject *self, DeeObject *key, DeeObject *default_);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__map_pop_with_default__with__seq_enumerate__and__seq_operator_delitem(DeeObject *self, DeeObject *key, DeeObject *default_);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__map_pop_with_default__with__seq_enumerate_index__and__seq_operator_delitem_index(DeeObject *self, DeeObject *key, DeeObject *default_);

/* map_popitem */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_popitem__with_callattr_popitem(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_popitem__with_callattr___map_popitem__(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_popitem__with_callobjectcache___map_popitem__(DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__map_popitem__with_callobjectcache___map_popitem__(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_popitem__unsupported(DeeObject *self);
#define default__map_popitem__empty (*(DREF DeeObject *(DCALL *)(DeeObject *))&_DeeNone_NewRef1)
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_popitem__with__seq_trygetlast__and__map_operator_delitem(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_popitem__with__seq_trygetfirst__and__map_operator_delitem(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_popitem__with__seq_pop(DeeObject *self);

/* iter_advance */
INTDEF WUNUSED NONNULL((1)) size_t DCALL default__iter_advance(DeeObject *__restrict self, size_t step);
INTDEF WUNUSED NONNULL((1)) size_t DCALL default__iter_advance__with_callattr_advance(DeeObject *__restrict self, size_t step);
INTDEF WUNUSED NONNULL((1)) size_t DCALL default__iter_advance__with_callattr___iter_advance__(DeeObject *__restrict self, size_t step);
INTDEF WUNUSED NONNULL((1)) size_t DCALL default__iter_advance__with_callobjectcache___iter_advance__(DeeObject *__restrict self, size_t step);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL tdefault__iter_advance__with_callobjectcache___iter_advance__(DeeTypeObject *tp_self, DeeObject *self, size_t step);
#define default__iter_advance__unsupported (*(size_t (DCALL *)(DeeObject *__restrict, size_t))&default__iter_advance__with__operator_next)
#define default__iter_advance__empty (*(size_t (DCALL *)(DeeObject *__restrict, size_t))&_DeeNone_rets0_2)
INTDEF WUNUSED NONNULL((1)) size_t DCALL default__iter_advance__with__iter_nextkey(DeeObject *__restrict self, size_t step);
INTDEF WUNUSED NONNULL((1)) size_t DCALL default__iter_advance__with__iter_nextvalue(DeeObject *__restrict self, size_t step);
INTDEF WUNUSED NONNULL((1)) size_t DCALL default__iter_advance__with__iter_nextpair(DeeObject *__restrict self, size_t step);
INTDEF WUNUSED NONNULL((1)) size_t DCALL default__iter_advance__with__operator_next(DeeObject *__restrict self, size_t step);

/* iter_prev */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__iter_prev__with_callattr_prev(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__iter_prev__with_callattr___iter_prev__(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__iter_prev__with_callobjectcache___iter_prev__(DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__iter_prev__with_callobjectcache___iter_prev__(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__iter_prev__unsupported(DeeObject *self);
#define default__iter_prev__none (*(DREF DeeObject *(DCALL *)(DeeObject *))&_DeeNone_NewRef1)
#define default__iter_prev__empty (*(DREF DeeObject *(DCALL *)(DeeObject *))&_DeeNone_retsm1_1)
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__iter_prev__with__iter_revert__and__iter_peek(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__iter_prev__with__iter_getindex__and_operator_next__and__iter_setindex(DeeObject *self);

/* iter_revert */
INTDEF WUNUSED NONNULL((1)) size_t DCALL default__iter_revert__with_callattr_revert(DeeObject *__restrict self, size_t step);
INTDEF WUNUSED NONNULL((1)) size_t DCALL default__iter_revert__with_callattr___iter_revert__(DeeObject *__restrict self, size_t step);
INTDEF WUNUSED NONNULL((1)) size_t DCALL default__iter_revert__with_callobjectcache___iter_revert__(DeeObject *__restrict self, size_t step);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL tdefault__iter_revert__with_callobjectcache___iter_revert__(DeeTypeObject *tp_self, DeeObject *self, size_t step);
INTDEF WUNUSED NONNULL((1)) size_t DCALL default__iter_revert__unsupported(DeeObject *__restrict self, size_t step);
#define default__iter_revert__empty (*(size_t (DCALL *)(DeeObject *__restrict, size_t))&_DeeNone_rets0_2)
INTDEF WUNUSED NONNULL((1)) size_t DCALL default__iter_revert__with__iter_prev(DeeObject *__restrict self, size_t step);
INTDEF WUNUSED NONNULL((1)) size_t DCALL default__iter_revert__with__iter_getindex__and__iter_setindex(DeeObject *__restrict self, size_t step);

/* iter_operator_bool */
INTDEF WUNUSED NONNULL((1)) int DCALL default__iter_operator_bool(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__iter_operator_bool__with_callattr___iter_bool__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__iter_operator_bool__with_callobjectcache___iter_bool__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__iter_operator_bool__with_callobjectcache___iter_bool__(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__iter_operator_bool__unsupported(DeeObject *__restrict self);
#define default__iter_operator_bool__empty (*(int (DCALL *)(DeeObject *__restrict))&_DeeNone_reti0_1)
INTDEF WUNUSED NONNULL((1)) int DCALL default__iter_operator_bool__with__iter_peek(DeeObject *__restrict self);

/* iter_getindex */
INTDEF WUNUSED NONNULL((1)) size_t DCALL default__iter_getindex__with_callattr_index(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) size_t DCALL default__iter_getindex__with_callattr___iter_index__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) size_t DCALL default__iter_getindex__with_callobjectcache___iter_index__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL tdefault__iter_getindex__with_callobjectcache___iter_index__(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) size_t DCALL default__iter_getindex__unsupported(DeeObject *__restrict self);
#define default__iter_getindex__empty (*(size_t (DCALL *)(DeeObject *__restrict))&_DeeNone_rets0_1)
INTDEF WUNUSED NONNULL((1)) size_t DCALL default__iter_getindex__with__iter_getseq__and__iter_operator_compare_eq(DeeObject *__restrict self);

/* iter_setindex */
INTDEF WUNUSED NONNULL((1)) int DCALL default__iter_setindex__with_callattr_index(DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1)) int DCALL default__iter_setindex__with_callattr___iter_index__(DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1)) int DCALL default__iter_setindex__with_callobjectcache___iter_index__(DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__iter_setindex__with_callobjectcache___iter_index__(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1)) int DCALL default__iter_setindex__unsupported(DeeObject *self, size_t index);
#define default__iter_setindex__empty (*(int (DCALL *)(DeeObject *, size_t))&_DeeNone_reti0_2)
INTDEF WUNUSED NONNULL((1)) int DCALL default__iter_setindex__with__iter_getseq__and__iter_operator_assign(DeeObject *self, size_t index);

/* iter_rewind */
INTDEF WUNUSED NONNULL((1)) int DCALL default__iter_rewind(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__iter_rewind__with_callattr_index(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__iter_rewind__with_callattr___iter_index__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__iter_rewind__with_callobjectcache___iter_index__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__iter_rewind__with_callobjectcache___iter_index__(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__iter_rewind__unsupported(DeeObject *__restrict self);
#define default__iter_rewind__empty (*(int (DCALL *)(DeeObject *__restrict))&_DeeNone_reti0_1)
INTDEF WUNUSED NONNULL((1)) int DCALL default__iter_rewind__with_callattr_rewind(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__iter_rewind__with_callattr___iter_rewind__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__iter_rewind__with__iter_getseq__and__iter_operator_assign(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__iter_rewind__with__iter_setindex(DeeObject *__restrict self);

/* iter_peek */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__iter_peek__with_callattr_peek(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__iter_peek__with_callattr___iter_peek__(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__iter_peek__with_callobjectcache___iter_peek__(DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__iter_peek__with_callobjectcache___iter_peek__(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__iter_peek__unsupported(DeeObject *self);
#define default__iter_peek__none (*(DREF DeeObject *(DCALL *)(DeeObject *))&_DeeNone_NewRef1)
#define default__iter_peek__empty (*(DREF DeeObject *(DCALL *)(DeeObject *))&_DeeNone_retsm1_1)
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__iter_peek__with__operator_copy__and__operator_next(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__iter_peek__with__operator_next__and__iter_revert(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__iter_peek__with__iter_getindex__and_operator_next__and__iter_setindex(DeeObject *self);

/* iter_getseq */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__iter_getseq(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__iter_getseq__with_callattr_seq(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__iter_getseq__with_callattr___iter_seq__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__iter_getseq__with_callobjectcache___iter_seq__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__iter_getseq__with_callobjectcache___iter_seq__(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__iter_getseq__unsupported(DeeObject *__restrict self);
#define default__iter_getseq__none (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&_DeeNone_NewRef1)
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__iter_getseq__empty(DeeObject *__restrict self);
/*[[[end]]]*/
/* clang-format on */

DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_METHOD_HINT_DEFAULTS_H */
