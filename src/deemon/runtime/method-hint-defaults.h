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
#ifndef GUARD_DEEMON_RUNTIME_METHOD_HINT_DEFAULTS_H
#define GUARD_DEEMON_RUNTIME_METHOD_HINT_DEFAULTS_H 1

#include <deemon/api.h>

#if defined(CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS) || defined(__DEEMON__)
#include <deemon/method-hints.h>
#include <deemon/none-operator.h>
#include <deemon/object.h>

#include "method-hints.h"

DECL_BEGIN

/* clang-format off */
/*[[[deemon (printDefaultImplDecls from "..method-hints.method-hints")();]]]*/
/* seq_operator_bool */
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_bool__with_callattr___seq_bool__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_bool__with_callobjectcache___seq_bool__(DeeObject *__restrict self);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_bool__with_callmethodcache___seq_bool__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_bool__with_callkwmethodcache___seq_bool__(DeeObject *__restrict self);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_bool__unsupported(DeeObject *__restrict self);
#define default__seq_operator_bool__empty (*(int (DCALL *)(DeeObject *__restrict))&_DeeNone_reti0_1)
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_bool__with__seq_operator_foreach(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_bool__with__seq_operator_size(DeeObject *__restrict self);

/* seq_operator_sizeob */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_sizeob__with_callattr___seq_size__(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_sizeob__with_callobjectcache___seq_size__(DeeObject *self);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_sizeob__with_callmethodcache___seq_size__(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_sizeob__with_callkwmethodcache___seq_size__(DeeObject *self);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_sizeob__unsupported(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_sizeob__empty(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_sizeob__with__seq_operator_size(DeeObject *self);

/* seq_operator_size */
#define default__seq_operator_size__with_callattr___seq_size__ (*(size_t (DCALL *)(DeeObject *))&default__seq_operator_size__with__seq_operator_sizeob)
#define default__seq_operator_size__with_callobjectcache___seq_size__ (*(size_t (DCALL *)(DeeObject *))&default__seq_operator_size__with__seq_operator_sizeob)
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
#define default__seq_operator_size__with_callmethodcache___seq_size__ (*(size_t (DCALL *)(DeeObject *))&default__seq_operator_size__with__seq_operator_sizeob)
#define default__seq_operator_size__with_callkwmethodcache___seq_size__ (*(size_t (DCALL *)(DeeObject *))&default__seq_operator_size__with__seq_operator_sizeob)
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1)) size_t DCALL default__seq_operator_size__unsupported(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) size_t DCALL default__seq_operator_size__empty(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) size_t DCALL default__seq_operator_size__with__seq_operator_foreach(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) size_t DCALL default__seq_operator_size__with__seq_operator_sizeob(DeeObject *self);

/* seq_operator_iter */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_iter__with_callattr___seq_iter__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_iter__with_callobjectcache___seq_iter__(DeeObject *__restrict self);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_iter__with_callmethodcache___seq_iter__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_iter__with_callkwmethodcache___seq_iter__(DeeObject *__restrict self);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_iter__unsupported(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_iter__empty(DeeObject *__restrict self);

/* seq_operator_foreach */
#define default__seq_operator_foreach__with_callattr___seq_iter__ (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&default__seq_operator_foreach__with__seq_operator_iter)
#define default__seq_operator_foreach__with_callobjectcache___seq_iter__ (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&default__seq_operator_foreach__with__seq_operator_iter)
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
#define default__seq_operator_foreach__with_callmethodcache___seq_iter__ (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&default__seq_operator_foreach__with__seq_operator_iter)
#define default__seq_operator_foreach__with_callkwmethodcache___seq_iter__ (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&default__seq_operator_foreach__with__seq_operator_iter)
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
#define default__seq_operator_foreach__unsupported (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&default__seq_operator_foreach__with__seq_operator_iter)
#define default__seq_operator_foreach__empty (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&_DeeNone_rets0_3)
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL default__seq_operator_foreach__with__seq_operator_iter(DeeObject *__restrict self, Dee_foreach_t cb, void *arg);

/* seq_operator_foreach_pair */
#define default__seq_operator_foreach_pair__with_callattr___seq_iter__ (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&default__seq_operator_foreach_pair__with__seq_operator_iter)
#define default__seq_operator_foreach_pair__with_callobjectcache___seq_iter__ (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&default__seq_operator_foreach_pair__with__seq_operator_iter)
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
#define default__seq_operator_foreach_pair__with_callmethodcache___seq_iter__ (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&default__seq_operator_foreach_pair__with__seq_operator_iter)
#define default__seq_operator_foreach_pair__with_callkwmethodcache___seq_iter__ (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&default__seq_operator_foreach_pair__with__seq_operator_iter)
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
#define default__seq_operator_foreach_pair__unsupported (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&default__seq_operator_foreach_pair__with__seq_operator_iter)
#define default__seq_operator_foreach_pair__empty (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&_DeeNone_rets0_3)
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL default__seq_operator_foreach_pair__with__seq_operator_iter(DeeObject *__restrict self, Dee_foreach_pair_t cb, void *arg);

/* seq_operator_iterkeys */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_iterkeys__with_callattr___seq_iterkeys__(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_iterkeys__with_callobjectcache___seq_iterkeys__(DeeObject *self);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_iterkeys__with_callmethodcache___seq_iterkeys__(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_iterkeys__with_callkwmethodcache___seq_iterkeys__(DeeObject *self);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_iterkeys__unsupported(DeeObject *self);
#define default__seq_operator_iterkeys__empty (*(DREF DeeObject *(DCALL *)(DeeObject *))&default__seq_operator_iter__empty)
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_iterkeys__with__seq_size(DeeObject *self);

/* seq_operator_enumerate */
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL default__seq_operator_enumerate__with_callattr___seq_enumerate__(DeeObject *__restrict self, Dee_enumerate_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL default__seq_operator_enumerate__with_callobjectcache___seq_enumerate__(DeeObject *__restrict self, Dee_enumerate_t proc, void *arg);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL default__seq_operator_enumerate__with_callmethodcache___seq_enumerate__(DeeObject *__restrict self, Dee_enumerate_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL default__seq_operator_enumerate__with_callkwmethodcache___seq_enumerate__(DeeObject *__restrict self, Dee_enumerate_t proc, void *arg);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL default__seq_operator_enumerate__unsupported(DeeObject *__restrict self, Dee_enumerate_t proc, void *arg);
#define default__seq_operator_enumerate__empty (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_enumerate_t, void *))&_DeeNone_rets0_3)
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL default__seq_operator_enumerate__with__seq_operator_size_and_seq_operator_getitem_index(DeeObject *__restrict self, Dee_enumerate_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL default__seq_operator_enumerate__with__seq_operator_sizeob_and_seq_operator_getitem(DeeObject *__restrict self, Dee_enumerate_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL default__seq_operator_enumerate__with__counter_and_seq_operator_foreach(DeeObject *__restrict self, Dee_enumerate_t proc, void *arg);

/* seq_operator_enumerate_index */
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5)) Dee_ssize_t DCALL default__seq_operator_enumerate_index__with_callattr___seq_enumerate__(DeeObject *__restrict self, Dee_enumerate_index_t proc, void *arg, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5)) Dee_ssize_t DCALL default__seq_operator_enumerate_index__with_callobjectcache___seq_enumerate__(DeeObject *__restrict self, Dee_enumerate_index_t proc, void *arg, size_t start, size_t end);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5)) Dee_ssize_t DCALL default__seq_operator_enumerate_index__with_callmethodcache___seq_enumerate__(DeeObject *__restrict self, Dee_enumerate_index_t proc, void *arg, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5)) Dee_ssize_t DCALL default__seq_operator_enumerate_index__with_callkwmethodcache___seq_enumerate__(DeeObject *__restrict self, Dee_enumerate_index_t proc, void *arg, size_t start, size_t end);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5)) Dee_ssize_t DCALL default__seq_operator_enumerate_index__unsupported(DeeObject *__restrict self, Dee_enumerate_index_t proc, void *arg, size_t start, size_t end);
#define default__seq_operator_enumerate_index__empty (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_enumerate_index_t, void *, size_t, size_t))&_DeeNone_rets0_5)
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5)) Dee_ssize_t DCALL default__seq_operator_enumerate_index__with__seq_operator_size_and_seq_operator_getitem_index(DeeObject *__restrict self, Dee_enumerate_index_t proc, void *arg, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5)) Dee_ssize_t DCALL default__seq_operator_enumerate_index__with__counter_and_seq_operator_foreach(DeeObject *__restrict self, Dee_enumerate_index_t proc, void *arg, size_t start, size_t end);

/* seq_operator_getitem */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_getitem__with_callattr___seq_getitem__(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_getitem__with_callobjectcache___seq_getitem__(DeeObject *self, DeeObject *index);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_getitem__with_callmethodcache___seq_getitem__(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_getitem__with_callkwmethodcache___seq_getitem__(DeeObject *self, DeeObject *index);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_getitem__unsupported(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_getitem__empty(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_getitem__with__seq_operator_getitem_index(DeeObject *self, DeeObject *index);

/* seq_operator_getitem_index */
#define default__seq_operator_getitem_index__with_callattr___seq_getitem__ (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict, size_t))&default__seq_operator_getitem_index__with__seq_operator_getitem)
#define default__seq_operator_getitem_index__with_callobjectcache___seq_getitem__ (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict, size_t))&default__seq_operator_getitem_index__with__seq_operator_getitem)
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
#define default__seq_operator_getitem_index__with_callmethodcache___seq_getitem__ (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict, size_t))&default__seq_operator_getitem_index__with__seq_operator_getitem)
#define default__seq_operator_getitem_index__with_callkwmethodcache___seq_getitem__ (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict, size_t))&default__seq_operator_getitem_index__with__seq_operator_getitem)
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_getitem_index__unsupported(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_getitem_index__empty(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_getitem_index__with__seq_operator_foreach(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_getitem_index__with__seq_operator_getitem(DeeObject *__restrict self, size_t index);

/* seq_operator_trygetitem_index */
#define default__seq_operator_trygetitem_index__with_callattr___seq_getitem__ (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict, size_t))&default__seq_operator_trygetitem_index__with__seq_operator_getitem_index)
#define default__seq_operator_trygetitem_index__with_callobjectcache___seq_getitem__ (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict, size_t))&default__seq_operator_trygetitem_index__with__seq_operator_getitem_index)
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
#define default__seq_operator_trygetitem_index__with_callmethodcache___seq_getitem__ (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict, size_t))&default__seq_operator_trygetitem_index__with__seq_operator_getitem_index)
#define default__seq_operator_trygetitem_index__with_callkwmethodcache___seq_getitem__ (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict, size_t))&default__seq_operator_trygetitem_index__with__seq_operator_getitem_index)
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
#define default__seq_operator_trygetitem_index__unsupported (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict, size_t))&default__seq_operator_getitem_index__unsupported)
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_trygetitem_index__empty(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_trygetitem_index__with__seq_operator_getitem_index(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_trygetitem_index__with__seq_operator_foreach(DeeObject *__restrict self, size_t index);

/* seq_operator_trygetitem */
#define default__seq_operator_trygetitem__with_callattr___seq_getitem__ (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&default__seq_operator_trygetitem__with__seq_operator_getitem)
#define default__seq_operator_trygetitem__with_callobjectcache___seq_getitem__ (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&default__seq_operator_trygetitem__with__seq_operator_getitem)
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
#define default__seq_operator_trygetitem__with_callmethodcache___seq_getitem__ (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&default__seq_operator_trygetitem__with__seq_operator_getitem)
#define default__seq_operator_trygetitem__with_callkwmethodcache___seq_getitem__ (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&default__seq_operator_trygetitem__with__seq_operator_getitem)
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
#define default__seq_operator_trygetitem__unsupported (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&default__seq_operator_getitem__unsupported)
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_trygetitem__empty(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_trygetitem__with__seq_operator_getitem(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_trygetitem__with__seq_operator_trygetitem_index(DeeObject *self, DeeObject *index);

/* seq_operator_hasitem */
#define default__seq_operator_hasitem__with_callattr___seq_getitem__ (*(int (DCALL *)(DeeObject *, DeeObject *))&default__seq_operator_hasitem__with__seq_operator_getitem)
#define default__seq_operator_hasitem__with_callobjectcache___seq_getitem__ (*(int (DCALL *)(DeeObject *, DeeObject *))&default__seq_operator_hasitem__with__seq_operator_getitem)
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
#define default__seq_operator_hasitem__with_callmethodcache___seq_getitem__ (*(int (DCALL *)(DeeObject *, DeeObject *))&default__seq_operator_hasitem__with__seq_operator_getitem)
#define default__seq_operator_hasitem__with_callkwmethodcache___seq_getitem__ (*(int (DCALL *)(DeeObject *, DeeObject *))&default__seq_operator_hasitem__with__seq_operator_getitem)
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_hasitem__unsupported(DeeObject *self, DeeObject *index);
#define default__seq_operator_hasitem__empty (*(int (DCALL *)(DeeObject *, DeeObject *))&_DeeNone_reti0_2)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_hasitem__with__seq_operator_hasitem_index(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_hasitem__with__seq_operator_getitem(DeeObject *self, DeeObject *index);

/* seq_operator_hasitem_index */
#define default__seq_operator_hasitem_index__with_callattr___seq_getitem__ (*(int (DCALL *)(DeeObject *__restrict, size_t))&default__seq_operator_hasitem_index__with__seq_operator_getitem_index)
#define default__seq_operator_hasitem_index__with_callobjectcache___seq_getitem__ (*(int (DCALL *)(DeeObject *__restrict, size_t))&default__seq_operator_hasitem_index__with__seq_operator_getitem_index)
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
#define default__seq_operator_hasitem_index__with_callmethodcache___seq_getitem__ (*(int (DCALL *)(DeeObject *__restrict, size_t))&default__seq_operator_hasitem_index__with__seq_operator_getitem_index)
#define default__seq_operator_hasitem_index__with_callkwmethodcache___seq_getitem__ (*(int (DCALL *)(DeeObject *__restrict, size_t))&default__seq_operator_hasitem_index__with__seq_operator_getitem_index)
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_hasitem_index__unsupported(DeeObject *__restrict self, size_t index);
#define default__seq_operator_hasitem_index__empty (*(int (DCALL *)(DeeObject *__restrict, size_t))&_DeeNone_reti0_2)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_hasitem_index__with__seq_operator_size(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_hasitem_index__with__seq_operator_getitem_index(DeeObject *__restrict self, size_t index);

/* seq_operator_bounditem */
#define default__seq_operator_bounditem__with_callattr___seq_getitem__ (*(int (DCALL *)(DeeObject *, DeeObject *))&default__seq_operator_bounditem__with__seq_operator_getitem)
#define default__seq_operator_bounditem__with_callobjectcache___seq_getitem__ (*(int (DCALL *)(DeeObject *, DeeObject *))&default__seq_operator_bounditem__with__seq_operator_getitem)
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
#define default__seq_operator_bounditem__with_callmethodcache___seq_getitem__ (*(int (DCALL *)(DeeObject *, DeeObject *))&default__seq_operator_bounditem__with__seq_operator_getitem)
#define default__seq_operator_bounditem__with_callkwmethodcache___seq_getitem__ (*(int (DCALL *)(DeeObject *, DeeObject *))&default__seq_operator_bounditem__with__seq_operator_getitem)
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_bounditem__unsupported(DeeObject *self, DeeObject *index);
#define default__seq_operator_bounditem__empty (*(int (DCALL *)(DeeObject *, DeeObject *))&_DeeNone_reti0_2)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_bounditem__with__seq_operator_bounditem_index(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_bounditem__with__seq_operator_getitem(DeeObject *self, DeeObject *index);

/* seq_operator_bounditem_index */
#define default__seq_operator_bounditem_index__with_callattr___seq_getitem__ (*(int (DCALL *)(DeeObject *__restrict, size_t))&default__seq_operator_bounditem_index__with__seq_operator_getitem_index)
#define default__seq_operator_bounditem_index__with_callobjectcache___seq_getitem__ (*(int (DCALL *)(DeeObject *__restrict, size_t))&default__seq_operator_bounditem_index__with__seq_operator_getitem_index)
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
#define default__seq_operator_bounditem_index__with_callmethodcache___seq_getitem__ (*(int (DCALL *)(DeeObject *__restrict, size_t))&default__seq_operator_bounditem_index__with__seq_operator_getitem_index)
#define default__seq_operator_bounditem_index__with_callkwmethodcache___seq_getitem__ (*(int (DCALL *)(DeeObject *__restrict, size_t))&default__seq_operator_bounditem_index__with__seq_operator_getitem_index)
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_bounditem_index__unsupported(DeeObject *__restrict self, size_t index);
#define default__seq_operator_bounditem_index__empty (*(int (DCALL *)(DeeObject *__restrict, size_t))&_DeeNone_reti0_2)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_bounditem_index__with__seq_operator_getitem_index(DeeObject *__restrict self, size_t index);

/* seq_operator_delitem */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_delitem__with_callattr___seq_delitem__(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_delitem__with_callobjectcache___seq_delitem__(DeeObject *self, DeeObject *index);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_delitem__with_callmethodcache___seq_delitem__(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_delitem__with_callkwmethodcache___seq_delitem__(DeeObject *self, DeeObject *index);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_delitem__unsupported(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_delitem__empty(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_delitem__with__seq_operator_delitem_index(DeeObject *self, DeeObject *index);

/* seq_operator_delitem_index */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_delitem_index__with_callattr___seq_delitem__(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_delitem_index__with_callobjectcache___seq_delitem__(DeeObject *__restrict self, size_t index);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_delitem_index__with_callmethodcache___seq_delitem__(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_delitem_index__with_callkwmethodcache___seq_delitem__(DeeObject *__restrict self, size_t index);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_delitem_index__unsupported(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_delitem_index__empty(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_delitem_index__with__seq_operator_size__and__seq_erase(DeeObject *__restrict self, size_t index);

/* seq_operator_setitem */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_operator_setitem__with_callattr___seq_setitem__(DeeObject *self, DeeObject *index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_operator_setitem__with_callobjectcache___seq_setitem__(DeeObject *self, DeeObject *index, DeeObject *value);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_operator_setitem__with_callmethodcache___seq_setitem__(DeeObject *self, DeeObject *index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_operator_setitem__with_callkwmethodcache___seq_setitem__(DeeObject *self, DeeObject *index, DeeObject *value);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_operator_setitem__unsupported(DeeObject *self, DeeObject *index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_operator_setitem__empty(DeeObject *self, DeeObject *index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_operator_setitem__with__seq_operator_setitem_index(DeeObject *self, DeeObject *index, DeeObject *value);

/* seq_operator_setitem_index */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_operator_setitem_index__with_callattr___seq_setitem__(DeeObject *self, size_t index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_operator_setitem_index__with_callobjectcache___seq_setitem__(DeeObject *self, size_t index, DeeObject *value);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_operator_setitem_index__with_callmethodcache___seq_setitem__(DeeObject *self, size_t index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_operator_setitem_index__with_callkwmethodcache___seq_setitem__(DeeObject *self, size_t index, DeeObject *value);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_operator_setitem_index__unsupported(DeeObject *self, size_t index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_operator_setitem_index__empty(DeeObject *self, size_t index, DeeObject *value);

/* seq_operator_getrange */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_operator_getrange__with_callattr___seq_getrange__(DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_operator_getrange__with_callobjectcache___seq_getrange__(DeeObject *self, DeeObject *start, DeeObject *end);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_operator_getrange__with_callmethodcache___seq_getrange__(DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_operator_getrange__with_callkwmethodcache___seq_getrange__(DeeObject *self, DeeObject *start, DeeObject *end);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_operator_getrange__unsupported(DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_operator_getrange__empty(DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_operator_getrange__with__seq_operator_getrange_index__and__seq_operator_getrange_index_n(DeeObject *self, DeeObject *start, DeeObject *end);

/* seq_operator_getrange_index */
#define default__seq_operator_getrange_index__with_callattr___seq_getrange__ (*(DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&default__seq_operator_getrange_index__with__seq_operator_getrange)
#define default__seq_operator_getrange_index__with_callobjectcache___seq_getrange__ (*(DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&default__seq_operator_getrange_index__with__seq_operator_getrange)
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
#define default__seq_operator_getrange_index__with_callmethodcache___seq_getrange__ (*(DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&default__seq_operator_getrange_index__with__seq_operator_getrange)
#define default__seq_operator_getrange_index__with_callkwmethodcache___seq_getrange__ (*(DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&default__seq_operator_getrange_index__with__seq_operator_getrange)
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_operator_getrange_index__unsupported(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_operator_getrange_index__empty(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_operator_getrange_index__with__seq_operator_getrange(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_getitem_index(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_getitem(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_iter(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);

/* seq_operator_getrange_index_n */
#define default__seq_operator_getrange_index_n__with_callattr___seq_getrange__ (*(DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t))&default__seq_operator_getrange_index_n__with__seq_operator_getrange)
#define default__seq_operator_getrange_index_n__with_callobjectcache___seq_getrange__ (*(DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t))&default__seq_operator_getrange_index_n__with__seq_operator_getrange)
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
#define default__seq_operator_getrange_index_n__with_callmethodcache___seq_getrange__ (*(DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t))&default__seq_operator_getrange_index_n__with__seq_operator_getrange)
#define default__seq_operator_getrange_index_n__with_callkwmethodcache___seq_getrange__ (*(DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t))&default__seq_operator_getrange_index_n__with__seq_operator_getrange)
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_getrange_index_n__unsupported(DeeObject *self, Dee_ssize_t start);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_getrange_index_n__empty(DeeObject *self, Dee_ssize_t start);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_getrange_index_n__with__seq_operator_getrange(DeeObject *self, Dee_ssize_t start);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_getrange_index(DeeObject *self, Dee_ssize_t start);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_getitem_index(DeeObject *self, Dee_ssize_t start);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *self, Dee_ssize_t start);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_getitem(DeeObject *self, Dee_ssize_t start);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_iter(DeeObject *self, Dee_ssize_t start);

/* seq_operator_delrange */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_operator_delrange__with_callattr___seq_delrange__(DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_operator_delrange__with_callobjectcache___seq_delrange__(DeeObject *self, DeeObject *start, DeeObject *end);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_operator_delrange__with_callmethodcache___seq_delrange__(DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_operator_delrange__with_callkwmethodcache___seq_delrange__(DeeObject *self, DeeObject *start, DeeObject *end);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_operator_delrange__unsupported(DeeObject *self, DeeObject *start, DeeObject *end);
#define default__seq_operator_delrange__empty (*(int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&_DeeNone_reti0_3)
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_operator_delrange__with__seq_operator_delrange_index__and__seq_operator_delrange_index_n(DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_operator_delrange__with__seq_operator_setrange(DeeObject *self, DeeObject *start, DeeObject *end);

/* seq_operator_delrange_index */
#define default__seq_operator_delrange_index__with_callattr___seq_delrange__ (*(int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&default__seq_operator_delrange_index__with__seq_operator_delrange)
#define default__seq_operator_delrange_index__with_callobjectcache___seq_delrange__ (*(int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&default__seq_operator_delrange_index__with__seq_operator_delrange)
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
#define default__seq_operator_delrange_index__with_callmethodcache___seq_delrange__ (*(int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&default__seq_operator_delrange_index__with__seq_operator_delrange)
#define default__seq_operator_delrange_index__with_callkwmethodcache___seq_delrange__ (*(int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&default__seq_operator_delrange_index__with__seq_operator_delrange)
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_operator_delrange_index__unsupported(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);
#define default__seq_operator_delrange_index__empty (*(int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&_DeeNone_reti0_3)
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_operator_delrange_index__with__seq_operator_delrange(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_operator_delrange_index__with__seq_operator_size__and__seq_erase(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_operator_delrange_index__with__seq_operator_setrange_index(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);

/* seq_operator_delrange_index_n */
#define default__seq_operator_delrange_index_n__with_callattr___seq_delrange__ (*(int (DCALL *)(DeeObject *, Dee_ssize_t))&default__seq_operator_delrange_index_n__with__seq_operator_delrange)
#define default__seq_operator_delrange_index_n__with_callobjectcache___seq_delrange__ (*(int (DCALL *)(DeeObject *, Dee_ssize_t))&default__seq_operator_delrange_index_n__with__seq_operator_delrange)
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
#define default__seq_operator_delrange_index_n__with_callmethodcache___seq_delrange__ (*(int (DCALL *)(DeeObject *, Dee_ssize_t))&default__seq_operator_delrange_index_n__with__seq_operator_delrange)
#define default__seq_operator_delrange_index_n__with_callkwmethodcache___seq_delrange__ (*(int (DCALL *)(DeeObject *, Dee_ssize_t))&default__seq_operator_delrange_index_n__with__seq_operator_delrange)
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_delrange_index_n__unsupported(DeeObject *self, Dee_ssize_t start);
#define default__seq_operator_delrange_index_n__empty (*(int (DCALL *)(DeeObject *, Dee_ssize_t))&_DeeNone_reti0_2)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_delrange_index_n__with__seq_operator_delrange_index(DeeObject *self, Dee_ssize_t start);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_delrange_index_n__with__seq_operator_delrange(DeeObject *self, Dee_ssize_t start);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_delrange_index_n__with__seq_operator_setrange_index_n(DeeObject *self, Dee_ssize_t start);

/* seq_operator_setrange */
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL default__seq_operator_setrange__with_callattr___seq_setrange__(DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *values);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL default__seq_operator_setrange__with_callobjectcache___seq_setrange__(DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *values);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL default__seq_operator_setrange__with_callmethodcache___seq_setrange__(DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *values);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL default__seq_operator_setrange__with_callkwmethodcache___seq_setrange__(DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *values);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL default__seq_operator_setrange__unsupported(DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *values);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL default__seq_operator_setrange__empty(DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *values);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL default__seq_operator_setrange__with__seq_operator_setrange_index__and__seq_operator_setrange_index_n(DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *values);

/* seq_operator_setrange_index */
#define default__seq_operator_setrange_index__with_callattr___seq_setrange__ (*(int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t, DeeObject *))&default__seq_operator_setrange_index__with__seq_operator_setrange)
#define default__seq_operator_setrange_index__with_callobjectcache___seq_setrange__ (*(int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t, DeeObject *))&default__seq_operator_setrange_index__with__seq_operator_setrange)
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
#define default__seq_operator_setrange_index__with_callmethodcache___seq_setrange__ (*(int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t, DeeObject *))&default__seq_operator_setrange_index__with__seq_operator_setrange)
#define default__seq_operator_setrange_index__with_callkwmethodcache___seq_setrange__ (*(int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t, DeeObject *))&default__seq_operator_setrange_index__with__seq_operator_setrange)
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL default__seq_operator_setrange_index__unsupported(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *values);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL default__seq_operator_setrange_index__empty(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *values);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL default__seq_operator_setrange_index__with__seq_operator_setrange(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *values);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL default__seq_operator_setrange_index__with__seq_operator_size__and__seq_erase__and__seq_insertall(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *values);

/* seq_operator_setrange_index_n */
#define default__seq_operator_setrange_index_n__with_callattr___seq_setrange__ (*(int (DCALL *)(DeeObject *, Dee_ssize_t, DeeObject *))&default__seq_operator_setrange_index_n__with__seq_operator_setrange)
#define default__seq_operator_setrange_index_n__with_callobjectcache___seq_setrange__ (*(int (DCALL *)(DeeObject *, Dee_ssize_t, DeeObject *))&default__seq_operator_setrange_index_n__with__seq_operator_setrange)
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
#define default__seq_operator_setrange_index_n__with_callmethodcache___seq_setrange__ (*(int (DCALL *)(DeeObject *, Dee_ssize_t, DeeObject *))&default__seq_operator_setrange_index_n__with__seq_operator_setrange)
#define default__seq_operator_setrange_index_n__with_callkwmethodcache___seq_setrange__ (*(int (DCALL *)(DeeObject *, Dee_ssize_t, DeeObject *))&default__seq_operator_setrange_index_n__with__seq_operator_setrange)
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_operator_setrange_index_n__unsupported(DeeObject *self, Dee_ssize_t start, DeeObject *values);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_operator_setrange_index_n__empty(DeeObject *self, Dee_ssize_t start, DeeObject *values);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_operator_setrange_index_n__with__seq_operator_setrange_index(DeeObject *self, Dee_ssize_t start, DeeObject *values);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_operator_setrange_index_n__with__seq_operator_setrange(DeeObject *self, Dee_ssize_t start, DeeObject *values);

/* seq_operator_hash */
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL default__seq_operator_hash__with_callattr___seq_hash__(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL default__seq_operator_hash__with_callobjectcache___seq_hash__(DeeObject *self);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL default__seq_operator_hash__with_callmethodcache___seq_hash__(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL default__seq_operator_hash__with_callkwmethodcache___seq_hash__(DeeObject *self);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL default__seq_operator_hash__unsupported(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL default__seq_operator_hash__empty(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL default__seq_operator_hash__with__seq_operator_foreach(DeeObject *self);

/* seq_operator_compare */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_compare__with_callattr___seq_compare__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_compare__with_callobjectcache___seq_compare__(DeeObject *lhs, DeeObject *rhs);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_compare__with_callmethodcache___seq_compare__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_compare__with_callkwmethodcache___seq_compare__(DeeObject *lhs, DeeObject *rhs);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_compare__unsupported(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_compare__empty(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_compare__with__seq_operator_foreach(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_compare__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_compare__with__seq_operator_size__and__seq_operator_getitem_index(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_compare__with__seq_operator_sizeob__and__seq_operator_getitem(DeeObject *lhs, DeeObject *rhs);

/* seq_operator_compare_eq */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_compare_eq__with_callattr___seq_compare_eq__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_compare_eq__with_callobjectcache___seq_compare_eq__(DeeObject *lhs, DeeObject *rhs);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_compare_eq__with_callmethodcache___seq_compare_eq__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_compare_eq__with_callkwmethodcache___seq_compare_eq__(DeeObject *lhs, DeeObject *rhs);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_compare_eq__unsupported(DeeObject *lhs, DeeObject *rhs);
#define default__seq_operator_compare_eq__empty (*(int (DCALL *)(DeeObject *, DeeObject *))&default__seq_operator_compare__empty)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_compare_eq__with__seq_operator_foreach(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_compare_eq__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_compare_eq__with__seq_operator_size__and__seq_operator_getitem_index(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_compare_eq__with__seq_operator_sizeob__and__seq_operator_getitem(DeeObject *lhs, DeeObject *rhs);

/* seq_operator_trycompare_eq */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_trycompare_eq__with_callattr___seq_trycompare_eq__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_trycompare_eq__with_callobjectcache___seq_trycompare_eq__(DeeObject *lhs, DeeObject *rhs);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_trycompare_eq__with_callmethodcache___seq_trycompare_eq__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_trycompare_eq__with_callkwmethodcache___seq_trycompare_eq__(DeeObject *lhs, DeeObject *rhs);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_trycompare_eq__unsupported(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_trycompare_eq__empty(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_trycompare_eq__with__seq_operator_foreach(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_trycompare_eq__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_trycompare_eq__with__seq_operator_size__and__seq_operator_getitem_index(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_trycompare_eq__with__seq_operator_sizeob__and__seq_operator_getitem(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_trycompare_eq__with__seq_operator_compare_eq(DeeObject *lhs, DeeObject *rhs);

/* seq_operator_eq */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_eq__with_callattr___seq_eq__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_eq__with_callobjectcache___seq_eq__(DeeObject *lhs, DeeObject *rhs);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_eq__with_callmethodcache___seq_eq__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_eq__with_callkwmethodcache___seq_eq__(DeeObject *lhs, DeeObject *rhs);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_eq__unsupported(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_eq__with__seq_operator_compare_eq(DeeObject *lhs, DeeObject *rhs);

/* seq_operator_ne */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_ne__with_callattr___seq_ne__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_ne__with_callobjectcache___seq_ne__(DeeObject *lhs, DeeObject *rhs);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_ne__with_callmethodcache___seq_ne__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_ne__with_callkwmethodcache___seq_ne__(DeeObject *lhs, DeeObject *rhs);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_ne__unsupported(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_ne__with__seq_operator_compare_eq(DeeObject *lhs, DeeObject *rhs);

/* seq_operator_lo */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_lo__with_callattr___seq_lo__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_lo__with_callobjectcache___seq_lo__(DeeObject *lhs, DeeObject *rhs);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_lo__with_callmethodcache___seq_lo__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_lo__with_callkwmethodcache___seq_lo__(DeeObject *lhs, DeeObject *rhs);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_lo__unsupported(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_lo__with__seq_operator_compare(DeeObject *lhs, DeeObject *rhs);

/* seq_operator_le */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_le__with_callattr___seq_le__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_le__with_callobjectcache___seq_le__(DeeObject *lhs, DeeObject *rhs);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_le__with_callmethodcache___seq_le__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_le__with_callkwmethodcache___seq_le__(DeeObject *lhs, DeeObject *rhs);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_le__unsupported(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_le__with__seq_operator_compare(DeeObject *lhs, DeeObject *rhs);

/* seq_operator_gr */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_gr__with_callattr___seq_gr__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_gr__with_callobjectcache___seq_gr__(DeeObject *lhs, DeeObject *rhs);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_gr__with_callmethodcache___seq_gr__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_gr__with_callkwmethodcache___seq_gr__(DeeObject *lhs, DeeObject *rhs);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_gr__unsupported(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_gr__with__seq_operator_compare(DeeObject *lhs, DeeObject *rhs);

/* seq_operator_ge */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_ge__with_callattr___seq_ge__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_ge__with_callobjectcache___seq_ge__(DeeObject *lhs, DeeObject *rhs);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_ge__with_callmethodcache___seq_ge__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_ge__with_callkwmethodcache___seq_ge__(DeeObject *lhs, DeeObject *rhs);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_ge__unsupported(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_ge__with__seq_operator_compare(DeeObject *lhs, DeeObject *rhs);

/* seq_trygetfirst */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_trygetfirst__with_callattr_first(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_trygetfirst__with_callattr___seq_first__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_trygetfirst__with_callobjectcache___seq_first__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_trygetfirst__unsupported(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_trygetfirst__empty(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_trygetfirst__with__size__and__getitem_index_fast(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_trygetfirst__with__seq_operator_trygetitem_index(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_trygetfirst__with__seq_operator_foreach(DeeObject *__restrict self);

/* seq_getfirst */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_getfirst__with_callattr_first(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_getfirst__with_callattr___seq_first__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_getfirst__with_callobjectcache___seq_first__(DeeObject *__restrict self);
#define default__seq_getfirst__unsupported (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&default__seq_trygetfirst__unsupported)
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_getfirst__empty(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_getfirst__with__seq_operator_getitem_index(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_getfirst__with__seq_trygetfirst(DeeObject *__restrict self);

/* seq_boundfirst */
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_boundfirst__with_callattr_first(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_boundfirst__with_callattr___seq_first__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_boundfirst__with_callobjectcache___seq_first__(DeeObject *__restrict self);
#define default__seq_boundfirst__unsupported (*(int (DCALL *)(DeeObject *__restrict))&default__seq_boundfirst__empty)
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_boundfirst__empty(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_boundfirst__with__seq_operator_bounditem_index(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_boundfirst__with__seq_trygetfirst(DeeObject *__restrict self);

/* seq_delfirst */
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_delfirst__with_callattr_first(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_delfirst__with_callattr___seq_first__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_delfirst__with_callobjectcache___seq_first__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_delfirst__unsupported(DeeObject *__restrict self);
#define default__seq_delfirst__empty (*(int (DCALL *)(DeeObject *__restrict))&_DeeNone_reti0_1)
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_delfirst__with__seq_operator_delitem_index(DeeObject *__restrict self);

/* seq_setfirst */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_setfirst__with_callattr_first(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_setfirst__with_callattr___seq_first__(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_setfirst__with_callobjectcache___seq_first__(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_setfirst__unsupported(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_setfirst__empty(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_setfirst__with__seq_operator_setitem_index(DeeObject *self, DeeObject *value);

/* seq_trygetlast */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_trygetlast__with_callattr_last(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_trygetlast__with_callattr___seq_last__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_trygetlast__with_callobjectcache___seq_last__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_trygetlast__unsupported(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_trygetlast__empty(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_trygetlast__with__size__and__getitem_index_fast(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_trygetlast__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_trygetlast__with__seq_operator_foreach(DeeObject *__restrict self);

/* seq_getlast */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_getlast__with_callattr_last(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_getlast__with_callattr___seq_last__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_getlast__with_callobjectcache___seq_last__(DeeObject *__restrict self);
#define default__seq_getlast__unsupported (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&default__seq_trygetlast__unsupported)
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_getlast__empty(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_getlast__with__seq_operator_size__and__seq_operator_getitem_index(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_getlast__with__seq_trygetlast(DeeObject *__restrict self);

/* seq_boundlast */
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_boundlast__with_callattr_last(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_boundlast__with_callattr___seq_last__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_boundlast__with_callobjectcache___seq_last__(DeeObject *__restrict self);
#define default__seq_boundlast__unsupported (*(int (DCALL *)(DeeObject *__restrict))&default__seq_boundlast__empty)
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_boundlast__empty(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_boundlast__with__seq_operator_size__and__seq_operator_bounditem_index(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_boundlast__with__seq_trygetlast(DeeObject *__restrict self);

/* seq_dellast */
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_dellast__with_callattr_last(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_dellast__with_callattr___seq_last__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_dellast__with_callobjectcache___seq_last__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_dellast__unsupported(DeeObject *__restrict self);
#define default__seq_dellast__empty (*(int (DCALL *)(DeeObject *__restrict))&_DeeNone_reti0_1)
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_dellast__with__seq_operator_size__and__seq_operator_delitem_index(DeeObject *__restrict self);

/* seq_setlast */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_setlast__with_callattr_last(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_setlast__with_callattr___seq_last__(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_setlast__with_callobjectcache___seq_last__(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_setlast__unsupported(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_setlast__empty(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_setlast__with__seq_operator_size__and__seq_operator_setitem_index(DeeObject *self, DeeObject *value);

/* seq_any */
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_any__with_callattr_any(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_any__with_callattr___seq_any__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_any__with_callobjectcache___seq_any__(DeeObject *__restrict self);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_any__with_callmethodcache___seq_any__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_any__with_callkwmethodcache___seq_any__(DeeObject *__restrict self);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_any__unsupported(DeeObject *__restrict self);
#define default__seq_any__empty (*(int (DCALL *)(DeeObject *__restrict))&_DeeNone_reti0_1)
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_any__with__seq_operator_foreach(DeeObject *__restrict self);

/* seq_any_with_key */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_any_with_key__with_callattr_any(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_any_with_key__with_callattr___seq_any__(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_any_with_key__with_callobjectcache___seq_any__(DeeObject *self, DeeObject *key);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_any_with_key__with_callmethodcache___seq_any__(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_any_with_key__with_callkwmethodcache___seq_any__(DeeObject *self, DeeObject *key);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_any_with_key__unsupported(DeeObject *self, DeeObject *key);
#define default__seq_any_with_key__empty (*(int (DCALL *)(DeeObject *, DeeObject *))&_DeeNone_reti0_2)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_any_with_key__with__seq_operator_foreach(DeeObject *self, DeeObject *key);

/* seq_any_with_range */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_any_with_range__with_callattr_any(DeeObject *__restrict self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_any_with_range__with_callattr___seq_any__(DeeObject *__restrict self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_any_with_range__with_callobjectcache___seq_any__(DeeObject *__restrict self, size_t start, size_t end);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_any_with_range__with_callmethodcache___seq_any__(DeeObject *__restrict self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_any_with_range__with_callkwmethodcache___seq_any__(DeeObject *__restrict self, size_t start, size_t end);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_any_with_range__unsupported(DeeObject *__restrict self, size_t start, size_t end);
#define default__seq_any_with_range__empty (*(int (DCALL *)(DeeObject *__restrict, size_t, size_t))&_DeeNone_reti0_3)
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_any_with_range__with__seq_enumerate_index(DeeObject *__restrict self, size_t start, size_t end);

/* seq_any_with_range_and_key */
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL default__seq_any_with_range_and_key__with_callattr_any(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL default__seq_any_with_range_and_key__with_callattr___seq_any__(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL default__seq_any_with_range_and_key__with_callobjectcache___seq_any__(DeeObject *self, size_t start, size_t end, DeeObject *key);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL default__seq_any_with_range_and_key__with_callmethodcache___seq_any__(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL default__seq_any_with_range_and_key__with_callkwmethodcache___seq_any__(DeeObject *self, size_t start, size_t end, DeeObject *key);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL default__seq_any_with_range_and_key__unsupported(DeeObject *self, size_t start, size_t end, DeeObject *key);
#define default__seq_any_with_range_and_key__empty (*(int (DCALL *)(DeeObject *, size_t, size_t, DeeObject *))&_DeeNone_reti0_4)
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL default__seq_any_with_range_and_key__with__seq_enumerate_index(DeeObject *self, size_t start, size_t end, DeeObject *key);

/* seq_all */
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_all__with_callattr_all(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_all__with_callattr___seq_all__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_all__with_callobjectcache___seq_all__(DeeObject *__restrict self);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_all__with_callmethodcache___seq_all__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_all__with_callkwmethodcache___seq_all__(DeeObject *__restrict self);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_all__unsupported(DeeObject *__restrict self);
#define default__seq_all__empty (*(int (DCALL *)(DeeObject *__restrict))&_DeeNone_reti0_1)
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_all__with__seq_operator_foreach(DeeObject *__restrict self);

/* seq_all_with_key */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_all_with_key__with_callattr_all(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_all_with_key__with_callattr___seq_all__(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_all_with_key__with_callobjectcache___seq_all__(DeeObject *self, DeeObject *key);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_all_with_key__with_callmethodcache___seq_all__(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_all_with_key__with_callkwmethodcache___seq_all__(DeeObject *self, DeeObject *key);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_all_with_key__unsupported(DeeObject *self, DeeObject *key);
#define default__seq_all_with_key__empty (*(int (DCALL *)(DeeObject *, DeeObject *))&_DeeNone_reti0_2)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_all_with_key__with__seq_operator_foreach(DeeObject *self, DeeObject *key);

/* seq_all_with_range */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_all_with_range__with_callattr_all(DeeObject *__restrict self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_all_with_range__with_callattr___seq_all__(DeeObject *__restrict self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_all_with_range__with_callobjectcache___seq_all__(DeeObject *__restrict self, size_t start, size_t end);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_all_with_range__with_callmethodcache___seq_all__(DeeObject *__restrict self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_all_with_range__with_callkwmethodcache___seq_all__(DeeObject *__restrict self, size_t start, size_t end);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_all_with_range__unsupported(DeeObject *__restrict self, size_t start, size_t end);
#define default__seq_all_with_range__empty (*(int (DCALL *)(DeeObject *__restrict, size_t, size_t))&_DeeNone_reti0_3)
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_all_with_range__with__seq_enumerate_index(DeeObject *__restrict self, size_t start, size_t end);

/* seq_all_with_range_and_key */
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL default__seq_all_with_range_and_key__with_callattr_all(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL default__seq_all_with_range_and_key__with_callattr___seq_all__(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL default__seq_all_with_range_and_key__with_callobjectcache___seq_all__(DeeObject *self, size_t start, size_t end, DeeObject *key);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL default__seq_all_with_range_and_key__with_callmethodcache___seq_all__(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL default__seq_all_with_range_and_key__with_callkwmethodcache___seq_all__(DeeObject *self, size_t start, size_t end, DeeObject *key);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL default__seq_all_with_range_and_key__unsupported(DeeObject *self, size_t start, size_t end, DeeObject *key);
#define default__seq_all_with_range_and_key__empty (*(int (DCALL *)(DeeObject *, size_t, size_t, DeeObject *))&_DeeNone_reti0_4)
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL default__seq_all_with_range_and_key__with__seq_enumerate_index(DeeObject *self, size_t start, size_t end, DeeObject *key);

/* seq_find */
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) size_t DCALL default__seq_find__with_callattr_find(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) size_t DCALL default__seq_find__with_callattr___seq_find__(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) size_t DCALL default__seq_find__with_callobjectcache___seq_find__(DeeObject *self, DeeObject *item, size_t start, size_t end);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) size_t DCALL default__seq_find__with_callmethodcache___seq_find__(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) size_t DCALL default__seq_find__with_callkwmethodcache___seq_find__(DeeObject *self, DeeObject *item, size_t start, size_t end);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) size_t DCALL default__seq_find__unsupported(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) size_t DCALL default__seq_find__empty(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) size_t DCALL default__seq_find__with__seq_operator_enumerate_index(DeeObject *self, DeeObject *item, size_t start, size_t end);

/* seq_find_with_key */
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5)) int DCALL default__seq_find_with_key__with_callattr_find(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5)) int DCALL default__seq_find_with_key__with_callattr___seq_find__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5)) int DCALL default__seq_find_with_key__with_callobjectcache___seq_find__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5)) int DCALL default__seq_find_with_key__with_callmethodcache___seq_find__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5)) int DCALL default__seq_find_with_key__with_callkwmethodcache___seq_find__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5)) int DCALL default__seq_find_with_key__unsupported(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
#define default__seq_find_with_key__empty (*(int (DCALL *)(DeeObject *, DeeObject *, size_t, size_t, DeeObject *))&_DeeNone_reti0_5)
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5)) int DCALL default__seq_find_with_key__with__seq_operator_enumerate_index(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);

/* seq_erase */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_erase__with_callattr_erase(DeeObject *__restrict self, size_t index, size_t count);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_erase__with_callattr___seq_erase__(DeeObject *__restrict self, size_t index, size_t count);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_erase__with_callobjectcache___seq_erase__(DeeObject *__restrict self, size_t index, size_t count);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_erase__with_callmethodcache___seq_erase__(DeeObject *__restrict self, size_t index, size_t count);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_erase__with_callkwmethodcache___seq_erase__(DeeObject *__restrict self, size_t index, size_t count);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_erase__unsupported(DeeObject *__restrict self, size_t index, size_t count);
#define default__seq_erase__empty (*(int (DCALL *)(DeeObject *__restrict, size_t, size_t))&_DeeNone_reti0_3)
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_erase__with__seq_operator_delrange_index(DeeObject *__restrict self, size_t index, size_t count);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_erase__with__seq_pop(DeeObject *__restrict self, size_t index, size_t count);

/* seq_insert */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_insert__with_callattr_insert(DeeObject *self, size_t index, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_insert__with_callattr___seq_insert__(DeeObject *self, size_t index, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_insert__with_callobjectcache___seq_insert__(DeeObject *self, size_t index, DeeObject *item);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_insert__with_callmethodcache___seq_insert__(DeeObject *self, size_t index, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_insert__with_callkwmethodcache___seq_insert__(DeeObject *self, size_t index, DeeObject *item);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_insert__unsupported(DeeObject *self, size_t index, DeeObject *item);
#define default__seq_insert__empty (*(int (DCALL *)(DeeObject *, size_t, DeeObject *))&default__seq_insert__unsupported)
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_insert__with__seq_insertall(DeeObject *self, size_t index, DeeObject *item);

/* seq_insertall */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_insertall__with_callattr_insertall(DeeObject *self, size_t index, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_insertall__with_callattr___seq_insertall__(DeeObject *self, size_t index, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_insertall__with_callobjectcache___seq_insertall__(DeeObject *self, size_t index, DeeObject *items);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_insertall__with_callmethodcache___seq_insertall__(DeeObject *self, size_t index, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_insertall__with_callkwmethodcache___seq_insertall__(DeeObject *self, size_t index, DeeObject *items);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_insertall__unsupported(DeeObject *self, size_t index, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_insertall__empty(DeeObject *self, size_t index, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_insertall__with__seq_operator_setrange_index(DeeObject *self, size_t index, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_insertall__with__seq_insert(DeeObject *self, size_t index, DeeObject *items);

/* seq_pushfront */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_pushfront__with_callattr_pushfront(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_pushfront__with_callattr___seq_pushfront__(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_pushfront__with_callobjectcache___seq_pushfront__(DeeObject *self, DeeObject *item);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_pushfront__with_callmethodcache___seq_pushfront__(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_pushfront__with_callkwmethodcache___seq_pushfront__(DeeObject *self, DeeObject *item);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_pushfront__unsupported(DeeObject *self, DeeObject *item);
#define default__seq_pushfront__empty (*(int (DCALL *)(DeeObject *, DeeObject *))&default__seq_pushfront__unsupported)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_pushfront__with__seq_insert(DeeObject *self, DeeObject *item);

/* seq_append */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_append__with_callattr_append(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_append__with_callattr_pushback(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_append__with_callattr___seq_append__(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_append__with_callobjectcache___seq_append__(DeeObject *self, DeeObject *item);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_append__with_callmethodcache___seq_append__(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_append__with_callkwmethodcache___seq_append__(DeeObject *self, DeeObject *item);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_append__unsupported(DeeObject *self, DeeObject *item);
#define default__seq_append__empty (*(int (DCALL *)(DeeObject *, DeeObject *))&default__seq_append__unsupported)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_append__with__seq_extend(DeeObject *self, DeeObject *item);

/* seq_extend */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_extend__with_callattr_extend(DeeObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_extend__with_callattr___seq_extend__(DeeObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_extend__with_callobjectcache___seq_extend__(DeeObject *self, DeeObject *items);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_extend__with_callmethodcache___seq_extend__(DeeObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_extend__with_callkwmethodcache___seq_extend__(DeeObject *self, DeeObject *items);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_extend__unsupported(DeeObject *self, DeeObject *items);
#define default__seq_extend__empty (*(int (DCALL *)(DeeObject *, DeeObject *))&default__seq_extend__unsupported)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_extend__with__seq_operator_size__and__seq_operator_setrange_index(DeeObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_extend__with__seq_operator_size__and__seq_insertall(DeeObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_extend__with__seq_append(DeeObject *self, DeeObject *items);

/* seq_xchitem_index */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_xchitem_index__with_callattr_xchitem(DeeObject *self, size_t index, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_xchitem_index__with_callattr___seq_xchitem__(DeeObject *self, size_t index, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_xchitem_index__with_callobjectcache___seq_xchitem__(DeeObject *self, size_t index, DeeObject *item);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_xchitem_index__with_callmethodcache___seq_xchitem__(DeeObject *self, size_t index, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_xchitem_index__with_callkwmethodcache___seq_xchitem__(DeeObject *self, size_t index, DeeObject *item);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_xchitem_index__unsupported(DeeObject *self, size_t index, DeeObject *item);
#define default__seq_xchitem_index__empty (*(DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *))&default__seq_xchitem_index__unsupported)
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_xchitem_index__with__seq_operator_getitem_index__and__seq_operator_setitem_index(DeeObject *self, size_t index, DeeObject *item);

/* seq_clear */
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_clear__with_callattr_clear(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_clear__with_callattr___seq_clear__(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_clear__with_callobjectcache___seq_clear__(DeeObject *self);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_clear__with_callmethodcache___seq_clear__(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_clear__with_callkwmethodcache___seq_clear__(DeeObject *self);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_clear__unsupported(DeeObject *self);
#define default__seq_clear__empty (*(int (DCALL *)(DeeObject *))&_DeeNone_reti0_1)
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_clear__with__seq_operator_delrange(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_clear__with__seq_operator_delrange_index_n(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_clear__with__seq_erase(DeeObject *self);

/* seq_pop */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_pop__with_callattr_pop(DeeObject *self, Dee_ssize_t index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_pop__with_callattr___seq_pop__(DeeObject *self, Dee_ssize_t index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_pop__with_callobjectcache___seq_pop__(DeeObject *self, Dee_ssize_t index);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_pop__with_callmethodcache___seq_pop__(DeeObject *self, Dee_ssize_t index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_pop__with_callkwmethodcache___seq_pop__(DeeObject *self, Dee_ssize_t index);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_pop__unsupported(DeeObject *self, Dee_ssize_t index);
#define default__seq_pop__empty (*(DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t))&default__seq_pop__unsupported)
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_pop__with__seq_operator_size__and__seq_operator_getitem_index__and__seq_erase(DeeObject *self, Dee_ssize_t index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_pop__with__seq_operator_size__and__seq_operator_getitem_index__and__seq_operator_delitem_index(DeeObject *self, Dee_ssize_t index);

/* set_operator_iter */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__set_operator_iter__with_callattr___set_iter__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__set_operator_iter__with_callobjectcache___set_iter__(DeeObject *__restrict self);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__set_operator_iter__with_callmethodcache___set_iter__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__set_operator_iter__with_callkwmethodcache___set_iter__(DeeObject *__restrict self);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__set_operator_iter__unsupported(DeeObject *__restrict self);
#define default__set_operator_iter__empty (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&default__seq_operator_iter__empty)
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__set_operator_iter__with__seq_operator_iter(DeeObject *__restrict self);

/* set_operator_foreach */
#define default__set_operator_foreach__with_callattr___set_iter__ (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&default__set_operator_foreach__with__set_operator_iter)
#define default__set_operator_foreach__with_callobjectcache___set_iter__ (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&default__set_operator_foreach__with__set_operator_iter)
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
#define default__set_operator_foreach__with_callmethodcache___set_iter__ (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&default__set_operator_foreach__with__set_operator_iter)
#define default__set_operator_foreach__with_callkwmethodcache___set_iter__ (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&default__set_operator_foreach__with__set_operator_iter)
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
#define default__set_operator_foreach__unsupported (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&default__set_operator_foreach__with__set_operator_iter)
#define default__set_operator_foreach__empty (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&_DeeNone_rets0_3)
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL default__set_operator_foreach__with__seq_operator_foreach(DeeObject *__restrict self, Dee_foreach_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL default__set_operator_foreach__with__set_operator_iter(DeeObject *__restrict self, Dee_foreach_t cb, void *arg);
/*[[[end]]]*/
/* clang-format on */

DECL_END
#endif /* CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */

#endif /* !GUARD_DEEMON_RUNTIME_METHOD_HINT_DEFAULTS_H */
