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
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_bool(DeeObject *__restrict self);
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
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_bool__with__seq_operator_sizeob(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_bool__with__seq_operator_compare_eq(DeeObject *__restrict self);

/* seq_operator_sizeob */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_sizeob(DeeObject *self);
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
INTDEF WUNUSED NONNULL((1)) size_t DCALL default__seq_operator_size(DeeObject *self);
#define default__seq_operator_size__with_callattr___seq_size__ (*(size_t (DCALL *)(DeeObject *))&default__seq_operator_size__with__seq_operator_sizeob)
#define default__seq_operator_size__with_callobjectcache___seq_size__ (*(size_t (DCALL *)(DeeObject *))&default__seq_operator_size__with__seq_operator_sizeob)
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
#define default__seq_operator_size__with_callmethodcache___seq_size__ (*(size_t (DCALL *)(DeeObject *))&default__seq_operator_size__with__seq_operator_sizeob)
#define default__seq_operator_size__with_callkwmethodcache___seq_size__ (*(size_t (DCALL *)(DeeObject *))&default__seq_operator_size__with__seq_operator_sizeob)
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1)) size_t DCALL default__seq_operator_size__unsupported(DeeObject *self);
#define default__seq_operator_size__empty (*(size_t (DCALL *)(DeeObject *))&_DeeNone_rets0_1)
INTDEF WUNUSED NONNULL((1)) size_t DCALL default__seq_operator_size__with__seq_operator_foreach(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) size_t DCALL default__seq_operator_size__with__seq_operator_foreach_pair(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) size_t DCALL default__seq_operator_size__with__seq_operator_sizeob(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) size_t DCALL default__seq_operator_size__with__map_enumerate(DeeObject *self);

/* seq_operator_iter */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_iter(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_iter__with_callattr___seq_iter__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_iter__with_callobjectcache___seq_iter__(DeeObject *__restrict self);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_iter__with_callmethodcache___seq_iter__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_iter__with_callkwmethodcache___seq_iter__(DeeObject *__restrict self);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_iter__unsupported(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_iter__empty(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_iter__with__seq_operator_size__and__operator_getitem_index_fast(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_iter__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_iter__with__seq_operator_size__and__seq_operator_getitem_index(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_iter__with__seq_operator_getitem_index(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_iter__with__seq_operator_sizeob__and__seq_operator_getitem(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_iter__with__seq_operator_getitem(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_iter__with__map_enumerate(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_iter__with__map_iterkeys__and__map_operator_trygetitem(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_iter__with__map_iterkeys__and__map_operator_getitem(DeeObject *__restrict self);

/* seq_operator_foreach */
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_operator_foreach(DeeObject *__restrict self, Dee_foreach_t cb, void *arg);
#define default__seq_operator_foreach__with_callattr___seq_iter__ (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&default__seq_operator_foreach__with__seq_operator_iter)
#define default__seq_operator_foreach__with_callobjectcache___seq_iter__ (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&default__seq_operator_foreach__with__seq_operator_iter)
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
#define default__seq_operator_foreach__with_callmethodcache___seq_iter__ (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&default__seq_operator_foreach__with__seq_operator_iter)
#define default__seq_operator_foreach__with_callkwmethodcache___seq_iter__ (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&default__seq_operator_foreach__with__seq_operator_iter)
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
#define default__seq_operator_foreach__unsupported (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&default__seq_operator_foreach__with__seq_operator_iter)
#define default__seq_operator_foreach__empty (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&_DeeNone_rets0_3)
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_operator_foreach__with__seq_operator_size__and__operator_getitem_index_fast(DeeObject *__restrict self, Dee_foreach_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *__restrict self, Dee_foreach_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_getitem_index(DeeObject *__restrict self, Dee_foreach_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_operator_foreach__with__seq_operator_sizeob__and__seq_operator_getitem(DeeObject *__restrict self, Dee_foreach_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_operator_foreach__with__seq_operator_foreach_pair(DeeObject *__restrict self, Dee_foreach_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_operator_foreach__with__seq_operator_iter(DeeObject *__restrict self, Dee_foreach_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_operator_foreach__with__map_enumerate(DeeObject *__restrict self, Dee_foreach_t cb, void *arg);

/* seq_operator_foreach_pair */
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_operator_foreach_pair(DeeObject *__restrict self, Dee_foreach_pair_t cb, void *arg);
#define default__seq_operator_foreach_pair__with_callattr___seq_iter__ (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&default__seq_operator_foreach_pair__with__seq_operator_foreach)
#define default__seq_operator_foreach_pair__with_callobjectcache___seq_iter__ (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&default__seq_operator_foreach_pair__with__seq_operator_foreach)
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
#define default__seq_operator_foreach_pair__with_callmethodcache___seq_iter__ (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&default__seq_operator_foreach_pair__with__seq_operator_foreach)
#define default__seq_operator_foreach_pair__with_callkwmethodcache___seq_iter__ (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&default__seq_operator_foreach_pair__with__seq_operator_foreach)
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
#define default__seq_operator_foreach_pair__unsupported (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&default__seq_operator_foreach_pair__with__seq_operator_foreach)
#define default__seq_operator_foreach_pair__empty (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&_DeeNone_rets0_3)
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_operator_foreach_pair__with__seq_operator_foreach(DeeObject *__restrict self, Dee_foreach_pair_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_operator_foreach_pair__with__seq_operator_iter(DeeObject *__restrict self, Dee_foreach_pair_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_operator_foreach_pair__with__map_enumerate(DeeObject *__restrict self, Dee_foreach_pair_t cb, void *arg);

/* seq_operator_getitem */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_getitem(DeeObject *self, DeeObject *index);
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
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_getitem_index(DeeObject *__restrict self, size_t index);
#define default__seq_operator_getitem_index__with_callattr___seq_getitem__ (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict, size_t))&default__seq_operator_getitem_index__with__seq_operator_getitem)
#define default__seq_operator_getitem_index__with_callobjectcache___seq_getitem__ (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict, size_t))&default__seq_operator_getitem_index__with__seq_operator_getitem)
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
#define default__seq_operator_getitem_index__with_callmethodcache___seq_getitem__ (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict, size_t))&default__seq_operator_getitem_index__with__seq_operator_getitem)
#define default__seq_operator_getitem_index__with_callkwmethodcache___seq_getitem__ (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict, size_t))&default__seq_operator_getitem_index__with__seq_operator_getitem)
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_getitem_index__unsupported(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_getitem_index__empty(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_getitem_index__with__seq_operator_foreach(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_getitem_index__with__seq_operator_getitem(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_getitem_index__with__map_enumerate(DeeObject *__restrict self, size_t index);

/* seq_operator_trygetitem */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_trygetitem(DeeObject *self, DeeObject *index);
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

/* seq_operator_trygetitem_index */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_trygetitem_index(DeeObject *__restrict self, size_t index);
#define default__seq_operator_trygetitem_index__with_callattr___seq_getitem__ (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict, size_t))&default__seq_operator_trygetitem_index__with__seq_operator_getitem_index)
#define default__seq_operator_trygetitem_index__with_callobjectcache___seq_getitem__ (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict, size_t))&default__seq_operator_trygetitem_index__with__seq_operator_getitem_index)
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
#define default__seq_operator_trygetitem_index__with_callmethodcache___seq_getitem__ (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict, size_t))&default__seq_operator_trygetitem_index__with__seq_operator_getitem_index)
#define default__seq_operator_trygetitem_index__with_callkwmethodcache___seq_getitem__ (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict, size_t))&default__seq_operator_trygetitem_index__with__seq_operator_getitem_index)
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
#define default__seq_operator_trygetitem_index__unsupported (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict, size_t))&default__seq_operator_getitem_index__unsupported)
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_trygetitem_index__empty(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_trygetitem_index__with__seq_operator_getitem_index(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_trygetitem_index__with__seq_operator_foreach(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_trygetitem_index__with__map_enumerate(DeeObject *__restrict self, size_t index);

/* seq_operator_hasitem */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_hasitem(DeeObject *self, DeeObject *index);
#define default__seq_operator_hasitem__with_callattr___seq_getitem__ (*(int (DCALL *)(DeeObject *, DeeObject *))&default__seq_operator_hasitem__with__seq_operator_getitem)
#define default__seq_operator_hasitem__with_callobjectcache___seq_getitem__ (*(int (DCALL *)(DeeObject *, DeeObject *))&default__seq_operator_hasitem__with__seq_operator_getitem)
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
#define default__seq_operator_hasitem__with_callmethodcache___seq_getitem__ (*(int (DCALL *)(DeeObject *, DeeObject *))&default__seq_operator_hasitem__with__seq_operator_getitem)
#define default__seq_operator_hasitem__with_callkwmethodcache___seq_getitem__ (*(int (DCALL *)(DeeObject *, DeeObject *))&default__seq_operator_hasitem__with__seq_operator_getitem)
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_hasitem__unsupported(DeeObject *self, DeeObject *index);
#define default__seq_operator_hasitem__empty (*(int (DCALL *)(DeeObject *, DeeObject *))&_DeeNone_reti0_2)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_hasitem__with__seq_operator_sizeob(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_hasitem__with__seq_operator_hasitem_index(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_hasitem__with__seq_operator_getitem(DeeObject *self, DeeObject *index);

/* seq_operator_hasitem_index */
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_hasitem_index(DeeObject *__restrict self, size_t index);
#define default__seq_operator_hasitem_index__with_callattr___seq_getitem__ (*(int (DCALL *)(DeeObject *__restrict, size_t))&default__seq_operator_hasitem_index__with__seq_operator_getitem_index)
#define default__seq_operator_hasitem_index__with_callobjectcache___seq_getitem__ (*(int (DCALL *)(DeeObject *__restrict, size_t))&default__seq_operator_hasitem_index__with__seq_operator_getitem_index)
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
#define default__seq_operator_hasitem_index__with_callmethodcache___seq_getitem__ (*(int (DCALL *)(DeeObject *__restrict, size_t))&default__seq_operator_hasitem_index__with__seq_operator_getitem_index)
#define default__seq_operator_hasitem_index__with_callkwmethodcache___seq_getitem__ (*(int (DCALL *)(DeeObject *__restrict, size_t))&default__seq_operator_hasitem_index__with__seq_operator_getitem_index)
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_hasitem_index__unsupported(DeeObject *__restrict self, size_t index);
#define default__seq_operator_hasitem_index__empty (*(int (DCALL *)(DeeObject *__restrict, size_t))&_DeeNone_reti0_2)
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_hasitem_index__with__seq_operator_size(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_hasitem_index__with__seq_operator_getitem_index(DeeObject *__restrict self, size_t index);

/* seq_operator_bounditem */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_bounditem(DeeObject *self, DeeObject *index);
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
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_bounditem_index(DeeObject *__restrict self, size_t index);
#define default__seq_operator_bounditem_index__with_callattr___seq_getitem__ (*(int (DCALL *)(DeeObject *__restrict, size_t))&default__seq_operator_bounditem_index__with__seq_operator_getitem_index)
#define default__seq_operator_bounditem_index__with_callobjectcache___seq_getitem__ (*(int (DCALL *)(DeeObject *__restrict, size_t))&default__seq_operator_bounditem_index__with__seq_operator_getitem_index)
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
#define default__seq_operator_bounditem_index__with_callmethodcache___seq_getitem__ (*(int (DCALL *)(DeeObject *__restrict, size_t))&default__seq_operator_bounditem_index__with__seq_operator_getitem_index)
#define default__seq_operator_bounditem_index__with_callkwmethodcache___seq_getitem__ (*(int (DCALL *)(DeeObject *__restrict, size_t))&default__seq_operator_bounditem_index__with__seq_operator_getitem_index)
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_bounditem_index__unsupported(DeeObject *__restrict self, size_t index);
#define default__seq_operator_bounditem_index__empty (*(int (DCALL *)(DeeObject *__restrict, size_t))&_DeeNone_reti0_2)
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_bounditem_index__with__seq_operator_getitem_index(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_bounditem_index__with__map_enumerate(DeeObject *__restrict self, size_t index);

/* seq_operator_delitem */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_delitem(DeeObject *self, DeeObject *index);
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
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_delitem_index(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_delitem_index__with_callattr___seq_delitem__(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_delitem_index__with_callobjectcache___seq_delitem__(DeeObject *__restrict self, size_t index);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_delitem_index__with_callmethodcache___seq_delitem__(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_delitem_index__with_callkwmethodcache___seq_delitem__(DeeObject *__restrict self, size_t index);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_delitem_index__unsupported(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_delitem_index__empty(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_delitem_index__with__seq_operator_delrange_index(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_delitem_index__with__seq_operator_size__and__seq_erase(DeeObject *__restrict self, size_t index);

/* seq_operator_setitem */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_operator_setitem(DeeObject *self, DeeObject *index, DeeObject *value);
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
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_operator_setitem_index(DeeObject *self, size_t index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_operator_setitem_index__with_callattr___seq_setitem__(DeeObject *self, size_t index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_operator_setitem_index__with_callobjectcache___seq_setitem__(DeeObject *self, size_t index, DeeObject *value);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_operator_setitem_index__with_callmethodcache___seq_setitem__(DeeObject *self, size_t index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_operator_setitem_index__with_callkwmethodcache___seq_setitem__(DeeObject *self, size_t index, DeeObject *value);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_operator_setitem_index__unsupported(DeeObject *self, size_t index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_operator_setitem_index__empty(DeeObject *self, size_t index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_operator_setitem_index__with__seq_operator_setrange_index(DeeObject *self, size_t index, DeeObject *value);

/* seq_operator_getrange */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_operator_getrange(DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_operator_getrange__with_callattr___seq_getrange__(DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_operator_getrange__with_callobjectcache___seq_getrange__(DeeObject *self, DeeObject *start, DeeObject *end);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_operator_getrange__with_callmethodcache___seq_getrange__(DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_operator_getrange__with_callkwmethodcache___seq_getrange__(DeeObject *self, DeeObject *start, DeeObject *end);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_operator_getrange__unsupported(DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_operator_getrange__empty(DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_operator_getrange__with__seq_operator_getrange_index__and__seq_operator_getrange_index_n(DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_operator_getrange__with__seq_operator_sizeob__and__seq_operator_getitem(DeeObject *self, DeeObject *start, DeeObject *end);

/* seq_operator_getrange_index */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_getrange_index(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);
#define default__seq_operator_getrange_index__with_callattr___seq_getrange__ (*(DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&default__seq_operator_getrange_index__with__seq_operator_getrange)
#define default__seq_operator_getrange_index__with_callobjectcache___seq_getrange__ (*(DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&default__seq_operator_getrange_index__with__seq_operator_getrange)
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
#define default__seq_operator_getrange_index__with_callmethodcache___seq_getrange__ (*(DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&default__seq_operator_getrange_index__with__seq_operator_getrange)
#define default__seq_operator_getrange_index__with_callkwmethodcache___seq_getrange__ (*(DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&default__seq_operator_getrange_index__with__seq_operator_getrange)
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_getrange_index__unsupported(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_getrange_index__empty(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_getrange_index__with__seq_operator_getrange(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_getrange_index__with__seq_operator_size__and__operator_getitem_index_fast(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_getitem_index(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_getitem(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_iter(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);

/* seq_operator_getrange_index_n */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_getrange_index_n(DeeObject *self, Dee_ssize_t start);
#define default__seq_operator_getrange_index_n__with_callattr___seq_getrange__ (*(DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t))&default__seq_operator_getrange_index_n__with__seq_operator_getrange)
#define default__seq_operator_getrange_index_n__with_callobjectcache___seq_getrange__ (*(DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t))&default__seq_operator_getrange_index_n__with__seq_operator_getrange)
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
#define default__seq_operator_getrange_index_n__with_callmethodcache___seq_getrange__ (*(DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t))&default__seq_operator_getrange_index_n__with__seq_operator_getrange)
#define default__seq_operator_getrange_index_n__with_callkwmethodcache___seq_getrange__ (*(DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t))&default__seq_operator_getrange_index_n__with__seq_operator_getrange)
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_getrange_index_n__unsupported(DeeObject *self, Dee_ssize_t start);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_getrange_index_n__empty(DeeObject *self, Dee_ssize_t start);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_getrange_index_n__with__seq_operator_getrange(DeeObject *self, Dee_ssize_t start);
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
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_operator_delrange__with_callmethodcache___seq_delrange__(DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_operator_delrange__with_callkwmethodcache___seq_delrange__(DeeObject *self, DeeObject *start, DeeObject *end);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_operator_delrange__unsupported(DeeObject *self, DeeObject *start, DeeObject *end);
#define default__seq_operator_delrange__empty (*(int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&_DeeNone_reti0_3)
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_operator_delrange__with__seq_operator_delrange_index__and__seq_operator_delrange_index_n(DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_operator_delrange__with__seq_operator_setrange(DeeObject *self, DeeObject *start, DeeObject *end);

/* seq_operator_delrange_index */
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_delrange_index(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);
#define default__seq_operator_delrange_index__with_callattr___seq_delrange__ (*(int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&default__seq_operator_delrange_index__with__seq_operator_delrange)
#define default__seq_operator_delrange_index__with_callobjectcache___seq_delrange__ (*(int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&default__seq_operator_delrange_index__with__seq_operator_delrange)
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
#define default__seq_operator_delrange_index__with_callmethodcache___seq_delrange__ (*(int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&default__seq_operator_delrange_index__with__seq_operator_delrange)
#define default__seq_operator_delrange_index__with_callkwmethodcache___seq_delrange__ (*(int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&default__seq_operator_delrange_index__with__seq_operator_delrange)
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_delrange_index__unsupported(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);
#define default__seq_operator_delrange_index__empty (*(int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&_DeeNone_reti0_3)
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_delrange_index__with__seq_operator_delrange(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_delrange_index__with__seq_operator_size__and__seq_erase(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_delrange_index__with__seq_operator_setrange_index(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);

/* seq_operator_delrange_index_n */
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_delrange_index_n(DeeObject *self, Dee_ssize_t start);
#define default__seq_operator_delrange_index_n__with_callattr___seq_delrange__ (*(int (DCALL *)(DeeObject *, Dee_ssize_t))&default__seq_operator_delrange_index_n__with__seq_operator_delrange)
#define default__seq_operator_delrange_index_n__with_callobjectcache___seq_delrange__ (*(int (DCALL *)(DeeObject *, Dee_ssize_t))&default__seq_operator_delrange_index_n__with__seq_operator_delrange)
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
#define default__seq_operator_delrange_index_n__with_callmethodcache___seq_delrange__ (*(int (DCALL *)(DeeObject *, Dee_ssize_t))&default__seq_operator_delrange_index_n__with__seq_operator_delrange)
#define default__seq_operator_delrange_index_n__with_callkwmethodcache___seq_delrange__ (*(int (DCALL *)(DeeObject *, Dee_ssize_t))&default__seq_operator_delrange_index_n__with__seq_operator_delrange)
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_delrange_index_n__unsupported(DeeObject *self, Dee_ssize_t start);
#define default__seq_operator_delrange_index_n__empty (*(int (DCALL *)(DeeObject *, Dee_ssize_t))&_DeeNone_reti0_2)
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_delrange_index_n__with__seq_operator_delrange_index(DeeObject *self, Dee_ssize_t start);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_delrange_index_n__with__seq_operator_delrange(DeeObject *self, Dee_ssize_t start);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_delrange_index_n__with__seq_operator_setrange_index_n(DeeObject *self, Dee_ssize_t start);

/* seq_operator_setrange */
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL default__seq_operator_setrange(DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL default__seq_operator_setrange__with_callattr___seq_setrange__(DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL default__seq_operator_setrange__with_callobjectcache___seq_setrange__(DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *items);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL default__seq_operator_setrange__with_callmethodcache___seq_setrange__(DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL default__seq_operator_setrange__with_callkwmethodcache___seq_setrange__(DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *items);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL default__seq_operator_setrange__unsupported(DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL default__seq_operator_setrange__empty(DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL default__seq_operator_setrange__with__seq_operator_setrange_index__and__seq_operator_setrange_index_n(DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *items);

/* seq_operator_setrange_index */
INTDEF WUNUSED NONNULL((1, 4)) int DCALL default__seq_operator_setrange_index(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *items);
#define default__seq_operator_setrange_index__with_callattr___seq_setrange__ (*(int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t, DeeObject *))&default__seq_operator_setrange_index__with__seq_operator_setrange)
#define default__seq_operator_setrange_index__with_callobjectcache___seq_setrange__ (*(int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t, DeeObject *))&default__seq_operator_setrange_index__with__seq_operator_setrange)
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
#define default__seq_operator_setrange_index__with_callmethodcache___seq_setrange__ (*(int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t, DeeObject *))&default__seq_operator_setrange_index__with__seq_operator_setrange)
#define default__seq_operator_setrange_index__with_callkwmethodcache___seq_setrange__ (*(int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t, DeeObject *))&default__seq_operator_setrange_index__with__seq_operator_setrange)
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 4)) int DCALL default__seq_operator_setrange_index__unsupported(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL default__seq_operator_setrange_index__empty(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL default__seq_operator_setrange_index__with__seq_operator_setrange(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL default__seq_operator_setrange_index__with__seq_operator_size__and__seq_erase__and__seq_insertall(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *items);

/* seq_operator_setrange_index_n */
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_operator_setrange_index_n(DeeObject *self, Dee_ssize_t start, DeeObject *items);
#define default__seq_operator_setrange_index_n__with_callattr___seq_setrange__ (*(int (DCALL *)(DeeObject *, Dee_ssize_t, DeeObject *))&default__seq_operator_setrange_index_n__with__seq_operator_setrange)
#define default__seq_operator_setrange_index_n__with_callobjectcache___seq_setrange__ (*(int (DCALL *)(DeeObject *, Dee_ssize_t, DeeObject *))&default__seq_operator_setrange_index_n__with__seq_operator_setrange)
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
#define default__seq_operator_setrange_index_n__with_callmethodcache___seq_setrange__ (*(int (DCALL *)(DeeObject *, Dee_ssize_t, DeeObject *))&default__seq_operator_setrange_index_n__with__seq_operator_setrange)
#define default__seq_operator_setrange_index_n__with_callkwmethodcache___seq_setrange__ (*(int (DCALL *)(DeeObject *, Dee_ssize_t, DeeObject *))&default__seq_operator_setrange_index_n__with__seq_operator_setrange)
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_operator_setrange_index_n__unsupported(DeeObject *self, Dee_ssize_t start, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_operator_setrange_index_n__empty(DeeObject *self, Dee_ssize_t start, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_operator_setrange_index_n__with__seq_operator_setrange_index(DeeObject *self, Dee_ssize_t start, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_operator_setrange_index_n__with__seq_operator_setrange(DeeObject *self, Dee_ssize_t start, DeeObject *items);

/* seq_operator_assign */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_assign(DeeObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_assign__with_callattr___seq_bool__(DeeObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_assign__with_callobjectcache___seq_bool__(DeeObject *self, DeeObject *items);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_assign__with_callmethodcache___seq_bool__(DeeObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_assign__with_callkwmethodcache___seq_bool__(DeeObject *self, DeeObject *items);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_assign__unsupported(DeeObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_assign__with__seq_operator_setrange(DeeObject *self, DeeObject *items);

/* seq_operator_hash */
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL default__seq_operator_hash(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL default__seq_operator_hash__with_callattr___seq_hash__(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL default__seq_operator_hash__with_callobjectcache___seq_hash__(DeeObject *self);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL default__seq_operator_hash__with_callmethodcache___seq_hash__(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL default__seq_operator_hash__with_callkwmethodcache___seq_hash__(DeeObject *self);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL default__seq_operator_hash__unsupported(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL default__seq_operator_hash__empty(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL default__seq_operator_hash__with__seq_operator_foreach(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL default__seq_operator_hash__with__seq_operator_size__and__operator_getitem_index_fast(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL default__seq_operator_hash__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL default__seq_operator_hash__with__seq_operator_size__and__seq_operator_getitem_index(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL default__seq_operator_hash__with__seq_operator_sizeob__and__seq_operator_getitem(DeeObject *self);

/* seq_operator_compare */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_compare(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_compare__with_callattr___seq_compare__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_compare__with_callobjectcache___seq_compare__(DeeObject *lhs, DeeObject *rhs);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_compare__with_callmethodcache___seq_compare__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_compare__with_callkwmethodcache___seq_compare__(DeeObject *lhs, DeeObject *rhs);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
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
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_compare_eq__with_callattr___seq_compare_eq__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_compare_eq__with_callobjectcache___seq_compare_eq__(DeeObject *lhs, DeeObject *rhs);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_compare_eq__with_callmethodcache___seq_compare_eq__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_compare_eq__with_callkwmethodcache___seq_compare_eq__(DeeObject *lhs, DeeObject *rhs);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
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
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_trycompare_eq__with_callattr___seq_trycompare_eq__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_trycompare_eq__with_callobjectcache___seq_trycompare_eq__(DeeObject *lhs, DeeObject *rhs);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_trycompare_eq__with_callmethodcache___seq_trycompare_eq__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_trycompare_eq__with_callkwmethodcache___seq_trycompare_eq__(DeeObject *lhs, DeeObject *rhs);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_trycompare_eq__unsupported(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_trycompare_eq__empty(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_trycompare_eq__with__seq_operator_foreach(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_trycompare_eq__with__seq_operator_size__and__operator_getitem_index_fast(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_trycompare_eq__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_trycompare_eq__with__seq_operator_size__and__seq_operator_getitem_index(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_trycompare_eq__with__seq_operator_sizeob__and__seq_operator_getitem(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_trycompare_eq__with__seq_operator_compare_eq(DeeObject *lhs, DeeObject *rhs);

/* seq_operator_eq */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_eq(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_eq__with_callattr___seq_eq__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_eq__with_callobjectcache___seq_eq__(DeeObject *lhs, DeeObject *rhs);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_eq__with_callmethodcache___seq_eq__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_eq__with_callkwmethodcache___seq_eq__(DeeObject *lhs, DeeObject *rhs);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_eq__unsupported(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_eq__with__seq_operator_compare_eq(DeeObject *lhs, DeeObject *rhs);

/* seq_operator_ne */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_ne(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_ne__with_callattr___seq_ne__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_ne__with_callobjectcache___seq_ne__(DeeObject *lhs, DeeObject *rhs);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_ne__with_callmethodcache___seq_ne__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_ne__with_callkwmethodcache___seq_ne__(DeeObject *lhs, DeeObject *rhs);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_ne__unsupported(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_ne__with__seq_operator_compare_eq(DeeObject *lhs, DeeObject *rhs);

/* seq_operator_lo */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_lo(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_lo__with_callattr___seq_lo__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_lo__with_callobjectcache___seq_lo__(DeeObject *lhs, DeeObject *rhs);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_lo__with_callmethodcache___seq_lo__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_lo__with_callkwmethodcache___seq_lo__(DeeObject *lhs, DeeObject *rhs);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_lo__unsupported(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_lo__with__seq_operator_compare(DeeObject *lhs, DeeObject *rhs);

/* seq_operator_le */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_le(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_le__with_callattr___seq_le__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_le__with_callobjectcache___seq_le__(DeeObject *lhs, DeeObject *rhs);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_le__with_callmethodcache___seq_le__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_le__with_callkwmethodcache___seq_le__(DeeObject *lhs, DeeObject *rhs);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_le__unsupported(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_le__with__seq_operator_compare(DeeObject *lhs, DeeObject *rhs);

/* seq_operator_gr */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_gr(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_gr__with_callattr___seq_gr__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_gr__with_callobjectcache___seq_gr__(DeeObject *lhs, DeeObject *rhs);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_gr__with_callmethodcache___seq_gr__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_gr__with_callkwmethodcache___seq_gr__(DeeObject *lhs, DeeObject *rhs);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_gr__unsupported(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_gr__with__seq_operator_compare(DeeObject *lhs, DeeObject *rhs);

/* seq_operator_ge */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_ge(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_ge__with_callattr___seq_ge__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_ge__with_callobjectcache___seq_ge__(DeeObject *lhs, DeeObject *rhs);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_ge__with_callmethodcache___seq_ge__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_ge__with_callkwmethodcache___seq_ge__(DeeObject *lhs, DeeObject *rhs);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_ge__unsupported(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_ge__with__seq_operator_compare(DeeObject *lhs, DeeObject *rhs);

/* seq_operator_inplace_add */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_inplace_add(DREF DeeObject **__restrict p_self, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_inplace_add__with_callattr___seq_inplace_add__(DREF DeeObject **__restrict p_self, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_inplace_add__with_callobjectcache___seq_inplace_add__(DREF DeeObject **__restrict p_self, DeeObject *rhs);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_inplace_add__with_callmethodcache___seq_inplace_add__(DREF DeeObject **__restrict p_self, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_inplace_add__with_callkwmethodcache___seq_inplace_add__(DREF DeeObject **__restrict p_self, DeeObject *rhs);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
#define default__seq_operator_inplace_add__unsupported (*(int (DCALL *)(DREF DeeObject **__restrict, DeeObject *))&default__seq_operator_inplace_add__with__DeeSeq_Concat)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_inplace_add__with__seq_extend(DREF DeeObject **__restrict p_self, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_inplace_add__with__DeeSeq_Concat(DREF DeeObject **__restrict p_self, DeeObject *rhs);

/* seq_operator_inplace_mul */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_inplace_mul(DREF DeeObject **__restrict p_self, DeeObject *repeat);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_inplace_mul__with_callattr___seq_inplace_mul__(DREF DeeObject **__restrict p_self, DeeObject *repeat);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_inplace_mul__with_callobjectcache___seq_inplace_mul__(DREF DeeObject **__restrict p_self, DeeObject *repeat);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_inplace_mul__with_callmethodcache___seq_inplace_mul__(DREF DeeObject **__restrict p_self, DeeObject *repeat);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_inplace_mul__with_callkwmethodcache___seq_inplace_mul__(DREF DeeObject **__restrict p_self, DeeObject *repeat);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
#define default__seq_operator_inplace_mul__unsupported (*(int (DCALL *)(DREF DeeObject **__restrict, DeeObject *))&default__seq_operator_inplace_mul__with__DeeSeq_Repeat)
#define default__seq_operator_inplace_mul__empty (*(int (DCALL *)(DREF DeeObject **__restrict, DeeObject *))&_DeeNone_reti0_2)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_inplace_mul__with__seq_clear__and__seq_extend(DREF DeeObject **__restrict p_self, DeeObject *repeat);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_operator_inplace_mul__with__DeeSeq_Repeat(DREF DeeObject **__restrict p_self, DeeObject *repeat);

/* seq_enumerate */
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_enumerate__with_callattr___seq_enumerate__(DeeObject *__restrict self, Dee_seq_enumerate_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_enumerate__with_callobjectcache___seq_enumerate__(DeeObject *__restrict self, Dee_seq_enumerate_t proc, void *arg);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_enumerate__with_callmethodcache___seq_enumerate__(DeeObject *__restrict self, Dee_seq_enumerate_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_enumerate__with_callkwmethodcache___seq_enumerate__(DeeObject *__restrict self, Dee_seq_enumerate_t proc, void *arg);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_enumerate__unsupported(DeeObject *__restrict self, Dee_seq_enumerate_t proc, void *arg);
#define default__seq_enumerate__empty (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_seq_enumerate_t, void *))&_DeeNone_rets0_3)
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_enumerate__with__seq_operator_size__and__operator_getitem_index_fast(DeeObject *__restrict self, Dee_seq_enumerate_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_enumerate__with__seq_operator_size__and__seq_operator_getitem_index(DeeObject *__restrict self, Dee_seq_enumerate_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_enumerate__with__seq_operator_sizeob__and__seq_operator_getitem(DeeObject *__restrict self, Dee_seq_enumerate_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_enumerate__with__counter__and__seq_operator_foreach(DeeObject *__restrict self, Dee_seq_enumerate_t proc, void *arg);

/* seq_enumerate_index */
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_enumerate_index__with_callattr___seq_enumerate__(DeeObject *__restrict self, Dee_seq_enumerate_index_t proc, void *arg, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_enumerate_index__with_callobjectcache___seq_enumerate__(DeeObject *__restrict self, Dee_seq_enumerate_index_t proc, void *arg, size_t start, size_t end);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_enumerate_index__with_callmethodcache___seq_enumerate__(DeeObject *__restrict self, Dee_seq_enumerate_index_t proc, void *arg, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_enumerate_index__with_callkwmethodcache___seq_enumerate__(DeeObject *__restrict self, Dee_seq_enumerate_index_t proc, void *arg, size_t start, size_t end);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_enumerate_index__unsupported(DeeObject *__restrict self, Dee_seq_enumerate_index_t proc, void *arg, size_t start, size_t end);
#define default__seq_enumerate_index__empty (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_seq_enumerate_index_t, void *, size_t, size_t))&_DeeNone_rets0_5)
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_enumerate_index__with__seq_operator_size__and__operator_getitem_index_fast(DeeObject *__restrict self, Dee_seq_enumerate_index_t proc, void *arg, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_enumerate_index__with__seq_operator_size__and__seq_operator_getitem_index(DeeObject *__restrict self, Dee_seq_enumerate_index_t proc, void *arg, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_enumerate_index__with__counter__and__seq_operator_foreach(DeeObject *__restrict self, Dee_seq_enumerate_index_t proc, void *arg, size_t start, size_t end);

/* seq_makeenumeration */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_makeenumeration__with_callattr___seq_enumerate_items__(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_makeenumeration__with_callobjectcache___seq_enumerate_items__(DeeObject *self);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_makeenumeration__with_callmethodcache___seq_enumerate_items__(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_makeenumeration__with_callkwmethodcache___seq_enumerate_items__(DeeObject *self);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_makeenumeration__unsupported(DeeObject *self);

/* seq_makeenumeration_with_int_range */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_makeenumeration_with_int_range__with_callattr___seq_enumerate_items__(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_makeenumeration_with_int_range__with_callobjectcache___seq_enumerate_items__(DeeObject *self, size_t start, size_t end);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_makeenumeration_with_int_range__with_callmethodcache___seq_enumerate_items__(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_makeenumeration_with_int_range__with_callkwmethodcache___seq_enumerate_items__(DeeObject *self, size_t start, size_t end);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_makeenumeration_with_int_range__unsupported(DeeObject *self, size_t start, size_t end);

/* seq_makeenumeration_with_range */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_makeenumeration_with_range__with_callattr___seq_enumerate_items__(DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_makeenumeration_with_range__with_callobjectcache___seq_enumerate_items__(DeeObject *self, DeeObject *start, DeeObject *end);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_makeenumeration_with_range__with_callmethodcache___seq_enumerate_items__(DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_makeenumeration_with_range__with_callkwmethodcache___seq_enumerate_items__(DeeObject *self, DeeObject *start, DeeObject *end);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_makeenumeration_with_range__unsupported(DeeObject *self, DeeObject *start, DeeObject *end);

/* seq_foreach_reverse */
#define default__seq_foreach_reverse__empty (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&_DeeNone_rets0_3)
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_foreach_reverse__with__seq_operator_size__and__getitem_index_fast(DeeObject *__restrict self, Dee_foreach_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_foreach_reverse__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *__restrict self, Dee_foreach_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_foreach_reverse__with__seq_operator_size__and__seq_operator_getitem_index(DeeObject *__restrict self, Dee_foreach_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_foreach_reverse__with__seq_operator_sizeob__and__seq_operator_getitem(DeeObject *__restrict self, Dee_foreach_t cb, void *arg);

/* seq_enumerate_index_reverse */
#define default__seq_enumerate_index_reverse__empty (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_seq_enumerate_index_t, void *, size_t, size_t))&_DeeNone_rets0_5)
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_enumerate_index_reverse__with__seq_operator_size__and__getitem_index_fast(DeeObject *__restrict self, Dee_seq_enumerate_index_t cb, void *arg, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_enumerate_index_reverse__with__seq_operator_size__and__seq_operator_trygetitem_index(DeeObject *__restrict self, Dee_seq_enumerate_index_t cb, void *arg, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_enumerate_index_reverse__with__seq_operator_size__and__seq_operator_getitem_index(DeeObject *__restrict self, Dee_seq_enumerate_index_t cb, void *arg, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_enumerate_index_reverse__with__seq_operator_sizeob__and__seq_operator_getitem(DeeObject *__restrict self, Dee_seq_enumerate_index_t cb, void *arg, size_t start, size_t end);

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
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_any_with_range__with_callattr_any(DeeObject *__restrict self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_any_with_range__with_callattr___seq_any__(DeeObject *__restrict self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_any_with_range__with_callobjectcache___seq_any__(DeeObject *__restrict self, size_t start, size_t end);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_any_with_range__with_callmethodcache___seq_any__(DeeObject *__restrict self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_any_with_range__with_callkwmethodcache___seq_any__(DeeObject *__restrict self, size_t start, size_t end);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_any_with_range__unsupported(DeeObject *__restrict self, size_t start, size_t end);
#define default__seq_any_with_range__empty (*(int (DCALL *)(DeeObject *__restrict, size_t, size_t))&_DeeNone_reti0_3)
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_any_with_range__with__seq_enumerate_index(DeeObject *__restrict self, size_t start, size_t end);

/* seq_any_with_range_and_key */
INTDEF WUNUSED NONNULL((1, 4)) int DCALL default__seq_any_with_range_and_key__with_callattr_any(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL default__seq_any_with_range_and_key__with_callattr___seq_any__(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL default__seq_any_with_range_and_key__with_callobjectcache___seq_any__(DeeObject *self, size_t start, size_t end, DeeObject *key);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 4)) int DCALL default__seq_any_with_range_and_key__with_callmethodcache___seq_any__(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL default__seq_any_with_range_and_key__with_callkwmethodcache___seq_any__(DeeObject *self, size_t start, size_t end, DeeObject *key);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 4)) int DCALL default__seq_any_with_range_and_key__unsupported(DeeObject *self, size_t start, size_t end, DeeObject *key);
#define default__seq_any_with_range_and_key__empty (*(int (DCALL *)(DeeObject *, size_t, size_t, DeeObject *))&_DeeNone_reti0_4)
INTDEF WUNUSED NONNULL((1, 4)) int DCALL default__seq_any_with_range_and_key__with__seq_enumerate_index(DeeObject *self, size_t start, size_t end, DeeObject *key);

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
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_all_with_range__with_callattr_all(DeeObject *__restrict self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_all_with_range__with_callattr___seq_all__(DeeObject *__restrict self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_all_with_range__with_callobjectcache___seq_all__(DeeObject *__restrict self, size_t start, size_t end);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_all_with_range__with_callmethodcache___seq_all__(DeeObject *__restrict self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_all_with_range__with_callkwmethodcache___seq_all__(DeeObject *__restrict self, size_t start, size_t end);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_all_with_range__unsupported(DeeObject *__restrict self, size_t start, size_t end);
#define default__seq_all_with_range__empty (*(int (DCALL *)(DeeObject *__restrict, size_t, size_t))&_DeeNone_reti0_3)
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_all_with_range__with__seq_enumerate_index(DeeObject *__restrict self, size_t start, size_t end);

/* seq_all_with_range_and_key */
INTDEF WUNUSED NONNULL((1, 4)) int DCALL default__seq_all_with_range_and_key__with_callattr_all(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL default__seq_all_with_range_and_key__with_callattr___seq_all__(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL default__seq_all_with_range_and_key__with_callobjectcache___seq_all__(DeeObject *self, size_t start, size_t end, DeeObject *key);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 4)) int DCALL default__seq_all_with_range_and_key__with_callmethodcache___seq_all__(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL default__seq_all_with_range_and_key__with_callkwmethodcache___seq_all__(DeeObject *self, size_t start, size_t end, DeeObject *key);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 4)) int DCALL default__seq_all_with_range_and_key__unsupported(DeeObject *self, size_t start, size_t end, DeeObject *key);
#define default__seq_all_with_range_and_key__empty (*(int (DCALL *)(DeeObject *, size_t, size_t, DeeObject *))&_DeeNone_reti0_4)
INTDEF WUNUSED NONNULL((1, 4)) int DCALL default__seq_all_with_range_and_key__with__seq_enumerate_index(DeeObject *self, size_t start, size_t end, DeeObject *key);

/* seq_parity */
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_parity__with_callattr_parity(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_parity__with_callattr___seq_parity__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_parity__with_callobjectcache___seq_parity__(DeeObject *__restrict self);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_parity__with_callmethodcache___seq_parity__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_parity__with_callkwmethodcache___seq_parity__(DeeObject *__restrict self);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_parity__unsupported(DeeObject *__restrict self);
#define default__seq_parity__empty (*(int (DCALL *)(DeeObject *__restrict))&_DeeNone_reti0_1)
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_parity__with__seq_count(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_parity__with__seq_operator_foreach(DeeObject *__restrict self);

/* seq_parity_with_key */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_parity_with_key__with_callattr_parity(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_parity_with_key__with_callattr___seq_parity__(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_parity_with_key__with_callobjectcache___seq_parity__(DeeObject *self, DeeObject *key);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_parity_with_key__with_callmethodcache___seq_parity__(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_parity_with_key__with_callkwmethodcache___seq_parity__(DeeObject *self, DeeObject *key);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_parity_with_key__unsupported(DeeObject *self, DeeObject *key);
#define default__seq_parity_with_key__empty (*(int (DCALL *)(DeeObject *, DeeObject *))&_DeeNone_reti0_2)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_parity_with_key__with__seq_operator_foreach(DeeObject *self, DeeObject *key);

/* seq_parity_with_range */
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_parity_with_range__with_callattr_parity(DeeObject *__restrict self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_parity_with_range__with_callattr___seq_parity__(DeeObject *__restrict self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_parity_with_range__with_callobjectcache___seq_parity__(DeeObject *__restrict self, size_t start, size_t end);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_parity_with_range__with_callmethodcache___seq_parity__(DeeObject *__restrict self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_parity_with_range__with_callkwmethodcache___seq_parity__(DeeObject *__restrict self, size_t start, size_t end);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_parity_with_range__unsupported(DeeObject *__restrict self, size_t start, size_t end);
#define default__seq_parity_with_range__empty (*(int (DCALL *)(DeeObject *__restrict, size_t, size_t))&_DeeNone_reti0_3)
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_parity_with_range__with__seq_count_with_range(DeeObject *__restrict self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_parity_with_range__with__seq_enumerate_index(DeeObject *__restrict self, size_t start, size_t end);

/* seq_parity_with_range_and_key */
INTDEF WUNUSED NONNULL((1, 4)) int DCALL default__seq_parity_with_range_and_key__with_callattr_parity(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL default__seq_parity_with_range_and_key__with_callattr___seq_parity__(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL default__seq_parity_with_range_and_key__with_callobjectcache___seq_parity__(DeeObject *self, size_t start, size_t end, DeeObject *key);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 4)) int DCALL default__seq_parity_with_range_and_key__with_callmethodcache___seq_parity__(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL default__seq_parity_with_range_and_key__with_callkwmethodcache___seq_parity__(DeeObject *self, size_t start, size_t end, DeeObject *key);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 4)) int DCALL default__seq_parity_with_range_and_key__unsupported(DeeObject *self, size_t start, size_t end, DeeObject *key);
#define default__seq_parity_with_range_and_key__empty (*(int (DCALL *)(DeeObject *, size_t, size_t, DeeObject *))&_DeeNone_reti0_4)
INTDEF WUNUSED NONNULL((1, 4)) int DCALL default__seq_parity_with_range_and_key__with__seq_enumerate_index(DeeObject *self, size_t start, size_t end, DeeObject *key);

/* seq_min */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_min__with_callattr_min(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_min__with_callattr___seq_min__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_min__with_callobjectcache___seq_min__(DeeObject *__restrict self);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_min__with_callmethodcache___seq_min__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_min__with_callkwmethodcache___seq_min__(DeeObject *__restrict self);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_min__unsupported(DeeObject *__restrict self);
#define default__seq_min__empty (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&_DeeNone_rets0_1)
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_min__with__seq_operator_foreach(DeeObject *__restrict self);

/* seq_min_with_key */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_min_with_key__with_callattr_min(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_min_with_key__with_callattr___seq_min__(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_min_with_key__with_callobjectcache___seq_min__(DeeObject *self, DeeObject *key);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_min_with_key__with_callmethodcache___seq_min__(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_min_with_key__with_callkwmethodcache___seq_min__(DeeObject *self, DeeObject *key);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_min_with_key__unsupported(DeeObject *self, DeeObject *key);
#define default__seq_min_with_key__empty (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&_DeeNone_rets0_2)
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_min_with_key__with__seq_operator_foreach(DeeObject *self, DeeObject *key);

/* seq_min_with_range */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_min_with_range__with_callattr_min(DeeObject *__restrict self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_min_with_range__with_callattr___seq_min__(DeeObject *__restrict self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_min_with_range__with_callobjectcache___seq_min__(DeeObject *__restrict self, size_t start, size_t end);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_min_with_range__with_callmethodcache___seq_min__(DeeObject *__restrict self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_min_with_range__with_callkwmethodcache___seq_min__(DeeObject *__restrict self, size_t start, size_t end);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_min_with_range__unsupported(DeeObject *__restrict self, size_t start, size_t end);
#define default__seq_min_with_range__empty (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict, size_t, size_t))&_DeeNone_rets0_3)
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_min_with_range__with__seq_enumerate_index(DeeObject *__restrict self, size_t start, size_t end);

/* seq_min_with_range_and_key */
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL default__seq_min_with_range_and_key__with_callattr_min(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL default__seq_min_with_range_and_key__with_callattr___seq_min__(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL default__seq_min_with_range_and_key__with_callobjectcache___seq_min__(DeeObject *self, size_t start, size_t end, DeeObject *key);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL default__seq_min_with_range_and_key__with_callmethodcache___seq_min__(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL default__seq_min_with_range_and_key__with_callkwmethodcache___seq_min__(DeeObject *self, size_t start, size_t end, DeeObject *key);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL default__seq_min_with_range_and_key__unsupported(DeeObject *self, size_t start, size_t end, DeeObject *key);
#define default__seq_min_with_range_and_key__empty (*(DREF DeeObject *(DCALL *)(DeeObject *, size_t, size_t, DeeObject *))&_DeeNone_rets0_4)
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL default__seq_min_with_range_and_key__with__seq_enumerate_index(DeeObject *self, size_t start, size_t end, DeeObject *key);

/* seq_max */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_max__with_callattr_max(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_max__with_callattr___seq_max__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_max__with_callobjectcache___seq_max__(DeeObject *__restrict self);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_max__with_callmethodcache___seq_max__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_max__with_callkwmethodcache___seq_max__(DeeObject *__restrict self);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_max__unsupported(DeeObject *__restrict self);
#define default__seq_max__empty (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&_DeeNone_rets0_1)
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_max__with__seq_operator_foreach(DeeObject *__restrict self);

/* seq_max_with_key */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_max_with_key__with_callattr_max(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_max_with_key__with_callattr___seq_max__(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_max_with_key__with_callobjectcache___seq_max__(DeeObject *self, DeeObject *key);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_max_with_key__with_callmethodcache___seq_max__(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_max_with_key__with_callkwmethodcache___seq_max__(DeeObject *self, DeeObject *key);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_max_with_key__unsupported(DeeObject *self, DeeObject *key);
#define default__seq_max_with_key__empty (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&_DeeNone_rets0_2)
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_max_with_key__with__seq_operator_foreach(DeeObject *self, DeeObject *key);

/* seq_max_with_range */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_max_with_range__with_callattr_max(DeeObject *__restrict self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_max_with_range__with_callattr___seq_max__(DeeObject *__restrict self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_max_with_range__with_callobjectcache___seq_max__(DeeObject *__restrict self, size_t start, size_t end);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_max_with_range__with_callmethodcache___seq_max__(DeeObject *__restrict self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_max_with_range__with_callkwmethodcache___seq_max__(DeeObject *__restrict self, size_t start, size_t end);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_max_with_range__unsupported(DeeObject *__restrict self, size_t start, size_t end);
#define default__seq_max_with_range__empty (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict, size_t, size_t))&_DeeNone_rets0_3)
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_max_with_range__with__seq_enumerate_index(DeeObject *__restrict self, size_t start, size_t end);

/* seq_max_with_range_and_key */
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL default__seq_max_with_range_and_key__with_callattr_max(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL default__seq_max_with_range_and_key__with_callattr___seq_max__(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL default__seq_max_with_range_and_key__with_callobjectcache___seq_max__(DeeObject *self, size_t start, size_t end, DeeObject *key);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL default__seq_max_with_range_and_key__with_callmethodcache___seq_max__(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL default__seq_max_with_range_and_key__with_callkwmethodcache___seq_max__(DeeObject *self, size_t start, size_t end, DeeObject *key);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL default__seq_max_with_range_and_key__unsupported(DeeObject *self, size_t start, size_t end, DeeObject *key);
#define default__seq_max_with_range_and_key__empty (*(DREF DeeObject *(DCALL *)(DeeObject *, size_t, size_t, DeeObject *))&_DeeNone_rets0_4)
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL default__seq_max_with_range_and_key__with__seq_enumerate_index(DeeObject *self, size_t start, size_t end, DeeObject *key);

/* seq_sum */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_sum__with_callattr_sum(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_sum__with_callattr___seq_sum__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_sum__with_callobjectcache___seq_sum__(DeeObject *__restrict self);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_sum__with_callmethodcache___seq_sum__(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_sum__with_callkwmethodcache___seq_sum__(DeeObject *__restrict self);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_sum__unsupported(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_sum__empty(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_sum__with__seq_operator_foreach(DeeObject *__restrict self);

/* seq_sum_with_range */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_sum_with_range__with_callattr_sum(DeeObject *__restrict self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_sum_with_range__with_callattr___seq_sum__(DeeObject *__restrict self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_sum_with_range__with_callobjectcache___seq_sum__(DeeObject *__restrict self, size_t start, size_t end);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_sum_with_range__with_callmethodcache___seq_sum__(DeeObject *__restrict self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_sum_with_range__with_callkwmethodcache___seq_sum__(DeeObject *__restrict self, size_t start, size_t end);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_sum_with_range__unsupported(DeeObject *__restrict self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_sum_with_range__empty(DeeObject *__restrict self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_sum_with_range__with__seq_enumerate_index(DeeObject *__restrict self, size_t start, size_t end);

/* seq_count */
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_count__with_callattr_count(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_count__with_callattr___seq_count__(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_count__with_callobjectcache___seq_count__(DeeObject *self, DeeObject *item);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_count__with_callmethodcache___seq_count__(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_count__with_callkwmethodcache___seq_count__(DeeObject *self, DeeObject *item);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_count__unsupported(DeeObject *self, DeeObject *item);
#define default__seq_count__empty (*(size_t (DCALL *)(DeeObject *, DeeObject *))&_DeeNone_rets0_2)
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_count__with__set_operator_contains(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_count__with__seq_operator_foreach(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_count__with__seq_find(DeeObject *self, DeeObject *item);

/* seq_count_with_key */
INTDEF WUNUSED NONNULL((1, 2, 3)) size_t DCALL default__seq_count_with_key__with_callattr_count(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) size_t DCALL default__seq_count_with_key__with_callattr___seq_count__(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) size_t DCALL default__seq_count_with_key__with_callobjectcache___seq_count__(DeeObject *self, DeeObject *item, DeeObject *key);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2, 3)) size_t DCALL default__seq_count_with_key__with_callmethodcache___seq_count__(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) size_t DCALL default__seq_count_with_key__with_callkwmethodcache___seq_count__(DeeObject *self, DeeObject *item, DeeObject *key);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2, 3)) size_t DCALL default__seq_count_with_key__unsupported(DeeObject *self, DeeObject *item, DeeObject *key);
#define default__seq_count_with_key__empty (*(size_t (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&_DeeNone_rets0_3)
INTDEF WUNUSED NONNULL((1, 2, 3)) size_t DCALL default__seq_count_with_key__with__seq_operator_foreach(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) size_t DCALL default__seq_count_with_key__with__seq_find_with_key(DeeObject *self, DeeObject *item, DeeObject *key);

/* seq_count_with_range */
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_count_with_range__with_callattr_count(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_count_with_range__with_callattr___seq_count__(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_count_with_range__with_callobjectcache___seq_count__(DeeObject *self, DeeObject *item, size_t start, size_t end);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_count_with_range__with_callmethodcache___seq_count__(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_count_with_range__with_callkwmethodcache___seq_count__(DeeObject *self, DeeObject *item, size_t start, size_t end);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_count_with_range__unsupported(DeeObject *self, DeeObject *item, size_t start, size_t end);
#define default__seq_count_with_range__empty (*(size_t (DCALL *)(DeeObject *, DeeObject *, size_t, size_t))&_DeeNone_rets0_4)
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_count_with_range__with__seq_enumerate_index(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_count_with_range__with__seq_find(DeeObject *self, DeeObject *item, size_t start, size_t end);

/* seq_count_with_range_and_key */
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL default__seq_count_with_range_and_key__with_callattr_count(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL default__seq_count_with_range_and_key__with_callattr___seq_count__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL default__seq_count_with_range_and_key__with_callobjectcache___seq_count__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL default__seq_count_with_range_and_key__with_callmethodcache___seq_count__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL default__seq_count_with_range_and_key__with_callkwmethodcache___seq_count__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL default__seq_count_with_range_and_key__unsupported(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
#define default__seq_count_with_range_and_key__empty (*(size_t (DCALL *)(DeeObject *, DeeObject *, size_t, size_t, DeeObject *))&_DeeNone_rets0_5)
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL default__seq_count_with_range_and_key__with__seq_enumerate_index(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL default__seq_count_with_range_and_key__with__seq_find_with_key(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);

/* seq_contains */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_contains__with_callattr___seq_contains__(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_contains__with_callobjectcache___seq_contains__(DeeObject *self, DeeObject *item);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_contains__with_callmethodcache___seq_contains__(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_contains__with_callkwmethodcache___seq_contains__(DeeObject *self, DeeObject *item);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_contains__unsupported(DeeObject *self, DeeObject *item);
#define default__seq_contains__empty (*(int (DCALL *)(DeeObject *, DeeObject *))&_DeeNone_reti0_2)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_contains__with__seq_operator_contains(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_contains__with__map_operator_trygetitem(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_contains__with__seq_operator_foreach(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_contains__with__seq_find(DeeObject *self, DeeObject *item);

/* seq_contains_with_key */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_contains_with_key__with_callattr___seq_contains__(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_contains_with_key__with_callobjectcache___seq_contains__(DeeObject *self, DeeObject *item, DeeObject *key);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_contains_with_key__with_callmethodcache___seq_contains__(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_contains_with_key__with_callkwmethodcache___seq_contains__(DeeObject *self, DeeObject *item, DeeObject *key);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_contains_with_key__unsupported(DeeObject *self, DeeObject *item, DeeObject *key);
#define default__seq_contains_with_key__empty (*(int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&_DeeNone_reti0_3)
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_contains_with_key__with__seq_operator_foreach(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_contains_with_key__with__seq_find_with_key(DeeObject *self, DeeObject *item, DeeObject *key);

/* seq_contains_with_range */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_contains_with_range__with_callattr___seq_contains__(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_contains_with_range__with_callobjectcache___seq_contains__(DeeObject *self, DeeObject *item, size_t start, size_t end);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_contains_with_range__with_callmethodcache___seq_contains__(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_contains_with_range__with_callkwmethodcache___seq_contains__(DeeObject *self, DeeObject *item, size_t start, size_t end);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_contains_with_range__unsupported(DeeObject *self, DeeObject *item, size_t start, size_t end);
#define default__seq_contains_with_range__empty (*(int (DCALL *)(DeeObject *, DeeObject *, size_t, size_t))&_DeeNone_reti0_4)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_contains_with_range__with__seq_enumerate_index(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_contains_with_range__with__seq_find(DeeObject *self, DeeObject *item, size_t start, size_t end);

/* seq_contains_with_range_and_key */
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL default__seq_contains_with_range_and_key__with_callattr___seq_contains__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL default__seq_contains_with_range_and_key__with_callobjectcache___seq_contains__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL default__seq_contains_with_range_and_key__with_callmethodcache___seq_contains__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL default__seq_contains_with_range_and_key__with_callkwmethodcache___seq_contains__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL default__seq_contains_with_range_and_key__unsupported(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
#define default__seq_contains_with_range_and_key__empty (*(int (DCALL *)(DeeObject *, DeeObject *, size_t, size_t, DeeObject *))&_DeeNone_reti0_5)
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL default__seq_contains_with_range_and_key__with__seq_enumerate_index(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL default__seq_contains_with_range_and_key__with__seq_find_with_key(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);

/* seq_operator_contains */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_contains(DeeObject *self, DeeObject *item);
#define default__seq_operator_contains__with_callattr___seq_contains__ (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&default__seq_operator_contains__with__seq_contains)
#define default__seq_operator_contains__with_callobjectcache___seq_contains__ (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&default__seq_operator_contains__with__seq_contains)
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
#define default__seq_operator_contains__with_callmethodcache___seq_contains__ (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&default__seq_operator_contains__with__seq_contains)
#define default__seq_operator_contains__with_callkwmethodcache___seq_contains__ (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&default__seq_operator_contains__with__seq_contains)
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_contains__unsupported(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_contains__empty(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__seq_operator_contains__with__seq_contains(DeeObject *self, DeeObject *item);

/* seq_locate */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_locate__with_callattr_locate(DeeObject *self, DeeObject *match, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_locate__with_callattr___seq_locate__(DeeObject *self, DeeObject *match, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_locate__with_callobjectcache___seq_locate__(DeeObject *self, DeeObject *match, DeeObject *def);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_locate__with_callmethodcache___seq_locate__(DeeObject *self, DeeObject *match, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_locate__with_callkwmethodcache___seq_locate__(DeeObject *self, DeeObject *match, DeeObject *def);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_locate__unsupported(DeeObject *self, DeeObject *match, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_locate__empty(DeeObject *self, DeeObject *match, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_locate__with__seq_operator_foreach(DeeObject *self, DeeObject *match, DeeObject *def);

/* seq_locate_with_range */
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL default__seq_locate_with_range__with_callattr_locate(DeeObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL default__seq_locate_with_range__with_callattr___seq_locate__(DeeObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL default__seq_locate_with_range__with_callobjectcache___seq_locate__(DeeObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL default__seq_locate_with_range__with_callmethodcache___seq_locate__(DeeObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL default__seq_locate_with_range__with_callkwmethodcache___seq_locate__(DeeObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL default__seq_locate_with_range__unsupported(DeeObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL default__seq_locate_with_range__empty(DeeObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL default__seq_locate_with_range__with__seq_enumerate_index(DeeObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def);

/* seq_rlocate */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_rlocate__with_callattr_rlocate(DeeObject *self, DeeObject *match, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_rlocate__with_callattr___seq_rlocate__(DeeObject *self, DeeObject *match, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_rlocate__with_callobjectcache___seq_rlocate__(DeeObject *self, DeeObject *match, DeeObject *def);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_rlocate__with_callmethodcache___seq_rlocate__(DeeObject *self, DeeObject *match, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_rlocate__with_callkwmethodcache___seq_rlocate__(DeeObject *self, DeeObject *match, DeeObject *def);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_rlocate__unsupported(DeeObject *self, DeeObject *match, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_rlocate__empty(DeeObject *self, DeeObject *match, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_rlocate__with__seq_foreach_reverse(DeeObject *self, DeeObject *match, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__seq_rlocate__with__seq_operator_foreach(DeeObject *self, DeeObject *match, DeeObject *def);

/* seq_rlocate_with_range */
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL default__seq_rlocate_with_range__with_callattr_rlocate(DeeObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL default__seq_rlocate_with_range__with_callattr___seq_rlocate__(DeeObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL default__seq_rlocate_with_range__with_callobjectcache___seq_rlocate__(DeeObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL default__seq_rlocate_with_range__with_callmethodcache___seq_rlocate__(DeeObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL default__seq_rlocate_with_range__with_callkwmethodcache___seq_rlocate__(DeeObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL default__seq_rlocate_with_range__unsupported(DeeObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL default__seq_rlocate_with_range__empty(DeeObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL default__seq_rlocate_with_range__with__seq_enumerate_index_reverse(DeeObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL default__seq_rlocate_with_range__with__seq_enumerate_index(DeeObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def);

/* seq_startswith */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_startswith__with_callattr_startswith(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_startswith__with_callattr___seq_startswith__(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_startswith__with_callobjectcache___seq_startswith__(DeeObject *self, DeeObject *item);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_startswith__with_callmethodcache___seq_startswith__(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_startswith__with_callkwmethodcache___seq_startswith__(DeeObject *self, DeeObject *item);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_startswith__unsupported(DeeObject *self, DeeObject *item);
#define default__seq_startswith__empty (*(int (DCALL *)(DeeObject *, DeeObject *))&_DeeNone_reti0_2)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_startswith__with__seq_trygetfirst(DeeObject *self, DeeObject *item);

/* seq_startswith_with_key */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_startswith_with_key__with_callattr_startswith(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_startswith_with_key__with_callattr___seq_startswith__(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_startswith_with_key__with_callobjectcache___seq_startswith__(DeeObject *self, DeeObject *item, DeeObject *key);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_startswith_with_key__with_callmethodcache___seq_startswith__(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_startswith_with_key__with_callkwmethodcache___seq_startswith__(DeeObject *self, DeeObject *item, DeeObject *key);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_startswith_with_key__unsupported(DeeObject *self, DeeObject *item, DeeObject *key);
#define default__seq_startswith_with_key__empty (*(int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&_DeeNone_reti0_3)
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_startswith_with_key__with__seq_trygetfirst(DeeObject *self, DeeObject *item, DeeObject *key);

/* seq_startswith_with_range */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_startswith_with_range__with_callattr_startswith(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_startswith_with_range__with_callattr___seq_startswith__(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_startswith_with_range__with_callobjectcache___seq_startswith__(DeeObject *self, DeeObject *item, size_t start, size_t end);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_startswith_with_range__with_callmethodcache___seq_startswith__(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_startswith_with_range__with_callkwmethodcache___seq_startswith__(DeeObject *self, DeeObject *item, size_t start, size_t end);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_startswith_with_range__unsupported(DeeObject *self, DeeObject *item, size_t start, size_t end);
#define default__seq_startswith_with_range__empty (*(int (DCALL *)(DeeObject *, DeeObject *, size_t, size_t))&_DeeNone_reti0_4)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_startswith_with_range__with__seq_operator_trygetitem_index(DeeObject *self, DeeObject *item, size_t start, size_t end);

/* seq_startswith_with_range_and_key */
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL default__seq_startswith_with_range_and_key__with_callattr_startswith(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL default__seq_startswith_with_range_and_key__with_callattr___seq_startswith__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL default__seq_startswith_with_range_and_key__with_callobjectcache___seq_startswith__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL default__seq_startswith_with_range_and_key__with_callmethodcache___seq_startswith__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL default__seq_startswith_with_range_and_key__with_callkwmethodcache___seq_startswith__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL default__seq_startswith_with_range_and_key__unsupported(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
#define default__seq_startswith_with_range_and_key__empty (*(int (DCALL *)(DeeObject *, DeeObject *, size_t, size_t, DeeObject *))&_DeeNone_reti0_5)
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL default__seq_startswith_with_range_and_key__with__seq_operator_trygetitem_index(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);

/* seq_endswith */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_endswith__with_callattr_endswith(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_endswith__with_callattr___seq_endswith__(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_endswith__with_callobjectcache___seq_endswith__(DeeObject *self, DeeObject *item);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_endswith__with_callmethodcache___seq_endswith__(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_endswith__with_callkwmethodcache___seq_endswith__(DeeObject *self, DeeObject *item);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_endswith__unsupported(DeeObject *self, DeeObject *item);
#define default__seq_endswith__empty (*(int (DCALL *)(DeeObject *, DeeObject *))&_DeeNone_reti0_2)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_endswith__with__seq_trygetlast(DeeObject *self, DeeObject *item);

/* seq_endswith_with_key */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_endswith_with_key__with_callattr_endswith(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_endswith_with_key__with_callattr___seq_endswith__(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_endswith_with_key__with_callobjectcache___seq_endswith__(DeeObject *self, DeeObject *item, DeeObject *key);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_endswith_with_key__with_callmethodcache___seq_endswith__(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_endswith_with_key__with_callkwmethodcache___seq_endswith__(DeeObject *self, DeeObject *item, DeeObject *key);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_endswith_with_key__unsupported(DeeObject *self, DeeObject *item, DeeObject *key);
#define default__seq_endswith_with_key__empty (*(int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&_DeeNone_reti0_3)
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_endswith_with_key__with__seq_trygetlast(DeeObject *self, DeeObject *item, DeeObject *key);

/* seq_endswith_with_range */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_endswith_with_range__with_callattr_endswith(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_endswith_with_range__with_callattr___seq_endswith__(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_endswith_with_range__with_callobjectcache___seq_endswith__(DeeObject *self, DeeObject *item, size_t start, size_t end);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_endswith_with_range__with_callmethodcache___seq_endswith__(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_endswith_with_range__with_callkwmethodcache___seq_endswith__(DeeObject *self, DeeObject *item, size_t start, size_t end);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_endswith_with_range__unsupported(DeeObject *self, DeeObject *item, size_t start, size_t end);
#define default__seq_endswith_with_range__empty (*(int (DCALL *)(DeeObject *, DeeObject *, size_t, size_t))&_DeeNone_reti0_4)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_endswith_with_range__with__seq_operator_size__and__operator_trygetitem_index(DeeObject *self, DeeObject *item, size_t start, size_t end);

/* seq_endswith_with_range_and_key */
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL default__seq_endswith_with_range_and_key__with_callattr_endswith(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL default__seq_endswith_with_range_and_key__with_callattr___seq_endswith__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL default__seq_endswith_with_range_and_key__with_callobjectcache___seq_endswith__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL default__seq_endswith_with_range_and_key__with_callmethodcache___seq_endswith__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL default__seq_endswith_with_range_and_key__with_callkwmethodcache___seq_endswith__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL default__seq_endswith_with_range_and_key__unsupported(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
#define default__seq_endswith_with_range_and_key__empty (*(int (DCALL *)(DeeObject *, DeeObject *, size_t, size_t, DeeObject *))&_DeeNone_reti0_5)
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL default__seq_endswith_with_range_and_key__with__seq_operator_size__and__operator_trygetitem_index(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);

/* seq_find */
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_find__with_callattr_find(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_find__with_callattr___seq_find__(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_find__with_callobjectcache___seq_find__(DeeObject *self, DeeObject *item, size_t start, size_t end);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_find__with_callmethodcache___seq_find__(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_find__with_callkwmethodcache___seq_find__(DeeObject *self, DeeObject *item, size_t start, size_t end);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_find__unsupported(DeeObject *self, DeeObject *item, size_t start, size_t end);
#define default__seq_find__empty (*(size_t (DCALL *)(DeeObject *, DeeObject *, size_t, size_t))&_DeeNone_rets0_4)
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_find__with__seq_enumerate_index(DeeObject *self, DeeObject *item, size_t start, size_t end);

/* seq_find_with_key */
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL default__seq_find_with_key__with_callattr_find(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL default__seq_find_with_key__with_callattr___seq_find__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL default__seq_find_with_key__with_callobjectcache___seq_find__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL default__seq_find_with_key__with_callmethodcache___seq_find__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL default__seq_find_with_key__with_callkwmethodcache___seq_find__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL default__seq_find_with_key__unsupported(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
#define default__seq_find_with_key__empty (*(size_t (DCALL *)(DeeObject *, DeeObject *, size_t, size_t, DeeObject *))&_DeeNone_rets0_5)
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL default__seq_find_with_key__with__seq_enumerate_index(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);

/* seq_rfind */
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_rfind__with_callattr_rfind(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_rfind__with_callattr___seq_rfind__(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_rfind__with_callobjectcache___seq_rfind__(DeeObject *self, DeeObject *item, size_t start, size_t end);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_rfind__with_callmethodcache___seq_rfind__(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_rfind__with_callkwmethodcache___seq_rfind__(DeeObject *self, DeeObject *item, size_t start, size_t end);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_rfind__unsupported(DeeObject *self, DeeObject *item, size_t start, size_t end);
#define default__seq_rfind__empty (*(size_t (DCALL *)(DeeObject *, DeeObject *, size_t, size_t))&_DeeNone_rets0_4)
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_rfind__with__seq_enumerate_index_reverse(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL default__seq_rfind__with__seq_enumerate_index(DeeObject *self, DeeObject *item, size_t start, size_t end);

/* seq_rfind_with_key */
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL default__seq_rfind_with_key__with_callattr_rfind(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL default__seq_rfind_with_key__with_callattr___seq_rfind__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL default__seq_rfind_with_key__with_callobjectcache___seq_rfind__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL default__seq_rfind_with_key__with_callmethodcache___seq_rfind__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL default__seq_rfind_with_key__with_callkwmethodcache___seq_rfind__(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL default__seq_rfind_with_key__unsupported(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
#define default__seq_rfind_with_key__empty (*(size_t (DCALL *)(DeeObject *, DeeObject *, size_t, size_t, DeeObject *))&_DeeNone_rets0_5)
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL default__seq_rfind_with_key__with__seq_enumerate_index_reverse(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL default__seq_rfind_with_key__with__seq_enumerate_index(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);

/* seq_erase */
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_erase__with_callattr_erase(DeeObject *__restrict self, size_t index, size_t count);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_erase__with_callattr___seq_erase__(DeeObject *__restrict self, size_t index, size_t count);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_erase__with_callobjectcache___seq_erase__(DeeObject *__restrict self, size_t index, size_t count);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_erase__with_callmethodcache___seq_erase__(DeeObject *__restrict self, size_t index, size_t count);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_erase__with_callkwmethodcache___seq_erase__(DeeObject *__restrict self, size_t index, size_t count);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_erase__unsupported(DeeObject *__restrict self, size_t index, size_t count);
#define default__seq_erase__empty (*(int (DCALL *)(DeeObject *__restrict, size_t, size_t))&_DeeNone_reti0_3)
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_erase__with__seq_operator_delrange_index(DeeObject *__restrict self, size_t index, size_t count);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_erase__with__seq_pop(DeeObject *__restrict self, size_t index, size_t count);

/* seq_insert */
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_insert__with_callattr_insert(DeeObject *self, size_t index, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_insert__with_callattr___seq_insert__(DeeObject *self, size_t index, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_insert__with_callobjectcache___seq_insert__(DeeObject *self, size_t index, DeeObject *item);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_insert__with_callmethodcache___seq_insert__(DeeObject *self, size_t index, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_insert__with_callkwmethodcache___seq_insert__(DeeObject *self, size_t index, DeeObject *item);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_insert__unsupported(DeeObject *self, size_t index, DeeObject *item);
#define default__seq_insert__empty (*(int (DCALL *)(DeeObject *, size_t, DeeObject *))&default__seq_insert__unsupported)
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_insert__with__seq_insertall(DeeObject *self, size_t index, DeeObject *item);

/* seq_insertall */
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_insertall__with_callattr_insertall(DeeObject *self, size_t index, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_insertall__with_callattr___seq_insertall__(DeeObject *self, size_t index, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_insertall__with_callobjectcache___seq_insertall__(DeeObject *self, size_t index, DeeObject *items);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_insertall__with_callmethodcache___seq_insertall__(DeeObject *self, size_t index, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_insertall__with_callkwmethodcache___seq_insertall__(DeeObject *self, size_t index, DeeObject *items);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_insertall__unsupported(DeeObject *self, size_t index, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_insertall__empty(DeeObject *self, size_t index, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_insertall__with__seq_operator_setrange_index(DeeObject *self, size_t index, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__seq_insertall__with__seq_insert(DeeObject *self, size_t index, DeeObject *items);

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
INTDEF WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL default__seq_xchitem_index__with_callattr_xchitem(DeeObject *self, size_t index, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL default__seq_xchitem_index__with_callattr___seq_xchitem__(DeeObject *self, size_t index, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL default__seq_xchitem_index__with_callobjectcache___seq_xchitem__(DeeObject *self, size_t index, DeeObject *item);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL default__seq_xchitem_index__with_callmethodcache___seq_xchitem__(DeeObject *self, size_t index, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL default__seq_xchitem_index__with_callkwmethodcache___seq_xchitem__(DeeObject *self, size_t index, DeeObject *item);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL default__seq_xchitem_index__unsupported(DeeObject *self, size_t index, DeeObject *item);
#define default__seq_xchitem_index__empty (*(DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *))&default__seq_xchitem_index__unsupported)
INTDEF WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL default__seq_xchitem_index__with__seq_operator_getitem_index__and__seq_operator_setitem_index(DeeObject *self, size_t index, DeeObject *item);

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
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_pop__with_callattr_pop(DeeObject *self, Dee_ssize_t index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_pop__with_callattr___seq_pop__(DeeObject *self, Dee_ssize_t index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_pop__with_callobjectcache___seq_pop__(DeeObject *self, Dee_ssize_t index);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_pop__with_callmethodcache___seq_pop__(DeeObject *self, Dee_ssize_t index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_pop__with_callkwmethodcache___seq_pop__(DeeObject *self, Dee_ssize_t index);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_pop__unsupported(DeeObject *self, Dee_ssize_t index);
#define default__seq_pop__empty (*(DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t))&default__seq_pop__unsupported)
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_pop__with__seq_operator_size__and__seq_operator_getitem_index__and__seq_erase(DeeObject *self, Dee_ssize_t index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_pop__with__seq_operator_size__and__seq_operator_getitem_index__and__seq_operator_delitem_index(DeeObject *self, Dee_ssize_t index);

/* set_operator_iter */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__set_operator_iter(DeeObject *__restrict self);
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
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__set_operator_foreach(DeeObject *__restrict self, Dee_foreach_t cb, void *arg);
#define default__set_operator_foreach__with_callattr___set_iter__ (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&default__set_operator_foreach__with__set_operator_iter)
#define default__set_operator_foreach__with_callobjectcache___set_iter__ (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&default__set_operator_foreach__with__set_operator_iter)
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
#define default__set_operator_foreach__with_callmethodcache___set_iter__ (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&default__set_operator_foreach__with__set_operator_iter)
#define default__set_operator_foreach__with_callkwmethodcache___set_iter__ (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&default__set_operator_foreach__with__set_operator_iter)
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
#define default__set_operator_foreach__unsupported (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&default__set_operator_foreach__with__set_operator_iter)
#define default__set_operator_foreach__empty (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&_DeeNone_rets0_3)
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__set_operator_foreach__with__seq_operator_foreach(DeeObject *__restrict self, Dee_foreach_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__set_operator_foreach__with__set_operator_iter(DeeObject *__restrict self, Dee_foreach_t cb, void *arg);

/* set_operator_foreach_pair */
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__set_operator_foreach_pair(DeeObject *__restrict self, Dee_foreach_pair_t cb, void *arg);
#define default__set_operator_foreach_pair__with_callattr___set_iter__ (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&default__set_operator_foreach_pair__with__set_operator_foreach)
#define default__set_operator_foreach_pair__with_callobjectcache___set_iter__ (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&default__set_operator_foreach_pair__with__set_operator_foreach)
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
#define default__set_operator_foreach_pair__with_callmethodcache___set_iter__ (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&default__set_operator_foreach_pair__with__set_operator_foreach)
#define default__set_operator_foreach_pair__with_callkwmethodcache___set_iter__ (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&default__set_operator_foreach_pair__with__set_operator_foreach)
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
#define default__set_operator_foreach_pair__unsupported (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&default__set_operator_foreach_pair__with__set_operator_foreach)
#define default__set_operator_foreach_pair__empty (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&_DeeNone_rets0_3)
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__set_operator_foreach_pair__with__set_operator_foreach(DeeObject *__restrict self, Dee_foreach_pair_t cb, void *arg);

/* set_operator_sizeob */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__set_operator_sizeob(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__set_operator_sizeob__with_callattr___set_size__(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__set_operator_sizeob__with_callobjectcache___set_size__(DeeObject *self);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__set_operator_sizeob__with_callmethodcache___set_size__(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__set_operator_sizeob__with_callkwmethodcache___set_size__(DeeObject *self);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__set_operator_sizeob__unsupported(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__set_operator_sizeob__empty(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__set_operator_sizeob__with__set_operator_size(DeeObject *self);

/* set_operator_size */
INTDEF WUNUSED NONNULL((1)) size_t DCALL default__set_operator_size(DeeObject *self);
#define default__set_operator_size__with_callattr___set_size__ (*(size_t (DCALL *)(DeeObject *))&default__set_operator_size__with__set_operator_sizeob)
#define default__set_operator_size__with_callobjectcache___set_size__ (*(size_t (DCALL *)(DeeObject *))&default__set_operator_size__with__set_operator_sizeob)
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
#define default__set_operator_size__with_callmethodcache___set_size__ (*(size_t (DCALL *)(DeeObject *))&default__set_operator_size__with__set_operator_sizeob)
#define default__set_operator_size__with_callkwmethodcache___set_size__ (*(size_t (DCALL *)(DeeObject *))&default__set_operator_size__with__set_operator_sizeob)
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1)) size_t DCALL default__set_operator_size__unsupported(DeeObject *self);
#define default__set_operator_size__empty (*(size_t (DCALL *)(DeeObject *))&_DeeNone_rets0_1)
INTDEF WUNUSED NONNULL((1)) size_t DCALL default__set_operator_size__with__set_operator_foreach(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) size_t DCALL default__set_operator_size__with__set_operator_sizeob(DeeObject *self);

/* set_operator_hash */
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL default__set_operator_hash(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL default__set_operator_hash__with_callattr___set_hash__(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL default__set_operator_hash__with_callobjectcache___set_hash__(DeeObject *self);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL default__set_operator_hash__with_callmethodcache___set_hash__(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL default__set_operator_hash__with_callkwmethodcache___set_hash__(DeeObject *self);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
#define default__set_operator_hash__unsupported (*(Dee_hash_t (DCALL *)(DeeObject *))&default__seq_operator_hash__unsupported)
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL default__set_operator_hash__empty(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL default__set_operator_hash__with__set_operator_foreach(DeeObject *self);

/* map_operator_getitem */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_getitem(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_getitem__with_callattr___map_getitem__(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_getitem__with_callobjectcache___map_getitem__(DeeObject *self, DeeObject *key);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_getitem__with_callmethodcache___map_getitem__(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_getitem__with_callkwmethodcache___map_getitem__(DeeObject *self, DeeObject *key);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_getitem__unsupported(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_getitem__empty(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_getitem__with__map_operator_getitem_index__and__map_operator_getitem_string_len_hash(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_getitem__with__map_operator_getitem_index__and__map_operator_getitem_string_hash(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_getitem__with__map_operator_getitem_string_len_hash(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_getitem__with__map_operator_getitem_string_hash(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_getitem__with__map_operator_getitem_index(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_getitem__with__map_enumerate(DeeObject *self, DeeObject *key);

/* map_operator_trygetitem */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_trygetitem(DeeObject *self, DeeObject *key);
#define default__map_operator_trygetitem__with_callattr___map_getitem__ (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&default__map_operator_trygetitem__with__map_operator_getitem)
#define default__map_operator_trygetitem__with_callobjectcache___map_getitem__ (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&default__map_operator_trygetitem__with__map_operator_getitem)
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
#define default__map_operator_trygetitem__with_callmethodcache___map_getitem__ (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&default__map_operator_trygetitem__with__map_operator_getitem)
#define default__map_operator_trygetitem__with_callkwmethodcache___map_getitem__ (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&default__map_operator_trygetitem__with__map_operator_getitem)
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
#define default__map_operator_trygetitem__unsupported (*(DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&default__map_operator_getitem__unsupported)
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_trygetitem__empty(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_trygetitem__with__map_operator_trygetitem_index__and__map_operator_trygetitem_string_len_hash(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_trygetitem__with__map_operator_trygetitem_index__and__map_operator_trygetitem_string_hash(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_trygetitem__with__map_operator_trygetitem_string_len_hash(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_trygetitem__with__map_operator_trygetitem_string_hash(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_trygetitem__with__map_operator_trygetitem_index(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_trygetitem__with__map_enumerate(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_trygetitem__with__map_operator_getitem(DeeObject *self, DeeObject *key);

/* map_operator_getitem_index */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_operator_getitem_index(DeeObject *self, size_t key);
#define default__map_operator_getitem_index__with_callattr___map_getitem__ (*(DREF DeeObject *(DCALL *)(DeeObject *, size_t))&default__map_operator_getitem_index__with__map_operator_getitem)
#define default__map_operator_getitem_index__with_callobjectcache___map_getitem__ (*(DREF DeeObject *(DCALL *)(DeeObject *, size_t))&default__map_operator_getitem_index__with__map_operator_getitem)
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
#define default__map_operator_getitem_index__with_callmethodcache___map_getitem__ (*(DREF DeeObject *(DCALL *)(DeeObject *, size_t))&default__map_operator_getitem_index__with__map_operator_getitem)
#define default__map_operator_getitem_index__with_callkwmethodcache___map_getitem__ (*(DREF DeeObject *(DCALL *)(DeeObject *, size_t))&default__map_operator_getitem_index__with__map_operator_getitem)
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_operator_getitem_index__unsupported(DeeObject *self, size_t key);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_operator_getitem_index__empty(DeeObject *self, size_t key);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_operator_getitem_index__with__map_operator_getitem(DeeObject *self, size_t key);

/* map_operator_trygetitem_index */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_operator_trygetitem_index(DeeObject *self, size_t key);
#define default__map_operator_trygetitem_index__with_callattr___map_getitem__ (*(DREF DeeObject *(DCALL *)(DeeObject *, size_t))&default__map_operator_trygetitem_index__with__map_operator_trygetitem)
#define default__map_operator_trygetitem_index__with_callobjectcache___map_getitem__ (*(DREF DeeObject *(DCALL *)(DeeObject *, size_t))&default__map_operator_trygetitem_index__with__map_operator_trygetitem)
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
#define default__map_operator_trygetitem_index__with_callmethodcache___map_getitem__ (*(DREF DeeObject *(DCALL *)(DeeObject *, size_t))&default__map_operator_trygetitem_index__with__map_operator_trygetitem)
#define default__map_operator_trygetitem_index__with_callkwmethodcache___map_getitem__ (*(DREF DeeObject *(DCALL *)(DeeObject *, size_t))&default__map_operator_trygetitem_index__with__map_operator_trygetitem)
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
#define default__map_operator_trygetitem_index__unsupported (*(DREF DeeObject *(DCALL *)(DeeObject *, size_t))&default__map_operator_getitem_index__unsupported)
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_operator_trygetitem_index__empty(DeeObject *self, size_t key);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_operator_trygetitem_index__with__map_operator_trygetitem(DeeObject *self, size_t key);

/* map_operator_getitem_string_hash */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_getitem_string_hash(DeeObject *self, char const *key, Dee_hash_t hash);
#define default__map_operator_getitem_string_hash__with_callattr___map_getitem__ (*(DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&default__map_operator_getitem_string_hash__with__map_operator_getitem)
#define default__map_operator_getitem_string_hash__with_callobjectcache___map_getitem__ (*(DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&default__map_operator_getitem_string_hash__with__map_operator_getitem)
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
#define default__map_operator_getitem_string_hash__with_callmethodcache___map_getitem__ (*(DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&default__map_operator_getitem_string_hash__with__map_operator_getitem)
#define default__map_operator_getitem_string_hash__with_callkwmethodcache___map_getitem__ (*(DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&default__map_operator_getitem_string_hash__with__map_operator_getitem)
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_getitem_string_hash__unsupported(DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_getitem_string_hash__empty(DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_getitem_string_hash__with__map_enumerate(DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_getitem_string_hash__with__map_operator_getitem(DeeObject *self, char const *key, Dee_hash_t hash);

/* map_operator_trygetitem_string_hash */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_trygetitem_string_hash(DeeObject *self, char const *key, Dee_hash_t hash);
#define default__map_operator_trygetitem_string_hash__with_callattr___map_getitem__ (*(DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&default__map_operator_trygetitem_string_hash__with__map_operator_trygetitem)
#define default__map_operator_trygetitem_string_hash__with_callobjectcache___map_getitem__ (*(DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&default__map_operator_trygetitem_string_hash__with__map_operator_trygetitem)
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
#define default__map_operator_trygetitem_string_hash__with_callmethodcache___map_getitem__ (*(DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&default__map_operator_trygetitem_string_hash__with__map_operator_trygetitem)
#define default__map_operator_trygetitem_string_hash__with_callkwmethodcache___map_getitem__ (*(DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&default__map_operator_trygetitem_string_hash__with__map_operator_trygetitem)
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
#define default__map_operator_trygetitem_string_hash__unsupported (*(DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&default__map_operator_getitem_string_hash__unsupported)
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_trygetitem_string_hash__empty(DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_trygetitem_string_hash__with__map_enumerate(DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_trygetitem_string_hash__with__map_operator_trygetitem(DeeObject *self, char const *key, Dee_hash_t hash);

/* map_operator_getitem_string_len_hash */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_operator_getitem_string_len_hash(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
#define default__map_operator_getitem_string_len_hash__with_callattr___map_getitem__ (*(DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&default__map_operator_getitem_string_len_hash__with__map_operator_getitem)
#define default__map_operator_getitem_string_len_hash__with_callobjectcache___map_getitem__ (*(DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&default__map_operator_getitem_string_len_hash__with__map_operator_getitem)
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
#define default__map_operator_getitem_string_len_hash__with_callmethodcache___map_getitem__ (*(DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&default__map_operator_getitem_string_len_hash__with__map_operator_getitem)
#define default__map_operator_getitem_string_len_hash__with_callkwmethodcache___map_getitem__ (*(DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&default__map_operator_getitem_string_len_hash__with__map_operator_getitem)
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_operator_getitem_string_len_hash__unsupported(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_operator_getitem_string_len_hash__empty(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_operator_getitem_string_len_hash__with__map_enumerate(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_operator_getitem_string_len_hash__with__map_operator_getitem(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);

/* map_operator_trygetitem_string_len_hash */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_operator_trygetitem_string_len_hash(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
#define default__map_operator_trygetitem_string_len_hash__with_callattr___map_getitem__ (*(DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&default__map_operator_trygetitem_string_len_hash__with__map_operator_trygetitem)
#define default__map_operator_trygetitem_string_len_hash__with_callobjectcache___map_getitem__ (*(DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&default__map_operator_trygetitem_string_len_hash__with__map_operator_trygetitem)
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
#define default__map_operator_trygetitem_string_len_hash__with_callmethodcache___map_getitem__ (*(DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&default__map_operator_trygetitem_string_len_hash__with__map_operator_trygetitem)
#define default__map_operator_trygetitem_string_len_hash__with_callkwmethodcache___map_getitem__ (*(DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&default__map_operator_trygetitem_string_len_hash__with__map_operator_trygetitem)
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
#define default__map_operator_trygetitem_string_len_hash__unsupported (*(DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&default__map_operator_getitem_string_len_hash__unsupported)
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_operator_trygetitem_string_len_hash__empty(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_operator_trygetitem_string_len_hash__with__map_enumerate(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_operator_trygetitem_string_len_hash__with__map_operator_trygetitem(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);

/* map_operator_bounditem */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_bounditem(DeeObject *self, DeeObject *key);
#define default__map_operator_bounditem__with_callattr___map_getitem__ (*(int (DCALL *)(DeeObject *, DeeObject *))&default__map_operator_bounditem__with__map_operator_getitem)
#define default__map_operator_bounditem__with_callobjectcache___map_getitem__ (*(int (DCALL *)(DeeObject *, DeeObject *))&default__map_operator_bounditem__with__map_operator_getitem)
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
#define default__map_operator_bounditem__with_callmethodcache___map_getitem__ (*(int (DCALL *)(DeeObject *, DeeObject *))&default__map_operator_bounditem__with__map_operator_getitem)
#define default__map_operator_bounditem__with_callkwmethodcache___map_getitem__ (*(int (DCALL *)(DeeObject *, DeeObject *))&default__map_operator_bounditem__with__map_operator_getitem)
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_bounditem__unsupported(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_bounditem__empty(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_bounditem__with__map_enumerate(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_bounditem__with__map_operator_contains(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_bounditem__with__map_operator_getitem(DeeObject *self, DeeObject *key);

/* map_operator_bounditem_index */
INTDEF WUNUSED NONNULL((1)) int DCALL default__map_operator_bounditem_index(DeeObject *self, size_t key);
#define default__map_operator_bounditem_index__with_callattr___map_getitem__ (*(int (DCALL *)(DeeObject *, size_t))&default__map_operator_bounditem_index__with__map_operator_getitem_index)
#define default__map_operator_bounditem_index__with_callobjectcache___map_getitem__ (*(int (DCALL *)(DeeObject *, size_t))&default__map_operator_bounditem_index__with__map_operator_getitem_index)
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
#define default__map_operator_bounditem_index__with_callmethodcache___map_getitem__ (*(int (DCALL *)(DeeObject *, size_t))&default__map_operator_bounditem_index__with__map_operator_getitem_index)
#define default__map_operator_bounditem_index__with_callkwmethodcache___map_getitem__ (*(int (DCALL *)(DeeObject *, size_t))&default__map_operator_bounditem_index__with__map_operator_getitem_index)
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1)) int DCALL default__map_operator_bounditem_index__unsupported(DeeObject *self, size_t key);
INTDEF WUNUSED NONNULL((1)) int DCALL default__map_operator_bounditem_index__empty(DeeObject *self, size_t key);
INTDEF WUNUSED NONNULL((1)) int DCALL default__map_operator_bounditem_index__with__map_operator_bounditem(DeeObject *self, size_t key);
INTDEF WUNUSED NONNULL((1)) int DCALL default__map_operator_bounditem_index__with__map_operator_getitem_index(DeeObject *self, size_t key);

/* map_operator_bounditem_string_hash */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_bounditem_string_hash(DeeObject *self, char const *key, Dee_hash_t hash);
#define default__map_operator_bounditem_string_hash__with_callattr___map_getitem__ (*(int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&default__map_operator_bounditem_string_hash__with__map_operator_getitem_string_hash)
#define default__map_operator_bounditem_string_hash__with_callobjectcache___map_getitem__ (*(int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&default__map_operator_bounditem_string_hash__with__map_operator_getitem_string_hash)
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
#define default__map_operator_bounditem_string_hash__with_callmethodcache___map_getitem__ (*(int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&default__map_operator_bounditem_string_hash__with__map_operator_getitem_string_hash)
#define default__map_operator_bounditem_string_hash__with_callkwmethodcache___map_getitem__ (*(int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&default__map_operator_bounditem_string_hash__with__map_operator_getitem_string_hash)
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_bounditem_string_hash__unsupported(DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_bounditem_string_hash__empty(DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_bounditem_string_hash__with__map_operator_bounditem(DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_bounditem_string_hash__with__map_operator_getitem_string_hash(DeeObject *self, char const *key, Dee_hash_t hash);

/* map_operator_bounditem_string_len_hash */
INTDEF WUNUSED NONNULL((1)) int DCALL default__map_operator_bounditem_string_len_hash(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
#define default__map_operator_bounditem_string_len_hash__with_callattr___map_getitem__ (*(int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&default__map_operator_bounditem_string_len_hash__with__map_operator_getitem_string_len_hash)
#define default__map_operator_bounditem_string_len_hash__with_callobjectcache___map_getitem__ (*(int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&default__map_operator_bounditem_string_len_hash__with__map_operator_getitem_string_len_hash)
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
#define default__map_operator_bounditem_string_len_hash__with_callmethodcache___map_getitem__ (*(int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&default__map_operator_bounditem_string_len_hash__with__map_operator_getitem_string_len_hash)
#define default__map_operator_bounditem_string_len_hash__with_callkwmethodcache___map_getitem__ (*(int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&default__map_operator_bounditem_string_len_hash__with__map_operator_getitem_string_len_hash)
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1)) int DCALL default__map_operator_bounditem_string_len_hash__unsupported(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1)) int DCALL default__map_operator_bounditem_string_len_hash__empty(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1)) int DCALL default__map_operator_bounditem_string_len_hash__with__map_operator_bounditem(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1)) int DCALL default__map_operator_bounditem_string_len_hash__with__map_operator_getitem_string_len_hash(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);

/* map_operator_hasitem */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_hasitem(DeeObject *self, DeeObject *key);
#define default__map_operator_hasitem__with_callattr___map_getitem__ (*(int (DCALL *)(DeeObject *, DeeObject *))&default__map_operator_hasitem__with__map_operator_bounditem)
#define default__map_operator_hasitem__with_callobjectcache___map_getitem__ (*(int (DCALL *)(DeeObject *, DeeObject *))&default__map_operator_hasitem__with__map_operator_bounditem)
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
#define default__map_operator_hasitem__with_callmethodcache___map_getitem__ (*(int (DCALL *)(DeeObject *, DeeObject *))&default__map_operator_hasitem__with__map_operator_bounditem)
#define default__map_operator_hasitem__with_callkwmethodcache___map_getitem__ (*(int (DCALL *)(DeeObject *, DeeObject *))&default__map_operator_hasitem__with__map_operator_bounditem)
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
#define default__map_operator_hasitem__unsupported (*(int (DCALL *)(DeeObject *, DeeObject *))&default__map_operator_hasitem__with__map_operator_bounditem)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_hasitem__with__map_operator_bounditem(DeeObject *self, DeeObject *key);

/* map_operator_hasitem_index */
INTDEF WUNUSED NONNULL((1)) int DCALL default__map_operator_hasitem_index(DeeObject *self, size_t key);
#define default__map_operator_hasitem_index__with_callattr___map_getitem__ (*(int (DCALL *)(DeeObject *, size_t))&default__map_operator_hasitem_index__with__map_operator_bounditem_index)
#define default__map_operator_hasitem_index__with_callobjectcache___map_getitem__ (*(int (DCALL *)(DeeObject *, size_t))&default__map_operator_hasitem_index__with__map_operator_bounditem_index)
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
#define default__map_operator_hasitem_index__with_callmethodcache___map_getitem__ (*(int (DCALL *)(DeeObject *, size_t))&default__map_operator_hasitem_index__with__map_operator_bounditem_index)
#define default__map_operator_hasitem_index__with_callkwmethodcache___map_getitem__ (*(int (DCALL *)(DeeObject *, size_t))&default__map_operator_hasitem_index__with__map_operator_bounditem_index)
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
#define default__map_operator_hasitem_index__unsupported (*(int (DCALL *)(DeeObject *, size_t))&default__map_operator_hasitem_index__with__map_operator_bounditem_index)
INTDEF WUNUSED NONNULL((1)) int DCALL default__map_operator_hasitem_index__with__map_operator_bounditem_index(DeeObject *self, size_t key);

/* map_operator_hasitem_string_hash */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_hasitem_string_hash(DeeObject *self, char const *key, Dee_hash_t hash);
#define default__map_operator_hasitem_string_hash__with_callattr___map_getitem__ (*(int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&default__map_operator_hasitem_string_hash__with__map_operator_bounditem_string_hash)
#define default__map_operator_hasitem_string_hash__with_callobjectcache___map_getitem__ (*(int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&default__map_operator_hasitem_string_hash__with__map_operator_bounditem_string_hash)
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
#define default__map_operator_hasitem_string_hash__with_callmethodcache___map_getitem__ (*(int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&default__map_operator_hasitem_string_hash__with__map_operator_bounditem_string_hash)
#define default__map_operator_hasitem_string_hash__with_callkwmethodcache___map_getitem__ (*(int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&default__map_operator_hasitem_string_hash__with__map_operator_bounditem_string_hash)
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
#define default__map_operator_hasitem_string_hash__unsupported (*(int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&default__map_operator_hasitem_string_hash__with__map_operator_bounditem_string_hash)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_hasitem_string_hash__with__map_operator_bounditem_string_hash(DeeObject *self, char const *key, Dee_hash_t hash);

/* map_operator_hasitem_string_len_hash */
INTDEF WUNUSED NONNULL((1)) int DCALL default__map_operator_hasitem_string_len_hash(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
#define default__map_operator_hasitem_string_len_hash__with_callattr___map_getitem__ (*(int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&default__map_operator_hasitem_string_len_hash__with__map_operator_bounditem_string_len_hash)
#define default__map_operator_hasitem_string_len_hash__with_callobjectcache___map_getitem__ (*(int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&default__map_operator_hasitem_string_len_hash__with__map_operator_bounditem_string_len_hash)
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
#define default__map_operator_hasitem_string_len_hash__with_callmethodcache___map_getitem__ (*(int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&default__map_operator_hasitem_string_len_hash__with__map_operator_bounditem_string_len_hash)
#define default__map_operator_hasitem_string_len_hash__with_callkwmethodcache___map_getitem__ (*(int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&default__map_operator_hasitem_string_len_hash__with__map_operator_bounditem_string_len_hash)
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
#define default__map_operator_hasitem_string_len_hash__unsupported (*(int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&default__map_operator_hasitem_string_len_hash__with__map_operator_bounditem_string_len_hash)
INTDEF WUNUSED NONNULL((1)) int DCALL default__map_operator_hasitem_string_len_hash__with__map_operator_bounditem_string_len_hash(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);

/* map_operator_delitem */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_delitem(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_delitem__with_callattr___map_delitem__(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_delitem__with_callobjectcache___map_delitem__(DeeObject *self, DeeObject *key);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_delitem__with_callmethodcache___map_delitem__(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_delitem__with_callkwmethodcache___map_delitem__(DeeObject *self, DeeObject *key);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_delitem__unsupported(DeeObject *self, DeeObject *key);
#define default__map_operator_delitem__empty (*(int (DCALL *)(DeeObject *, DeeObject *))&_DeeNone_reti0_2)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_delitem__with__map_operator_delitem_index__and__map_operator_delitem_string_len_hash(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_delitem__with__map_operator_delitem_index__and__map_operator_delitem_string_hash(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_delitem__with__map_operator_delitem_string_len_hash(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_delitem__with__map_operator_delitem_string_hash(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_delitem__with__map_operator_delitem_index(DeeObject *self, DeeObject *key);

/* map_operator_delitem_index */
INTDEF WUNUSED NONNULL((1)) int DCALL default__map_operator_delitem_index(DeeObject *self, size_t key);
#define default__map_operator_delitem_index__with_callattr___map_delitem__ (*(int (DCALL *)(DeeObject *, size_t))&default__map_operator_delitem_index__with__map_operator_delitem)
#define default__map_operator_delitem_index__with_callobjectcache___map_delitem__ (*(int (DCALL *)(DeeObject *, size_t))&default__map_operator_delitem_index__with__map_operator_delitem)
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
#define default__map_operator_delitem_index__with_callmethodcache___map_delitem__ (*(int (DCALL *)(DeeObject *, size_t))&default__map_operator_delitem_index__with__map_operator_delitem)
#define default__map_operator_delitem_index__with_callkwmethodcache___map_delitem__ (*(int (DCALL *)(DeeObject *, size_t))&default__map_operator_delitem_index__with__map_operator_delitem)
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1)) int DCALL default__map_operator_delitem_index__unsupported(DeeObject *self, size_t key);
#define default__map_operator_delitem_index__empty (*(int (DCALL *)(DeeObject *, size_t))&_DeeNone_reti0_2)
INTDEF WUNUSED NONNULL((1)) int DCALL default__map_operator_delitem_index__with__map_operator_delitem(DeeObject *self, size_t key);

/* map_operator_delitem_string_hash */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_delitem_string_hash(DeeObject *self, char const *key, Dee_hash_t hash);
#define default__map_operator_delitem_string_hash__with_callattr___map_delitem__ (*(int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&default__map_operator_delitem_string_hash__with__map_operator_delitem)
#define default__map_operator_delitem_string_hash__with_callobjectcache___map_delitem__ (*(int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&default__map_operator_delitem_string_hash__with__map_operator_delitem)
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
#define default__map_operator_delitem_string_hash__with_callmethodcache___map_delitem__ (*(int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&default__map_operator_delitem_string_hash__with__map_operator_delitem)
#define default__map_operator_delitem_string_hash__with_callkwmethodcache___map_delitem__ (*(int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&default__map_operator_delitem_string_hash__with__map_operator_delitem)
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_delitem_string_hash__unsupported(DeeObject *self, char const *key, Dee_hash_t hash);
#define default__map_operator_delitem_string_hash__empty (*(int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&_DeeNone_reti0_3)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__map_operator_delitem_string_hash__with__map_operator_delitem(DeeObject *self, char const *key, Dee_hash_t hash);

/* map_operator_delitem_string_len_hash */
INTDEF WUNUSED NONNULL((1)) int DCALL default__map_operator_delitem_string_len_hash(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
#define default__map_operator_delitem_string_len_hash__with_callattr___map_delitem__ (*(int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&default__map_operator_delitem_string_len_hash__with__map_operator_delitem)
#define default__map_operator_delitem_string_len_hash__with_callobjectcache___map_delitem__ (*(int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&default__map_operator_delitem_string_len_hash__with__map_operator_delitem)
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
#define default__map_operator_delitem_string_len_hash__with_callmethodcache___map_delitem__ (*(int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&default__map_operator_delitem_string_len_hash__with__map_operator_delitem)
#define default__map_operator_delitem_string_len_hash__with_callkwmethodcache___map_delitem__ (*(int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&default__map_operator_delitem_string_len_hash__with__map_operator_delitem)
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1)) int DCALL default__map_operator_delitem_string_len_hash__unsupported(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
#define default__map_operator_delitem_string_len_hash__empty (*(int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&_DeeNone_reti0_4)
INTDEF WUNUSED NONNULL((1)) int DCALL default__map_operator_delitem_string_len_hash__with__map_operator_delitem(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);

/* map_operator_setitem */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__map_operator_setitem(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__map_operator_setitem__with_callattr___map_setitem__(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__map_operator_setitem__with_callobjectcache___map_setitem__(DeeObject *self, DeeObject *key, DeeObject *value);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__map_operator_setitem__with_callmethodcache___map_setitem__(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__map_operator_setitem__with_callkwmethodcache___map_setitem__(DeeObject *self, DeeObject *key, DeeObject *value);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__map_operator_setitem__unsupported(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__map_operator_setitem__empty(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__map_operator_setitem__with__map_operator_setitem_index__and__map_operator_setitem_string_len_hash(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__map_operator_setitem__with__map_operator_setitem_index__and__map_operator_setitem_string_hash(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__map_operator_setitem__with__map_operator_setitem_string_len_hash(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__map_operator_setitem__with__map_operator_setitem_string_hash(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__map_operator_setitem__with__map_operator_setitem_index(DeeObject *self, DeeObject *key, DeeObject *value);

/* map_operator_setitem_index */
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__map_operator_setitem_index(DeeObject *self, size_t key, DeeObject *value);
#define default__map_operator_setitem_index__with_callattr___map_setitem__ (*(int (DCALL *)(DeeObject *, size_t, DeeObject *))&default__map_operator_setitem_index__with__map_operator_setitem)
#define default__map_operator_setitem_index__with_callobjectcache___map_setitem__ (*(int (DCALL *)(DeeObject *, size_t, DeeObject *))&default__map_operator_setitem_index__with__map_operator_setitem)
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
#define default__map_operator_setitem_index__with_callmethodcache___map_setitem__ (*(int (DCALL *)(DeeObject *, size_t, DeeObject *))&default__map_operator_setitem_index__with__map_operator_setitem)
#define default__map_operator_setitem_index__with_callkwmethodcache___map_setitem__ (*(int (DCALL *)(DeeObject *, size_t, DeeObject *))&default__map_operator_setitem_index__with__map_operator_setitem)
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__map_operator_setitem_index__unsupported(DeeObject *self, size_t key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__map_operator_setitem_index__empty(DeeObject *self, size_t key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__map_operator_setitem_index__with__map_operator_setitem(DeeObject *self, size_t key, DeeObject *value);

/* map_operator_setitem_string_hash */
INTDEF WUNUSED NONNULL((1, 2, 4)) int DCALL default__map_operator_setitem_string_hash(DeeObject *self, char const *key, Dee_hash_t hash, DeeObject *value);
#define default__map_operator_setitem_string_hash__with_callattr___map_setitem__ (*(int (DCALL *)(DeeObject *, char const *, Dee_hash_t, DeeObject *))&default__map_operator_setitem_string_hash__with__map_operator_setitem)
#define default__map_operator_setitem_string_hash__with_callobjectcache___map_setitem__ (*(int (DCALL *)(DeeObject *, char const *, Dee_hash_t, DeeObject *))&default__map_operator_setitem_string_hash__with__map_operator_setitem)
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
#define default__map_operator_setitem_string_hash__with_callmethodcache___map_setitem__ (*(int (DCALL *)(DeeObject *, char const *, Dee_hash_t, DeeObject *))&default__map_operator_setitem_string_hash__with__map_operator_setitem)
#define default__map_operator_setitem_string_hash__with_callkwmethodcache___map_setitem__ (*(int (DCALL *)(DeeObject *, char const *, Dee_hash_t, DeeObject *))&default__map_operator_setitem_string_hash__with__map_operator_setitem)
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2, 4)) int DCALL default__map_operator_setitem_string_hash__unsupported(DeeObject *self, char const *key, Dee_hash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 4)) int DCALL default__map_operator_setitem_string_hash__empty(DeeObject *self, char const *key, Dee_hash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 4)) int DCALL default__map_operator_setitem_string_hash__with__map_operator_setitem(DeeObject *self, char const *key, Dee_hash_t hash, DeeObject *value);

/* map_operator_setitem_string_len_hash */
INTDEF WUNUSED NONNULL((1, 5)) int DCALL default__map_operator_setitem_string_len_hash(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash, DeeObject *value);
#define default__map_operator_setitem_string_len_hash__with_callattr___map_setitem__ (*(int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t, DeeObject *))&default__map_operator_setitem_string_len_hash__with__map_operator_setitem)
#define default__map_operator_setitem_string_len_hash__with_callobjectcache___map_setitem__ (*(int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t, DeeObject *))&default__map_operator_setitem_string_len_hash__with__map_operator_setitem)
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
#define default__map_operator_setitem_string_len_hash__with_callmethodcache___map_setitem__ (*(int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t, DeeObject *))&default__map_operator_setitem_string_len_hash__with__map_operator_setitem)
#define default__map_operator_setitem_string_len_hash__with_callkwmethodcache___map_setitem__ (*(int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t, DeeObject *))&default__map_operator_setitem_string_len_hash__with__map_operator_setitem)
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 5)) int DCALL default__map_operator_setitem_string_len_hash__unsupported(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 5)) int DCALL default__map_operator_setitem_string_len_hash__empty(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 5)) int DCALL default__map_operator_setitem_string_len_hash__with__map_operator_setitem(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash, DeeObject *value);

/* map_operator_contains */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_contains(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_contains__with_callattr___map_contains__(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_contains__with_callobjectcache___map_contains__(DeeObject *self, DeeObject *key);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_contains__with_callmethodcache___map_contains__(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_contains__with_callkwmethodcache___map_contains__(DeeObject *self, DeeObject *key);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_contains__unsupported(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_contains__empty(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_contains__with__map_operator_trygetitem(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__map_operator_contains__with__map_operator_bounditem(DeeObject *self, DeeObject *key);

/* map_iterkeys */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_iterkeys__with_callattr___map_iterkeys__(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_iterkeys__with_callobjectcache___map_iterkeys__(DeeObject *self);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_iterkeys__with_callmethodcache___map_iterkeys__(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_iterkeys__with_callkwmethodcache___map_iterkeys__(DeeObject *self);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_iterkeys__unsupported(DeeObject *self);
#define default__map_iterkeys__empty (*(DREF DeeObject *(DCALL *)(DeeObject *))&default__set_operator_iter__empty)
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_iterkeys__with__map_enumerate(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__map_iterkeys__with__set_operator_iter(DeeObject *self);

/* map_enumerate */
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__map_enumerate(DeeObject *__restrict self, Dee_seq_enumerate_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__map_enumerate__with_callattr___map_enumerate__(DeeObject *__restrict self, Dee_seq_enumerate_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__map_enumerate__with_callobjectcache___map_enumerate__(DeeObject *__restrict self, Dee_seq_enumerate_t cb, void *arg);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__map_enumerate__with_callmethodcache___map_enumerate__(DeeObject *__restrict self, Dee_seq_enumerate_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__map_enumerate__with_callkwmethodcache___map_enumerate__(DeeObject *__restrict self, Dee_seq_enumerate_t cb, void *arg);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__map_enumerate__unsupported(DeeObject *__restrict self, Dee_seq_enumerate_t cb, void *arg);
#define default__map_enumerate__empty (*(Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_seq_enumerate_t, void *))&default__set_operator_foreach_pair__empty)
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__map_enumerate__with__map_iterkeys__and__map_operator_trygetitem(DeeObject *__restrict self, Dee_seq_enumerate_t cb, void *arg);

/* map_enumerate_range */
INTDEF WUNUSED NONNULL((1, 2, 4, 5)) Dee_ssize_t DCALL default__map_enumerate_range__with_callattr___map_enumerate__(DeeObject *self, Dee_seq_enumerate_t cb, void *arg, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 4, 5)) Dee_ssize_t DCALL default__map_enumerate_range__with_callobjectcache___map_enumerate__(DeeObject *self, Dee_seq_enumerate_t cb, void *arg, DeeObject *start, DeeObject *end);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2, 4, 5)) Dee_ssize_t DCALL default__map_enumerate_range__with_callmethodcache___map_enumerate__(DeeObject *self, Dee_seq_enumerate_t cb, void *arg, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 4, 5)) Dee_ssize_t DCALL default__map_enumerate_range__with_callkwmethodcache___map_enumerate__(DeeObject *self, Dee_seq_enumerate_t cb, void *arg, DeeObject *start, DeeObject *end);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2, 4, 5)) Dee_ssize_t DCALL default__map_enumerate_range__unsupported(DeeObject *self, Dee_seq_enumerate_t cb, void *arg, DeeObject *start, DeeObject *end);
#define default__map_enumerate_range__empty (*(Dee_ssize_t (DCALL *)(DeeObject *, Dee_seq_enumerate_t, void *, DeeObject *, DeeObject *))&_DeeNone_rets0_5)
INTDEF WUNUSED NONNULL((1, 2, 4, 5)) Dee_ssize_t DCALL default__map_enumerate_range__with__map_enumerate(DeeObject *self, Dee_seq_enumerate_t cb, void *arg, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 4, 5)) Dee_ssize_t DCALL default__map_enumerate_range__with__map_iterkeys__and__map_operator_trygetitem(DeeObject *self, Dee_seq_enumerate_t cb, void *arg, DeeObject *start, DeeObject *end);
/*[[[end]]]*/
/* clang-format on */

DECL_END
#endif /* CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */

#endif /* !GUARD_DEEMON_RUNTIME_METHOD_HINT_DEFAULTS_H */
