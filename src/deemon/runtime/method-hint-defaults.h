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
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_bool__with_callattr___seq_bool__(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_bool__with_callobjectcache___seq_bool__(DeeObject *self);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_bool__with_callmethodcache___seq_bool__(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_bool__with_callkwmethodcache___seq_bool__(DeeObject *self);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_bool__unsupported(DeeObject *self);
#define default__seq_operator_bool__empty (*(int (DCALL *)(DeeObject *))&_DeeNone_reti0_1)
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_bool__with__seq_operator_foreach(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_operator_bool__with__seq_operator_size(DeeObject *self);

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
#define default__seq_operator_size__unsupported (*(size_t (DCALL *)(DeeObject *))&default__seq_operator_size__with__seq_operator_sizeob)
INTDEF WUNUSED NONNULL((1)) size_t DCALL default__seq_operator_size__empty(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) size_t DCALL default__seq_operator_size__with__seq_operator_foreach(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) size_t DCALL default__seq_operator_size__with__seq_operator_sizeob(DeeObject *self);

/* seq_operator_iter */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_iter__with_callattr___seq_iter__(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_iter__with_callobjectcache___seq_iter__(DeeObject *self);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_iter__with_callmethodcache___seq_iter__(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_iter__with_callkwmethodcache___seq_iter__(DeeObject *self);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_iter__unsupported(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_operator_iter__empty(DeeObject *self);

/* seq_operator_foreach */
#define default__seq_operator_foreach__with_callattr___seq_iter__ (*(Dee_ssize_t (DCALL *)(DeeObject *, Dee_foreach_t, void *))&default__seq_operator_foreach__with__seq_operator_iter)
#define default__seq_operator_foreach__with_callobjectcache___seq_iter__ (*(Dee_ssize_t (DCALL *)(DeeObject *, Dee_foreach_t, void *))&default__seq_operator_foreach__with__seq_operator_iter)
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
#define default__seq_operator_foreach__with_callmethodcache___seq_iter__ (*(Dee_ssize_t (DCALL *)(DeeObject *, Dee_foreach_t, void *))&default__seq_operator_foreach__with__seq_operator_iter)
#define default__seq_operator_foreach__with_callkwmethodcache___seq_iter__ (*(Dee_ssize_t (DCALL *)(DeeObject *, Dee_foreach_t, void *))&default__seq_operator_foreach__with__seq_operator_iter)
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
#define default__seq_operator_foreach__unsupported (*(Dee_ssize_t (DCALL *)(DeeObject *, Dee_foreach_t, void *))&default__seq_operator_foreach__with__seq_operator_iter)
#define default__seq_operator_foreach__empty (*(Dee_ssize_t (DCALL *)(DeeObject *, Dee_foreach_t, void *))&_DeeNone_rets0_3)
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL default__seq_operator_foreach__with__seq_operator_iter(DeeObject *self, Dee_foreach_t proc, void *arg);

/* seq_operator_foreach_pair */
#define default__seq_operator_foreach_pair__with_callattr___seq_iter__ (*(Dee_ssize_t (DCALL *)(DeeObject *, Dee_foreach_pair_t, void *))&default__seq_operator_foreach_pair__with__seq_operator_iter)
#define default__seq_operator_foreach_pair__with_callobjectcache___seq_iter__ (*(Dee_ssize_t (DCALL *)(DeeObject *, Dee_foreach_pair_t, void *))&default__seq_operator_foreach_pair__with__seq_operator_iter)
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
#define default__seq_operator_foreach_pair__with_callmethodcache___seq_iter__ (*(Dee_ssize_t (DCALL *)(DeeObject *, Dee_foreach_pair_t, void *))&default__seq_operator_foreach_pair__with__seq_operator_iter)
#define default__seq_operator_foreach_pair__with_callkwmethodcache___seq_iter__ (*(Dee_ssize_t (DCALL *)(DeeObject *, Dee_foreach_pair_t, void *))&default__seq_operator_foreach_pair__with__seq_operator_iter)
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
#define default__seq_operator_foreach_pair__unsupported (*(Dee_ssize_t (DCALL *)(DeeObject *, Dee_foreach_pair_t, void *))&default__seq_operator_foreach_pair__with__seq_operator_iter)
#define default__seq_operator_foreach_pair__empty (*(Dee_ssize_t (DCALL *)(DeeObject *, Dee_foreach_pair_t, void *))&_DeeNone_rets0_3)
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL default__seq_operator_foreach_pair__with__seq_operator_iter(DeeObject *self, Dee_foreach_pair_t proc, void *arg);

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
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_any__with__seq_foreach(DeeObject *__restrict self);

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
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_any_with_key__with__seq_foreach(DeeObject *self, DeeObject *key);

/* seq_any_with_range */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_any_with_range__with_callattr_any(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_any_with_range__with_callattr___seq_any__(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_any_with_range__with_callobjectcache___seq_any__(DeeObject *self, size_t start, size_t end);
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_any_with_range__with_callmethodcache___seq_any__(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_any_with_range__with_callkwmethodcache___seq_any__(DeeObject *self, size_t start, size_t end);
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_any_with_range__unsupported(DeeObject *self, size_t start, size_t end);
#define default__seq_any_with_range__empty (*(int (DCALL *)(DeeObject *, size_t, size_t))&_DeeNone_reti0_3)
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__seq_any_with_range__with__seq_enumerate_index(DeeObject *self, size_t start, size_t end);

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

/* seq_trygetfirst */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_trygetfirst__with_callattr_first(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_trygetfirst__with_callattr___seq_first__(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_trygetfirst__with_callobjectcache___seq_first__(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_trygetfirst__unsupported(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_trygetfirst__empty(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_trygetfirst__with__seq_operator_foreach(DeeObject *self);

/* seq_getfirst */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_getfirst__with_callattr_first(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_getfirst__with_callattr___seq_first__(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_getfirst__with_callobjectcache___seq_first__(DeeObject *self);
#define default__seq_getfirst__unsupported (*(DREF DeeObject *(DCALL *)(DeeObject *))&default__seq_trygetfirst__unsupported)
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_getfirst__empty(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_getfirst__with__seq_operator_foreach(DeeObject *self);

/* seq_boundfirst */
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_boundfirst__with_callattr_first(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_boundfirst__with_callattr___seq_first__(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_boundfirst__with_callobjectcache___seq_first__(DeeObject *self);
#define default__seq_boundfirst__unsupported (*(int (DCALL *)(DeeObject *))&default__seq_boundfirst__empty)
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_boundfirst__empty(DeeObject *self);

/* seq_delfirst */
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_delfirst__with_callattr_first(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_delfirst__with_callattr___seq_first__(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_delfirst__with_callobjectcache___seq_first__(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__seq_delfirst__unsupported(DeeObject *self);
#define default__seq_delfirst__empty (*(int (DCALL *)(DeeObject *))&_DeeNone_reti0_1)

/* seq_setfirst */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_setfirst__with_callattr_first(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_setfirst__with_callattr___seq_first__(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_setfirst__with_callobjectcache___seq_first__(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_setfirst__unsupported(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__seq_setfirst__empty(DeeObject *self, DeeObject *value);
/*[[[end]]]*/
/* clang-format on */

DECL_END
#endif /* CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */

#endif /* !GUARD_DEEMON_RUNTIME_METHOD_HINT_DEFAULTS_H */
