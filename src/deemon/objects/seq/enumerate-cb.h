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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_ENUMERATE_CB_H
#define GUARD_DEEMON_OBJECTS_SEQ_ENUMERATE_CB_H 1

#include <deemon/api.h>

#include <deemon/method-hints.h> /* Dee_seq_enumerate_index_t, Dee_seq_enumerate_t */
#include <deemon/object.h>
#include <deemon/util/rlock.h>   /* Dee_rshared_lock_t */

#include <stddef.h> /* size_t */

DECL_BEGIN

/************************************************************************/
/* USER -> NATIVE                                                       */
/************************************************************************/

/* For calling the native "seq_enumerate" / "seq_enumerate_index"
 * method hints with user-code function callbacks. */
struct seq_enumerate_data {
	DeeObject      *sed_cb;     /* [1..1] Enumeration callback */
	DREF DeeObject *sed_result; /* [?..1][valid_if(return == -2)] Enumeration result */
};

INTDEF WUNUSED NONNULL((2)) Dee_ssize_t DCALL
seq_enumerate_cb(void *arg, DeeObject *index, /*nullable*/ DeeObject *value);

INTDEF WUNUSED Dee_ssize_t DCALL
seq_enumerate_index_cb(void *arg, size_t index, /*nullable*/ DeeObject *value);

/* Helpers for enumerating a sequence by invoking a given callback. */
INTDEF NONNULL((1, 2)) DREF DeeObject *DCALL
seq_call_enumerate(DeeObject *self, DeeObject *cb);
INTDEF NONNULL((1, 2)) DREF DeeObject *DCALL
seq_call_enumerate_with_intrange(DeeObject *self, DeeObject *cb,
                                 size_t start, size_t end);
INTDEF NONNULL((1, 2, 3, 4)) DREF DeeObject *DCALL
seq_call_enumerate_with_range(DeeObject *self, DeeObject *cb,
                              DeeObject *start, DeeObject *end);

/* Helpers for enumerating a mapping by invoking a given callback. */
INTDEF NONNULL((1, 2)) DREF DeeObject *DCALL
map_call_enumerate(DeeObject *self, DeeObject *cb);
INTDEF NONNULL((1, 2, 3, 4)) DREF DeeObject *DCALL
map_call_enumerate_with_range(DeeObject *self, DeeObject *cb,
                              DeeObject *start, DeeObject *end);





/************************************************************************/
/* NATIVE -> USER                                                       */
/************************************************************************/

/* For passing a native "seq_enumerate" / "seq_enumerate_index"
 * callback to wrapped within a deemon object to user-code callbacks. */
typedef struct {
	OBJECT_HEAD
	union {
		Dee_seq_enumerate_t       cb_enumerate;       /* For `SeqEnumerateWrapper_Type' */
		Dee_seq_enumerate_index_t cb_enumerate_index; /* For `SeqEnumerateIndexWrapper_Type' */
	}                  sew_cb;   /* [1..1][lock(sew_lock)] User-defined callback (unless deleted) */
	void              *sew_arg;  /* [?..?][lock(sew_lock)] Cookie for `sew_cb' */
	Dee_ssize_t        sew_res;  /* [lock(sew_lock)] Result status. */
	DREF DeeObject    *sew_err;  /* [0..1][lock(sew_lock)] Error that was thrown by the last invocation of "sew_cb" (must be NULL when `sew_res >= 0') */
#ifndef CONFIG_NO_THREADS
	Dee_rshared_lock_t sew_lock; /* Lock for ensuring that `sew_cb' is only called from
	                             * **1** thread, and can be deleted, even if shared. */
#endif /* !CONFIG_NO_THREADS */
} SeqEnumerateWrapper;

INTDEF DeeTypeObject SeqEnumerateWrapper_Type;
INTDEF DeeTypeObject SeqEnumerateIndexWrapper_Type;

/* Destroy "self" if not shared, else clear the "sew_cb" callback,
 * to ensure that it won't be called anymore. This function also
 * briefly acquires a lock to "sew_lock" (without interrupts) to
 * ensure that after a call to this function, nothing may still
 * invoke the callback.
 *
 * @param userproc_result: The return value of the user-defined callback.
 *                         May be "NULL" if the user-defined callback threw
 *                         an error. */
INTDEF WUNUSED NONNULL((1)) Dee_ssize_t DCALL
SeqEnumerateWrapper_Decref(/*inherit(always)*/ DREF SeqEnumerateWrapper *self,
                           /*inherit(always)*/ DREF DeeObject *userproc_result);

/* Create new enumerate/enumerate_index wrapper objects. */
INTDEF WUNUSED NONNULL((1)) DREF SeqEnumerateWrapper *DCALL
SeqEnumerateWrapper_New(Dee_seq_enumerate_t cb, void *arg);
INTDEF WUNUSED NONNULL((1)) DREF SeqEnumerateWrapper *DCALL
SeqEnumerateIndexWrapper_New(Dee_seq_enumerate_index_t cb, void *arg);

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_ENUMERATE_CB_H */
