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
#ifndef GUARD_DEX_THREADING_TLS_C
#define GUARD_DEX_THREADING_TLS_C 1
#define CONFIG_BUILDING_LIBTHREADING
#define DEE_SOURCE

#include "libthreading.h"
/**/

#include <deemon/api.h>

#include <deemon/alloc.h>           /* Dee_CollectMemoryc, Dee_Free, Dee_Reallococ, Dee_TYPE_CONSTRUCTOR_INIT_FIXED, Dee_TryReallocc */
#include <deemon/arg.h>             /* DeeArg_Unpack* */
#include <deemon/bool.h>            /* return_bool */
#include <deemon/error.h>           /* DeeError_* */
#include <deemon/format.h>          /* DeeFormat_PRINT, DeeFormat_Printf */
#include <deemon/none.h>            /* DeeNone_Check, DeeNone_Singleton, Dee_None, return_none */
#include <deemon/object.h>          /* DREF, DeeObject, DeeObject_*, DeeTypeObject, Dee_AsObject, Dee_BOUND_FROMBOOL, Dee_Decref, Dee_Incref, Dee_Incref_n, Dee_XDecref, Dee_XIncref, Dee_formatprinter_t, Dee_hash_t, Dee_ssize_t, ITER_DONE, ITER_ISOK, OBJECT_HEAD, OBJECT_HEAD_INIT */
#include <deemon/system-features.h> /* bzeroc */
#include <deemon/thread.h>          /* DeeThreadObject, DeeThread_Self, Dee_THREAD_STATE_TERMINATING */
#include <deemon/type.h>            /* DeeObject_GenericCmpByAddr, DeeType_Type, Dee_TYPE_CONSTRUCTOR_INIT_FIXED, Dee_Visit, Dee_XVisit, Dee_visit_t, METHOD_FNOREFESCAPE, TF_NONE, TP_FNORMAL, TYPE_*, type_getset, type_method */
#include <deemon/util/hash.h>       /* DeeObject_HashGeneric */
#include <deemon/util/lock.h>       /* Dee_atomic_lock_* */

#include <stddef.h> /* offsetof, size_t */
#include <stdint.h> /* uint8_t */

DECL_BEGIN

#ifndef CONFIG_NO_THREADS

/* Ensure allocation and return a pointer to the TLS variable
 * slot associated with `index' within the calling thread.
 * @return: NULL: Failed to allocated the slot for the given index.
 * WARNING: Consecutive calls to this function may
 *          invalidate previously returned pointers. */
PRIVATE WUNUSED DREF DeeObject **DCALL
thread_tls_get(size_t index) {
	struct tls_descriptor *desc;
	DeeThreadObject *caller;
	caller = DeeThread_Self();
	desc   = (struct tls_descriptor *)caller->t_context.d_tls;
	if unlikely(!desc || index >= desc->td_size) {
		size_t old_size;
		if (caller->t_state & Dee_THREAD_STATE_TERMINATING) {
			/* If `Dee_THREAD_STATE_TERMINATING' is set, don't allow TLS alloc! */
			DeeError_Throwf(&DeeError_RuntimeError,
			                "Cannot allocate TLS variables for "
			                "thread %r that has begun termination",
			                caller);
			goto err;
		}
		old_size = desc ? desc->td_size : 0;
		desc = (struct tls_descriptor *)Dee_Reallococ(desc, offsetof(struct tls_descriptor, td_elem),
		                                              index + 1, sizeof(DREF DeeObject *));
		if unlikely(!desc)
			goto err;

		/* Save the new descriptor length. */
		desc->td_size = index + 1;

		/* ZERO-initialize all newly allocated indices. */
		bzeroc(desc->td_elem + old_size,
		       (index + 1) - old_size,
		       sizeof(DREF DeeObject *));

		/* Save the new descriptor. */
		caller->t_context.d_tls = (void *)desc;
	}

	/* Return a pointer into the descriptor. */
	return &desc->td_elem[index];
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject **DCALL
thread_tls_tryget(size_t index) {
	struct tls_descriptor *desc;
	DeeThreadObject *caller;
	caller = DeeThread_Self();
	desc   = (struct tls_descriptor *)caller->t_context.d_tls;
	if unlikely(!desc || index >= desc->td_size)
		goto err;

	/* Return a pointer into the descriptor. */
	return &desc->td_elem[index];
err:
	return NULL;
}


/* TLS controller callbacks for libthreading's TLS implementation. */
INTERN NONNULL((1)) void DCALL
thread_tls_fini(struct tls_descriptor *__restrict data) {
	size_t i, count = data->td_size;

	/* Decref all allocated instances within this TLS vector. */
	for (i = 0; i < count; ++i) {
		DREF DeeObject *ob = data->td_elem[i];
		if (ITER_ISOK(ob))
			Dee_Decref(ob);
	}
	Dee_Free(data);
}

/* Lock for registering TLS objects. */
PRIVATE Dee_atomic_lock_t tls_reglock = Dee_ATOMIC_LOCK_INIT;
#define tls_reglock_available()  Dee_atomic_lock_available(&tls_reglock)
#define tls_reglock_acquired()   Dee_atomic_lock_acquired(&tls_reglock)
#define tls_reglock_tryacquire() Dee_atomic_lock_tryacquire(&tls_reglock)
#define tls_reglock_acquire()    Dee_atomic_lock_acquire(&tls_reglock)
#define tls_reglock_waitfor()    Dee_atomic_lock_waitfor(&tls_reglock)
#define tls_reglock_release()    Dee_atomic_lock_release(&tls_reglock)

PRIVATE uint8_t *tls_inuse = NULL; /* [lock(tls_reglock)][0..(tls_nexti+7)/8][owned] Bitset of all TLS indices currently in use. */
PRIVATE size_t   tls_nexti = 0;    /* [lock(tls_reglock)] The next TLS index used when all others are already in use. */
PRIVATE size_t   tls_count = 0;    /* [lock(tls_reglock)] The total number of TLS indices currently assigned. */

/* Allocate a new TLS index.
 * @return: * :         The newly allocated TLS index.
 * @return: (size_t)-1: Not enough available memory. */
PRIVATE WUNUSED size_t DCALL tls_alloc(void);

/* Free a previously allocated TLS index. */
PRIVATE void DCALL tls_free(size_t index);





PRIVATE WUNUSED size_t DCALL tls_alloc(void) {
	size_t result;
again:
	tls_reglock_acquire();
	ASSERTF(tls_nexti >= tls_count, "Inconsistent TLS state");
	if (tls_nexti != tls_count) {
		/* Not all existing indices are in use. */
		uint8_t *iter = tls_inuse + (tls_nexti + 7) / 8;
		for (;;) {
			uint8_t byte, bitno;
			ASSERTF(iter != tls_inuse,
			        "But `tls_nexti' said there would be unused entries...");
			byte = *--iter;
			if (byte == 0xff)
				continue;

			/* One of these 8 indices isn't in use right now. */
			result = (size_t)(iter - tls_inuse) * 8, bitno = 0;
			while (byte & 1)
				byte >>= 1, ++bitno;
			ASSERT(!(*iter & (1 << bitno)));

			/* Mask the index as now being in-use. */
			*iter |= 1 << bitno;
			result += bitno;
			goto done;
		}
	}

	/* Use the next unused index. */
	if ((tls_nexti & 7) == 0) {
		/* Must allocate more bitset memory. */
		uint8_t *new_bitset;
		new_bitset = (uint8_t *)Dee_TryReallocc(tls_inuse, (tls_nexti / 8) + 1, sizeof(uint8_t));
		/* The the realloc failed, return `(size_t)-1'. */
		if unlikely(!new_bitset) {
			tls_reglock_release();
			/* Try to collect some memory. */
			if (Dee_CollectMemoryc((tls_nexti / 8) + 1, sizeof(uint8_t)))
				goto again;
			return (size_t)-1;
		}
		tls_inuse                = new_bitset;
		tls_inuse[tls_nexti / 8] = 0; /* Clear the newly allocated 8 bits. */
	}
	result = tls_nexti++;
	ASSERT(result != (size_t)-1);
	ASSERT(!(tls_inuse[result / 8] & (1 << (result % 8))));
	/* Mark the index as being in-use now. */
	tls_inuse[result / 8] |= 1 << (result % 8);
done:
	/* Track the total number of allocated indices. */
	++tls_count;
	tls_reglock_release();
	return result;
}

PRIVATE void DCALL tls_free(size_t index) {
	tls_reglock_acquire();
	ASSERTF(tls_count, "No indices allocated");
	ASSERTF(tls_nexti >= tls_count, "Inconsistent TLS state");
	ASSERTF(index <= tls_nexti, "Invalid index");
	ASSERTF((tls_inuse[index / 8] & (1 << (index % 8))), "The index wasn't allocated");

	/* Clear the in-use bit of the given index. */
	tls_inuse[index / 8] &= ~(1 << (index % 8));

	/* Track the total number of allocated indices. */
	if (!--tls_count) {
		/* Last index was deallocated (Free the bitset) */
		Dee_Free(tls_inuse);
		tls_inuse = NULL;
		tls_nexti = 0;
	}
	tls_reglock_release();
}




typedef struct {
	OBJECT_HEAD
	DREF DeeObject *t_factory; /* [0..1][const] A factory function used to construct TLS default value. */
	size_t          t_index;   /* [owned(tls_free)][const] The per-thread TLS index used by this controller. */
} TLS;

PRIVATE WUNUSED NONNULL((1)) int DCALL
tls_init(TLS *__restrict self,
         size_t argc, DeeObject *const *argv) {
	self->t_factory = NULL;
	DeeArg_Unpack0Or1(err, argc, argv, "TLS", &self->t_factory);
	self->t_index = tls_alloc();
	if unlikely(self->t_index == (size_t)-1)
		goto err;

	/* Save a reference for the factory. */
	Dee_XIncref(self->t_factory);
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
tls_fini(TLS *__restrict self) {
	/* Free the allocated TLS index. */
	tls_free(self->t_index);

	/* Destroy the associated factory. */
	Dee_XDecref(self->t_factory);
}

PRIVATE NONNULL((1, 2)) void DCALL
tls_visit(TLS *__restrict self, Dee_visit_t proc, void *arg) {
	Dee_XVisit(self->t_factory);
}


PRIVATE ATTR_COLD int DCALL err_tls_unbound(void) {
	return DeeError_Throwf(&DeeError_UnboundAttribute,
	                       "TLS variable is unbound");
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
tls_getvalue(TLS *__restrict self) {
	DREF DeeObject **p_result, *result;
	p_result = thread_tls_get(self->t_index);
	if unlikely(!p_result)
		goto err;
	result = *p_result;
	if unlikely(result == ITER_DONE) {
		err_tls_unbound();
		goto err;
	}
	if (result) {
		Dee_Incref(result);
	} else if (!self->t_factory) {
		err_tls_unbound();
		goto err;
	} else if (DeeNone_Check(self->t_factory)) {
		Dee_Incref_n(&DeeNone_Singleton, 2);
		result = *p_result = Dee_AsObject(&DeeNone_Singleton); /* Save and return `none'. */
	} else {
		/* Invoke the factory. */
		result = DeeObject_Call(self->t_factory, 0, NULL);
		if unlikely(!result)
			goto err;

		/* Must re-retrieve the TLS pointer in case the factory
		 * did some other TLS manipulations that changed the vector. */
		p_result = thread_tls_get(self->t_index);
		ASSERTF(p_result, "Since we've already allocated this index "
		                 "before, it should have already existed");
		if unlikely(*p_result) {
			/* Highly unlikely: The factory called the TLS recursively
			 *                  but actually returned a value in some
			 *                  inner callback.
			 *                  As a result of that, we now have 2 candidates
			 *                  for the TLS value. However in the interest
			 *                  of keeping things consistent, don't overwrite
			 *                  the existing value, but drop the new and
			 *                  re-use the existing one. */
			DREF DeeObject *new_result = *p_result;

			/* Extract the existing value first, in case the decref()
			 * on the factory return value changes it again... */
			Dee_Incref(new_result);
			COMPILER_READ_BARRIER();
			Dee_Decref(result);
			result = new_result; /* Inherit reference. */
		} else {
			/* Save the factory return value in the TLS variable slot. */
			Dee_Incref(result);
			*p_result = result; /* Inherit reference. */
		}
	}
	return result;
err:
	return NULL;
}

/* @return:  1: The TLS variable was previously unbound.
 * @return:  0: Successfully unbound the TLS variable.
 * @return: -1: An error occurred. */
PRIVATE WUNUSED NONNULL((1)) int DCALL
tls_dodelitem(TLS *__restrict self) {
	DREF DeeObject **pitem, *item;
	pitem = thread_tls_get(self->t_index);
	if unlikely(!pitem)
		goto err;
again:
	item = *pitem; /* Inherit */

	/* Check if the variable had been manually unbound. */
	if unlikely(item == ITER_DONE)
		return 1;
	if (!item) {
		/* The variable had never been assigned. */
		if (self->t_factory &&
		    !DeeNone_Check(self->t_factory)) {

			/* Must still invoke the factory to remain consistent. */
			item = DeeObject_Call(self->t_factory, 0, NULL);
			if unlikely(!item)
				goto err;

			/* Mark the variable as unbound. */
			pitem = thread_tls_get(self->t_index);

			/* Check if the factory tinkered with the TLS variable. */
			if unlikely(*pitem != NULL) {
				Dee_Decref(item);
				goto again;
			}
			*pitem = ITER_DONE;
			COMPILER_BARRIER();
			Dee_Decref(item);
		} else {
			/* Mark the variable as unbound. */
			*pitem = ITER_DONE;
		}
	} else {
		/* Mark the variable as unbound. */
		*pitem = ITER_DONE;
		COMPILER_WRITE_BARRIER();
		Dee_Decref(item);
	}
/*done:*/
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
tls_delvalue(TLS *__restrict self) {
	int result = tls_dodelitem(self);
	if (result > 0)
		result = 0;
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
tls_setvalue(TLS *self, DeeObject *value) {
	DREF DeeObject **pitem, *item;
	pitem = thread_tls_get(self->t_index);
	if unlikely(!pitem)
		goto err;
	Dee_Incref(value);
	item   = *pitem; /* Inherit */
	*pitem = value;  /* Inherit */
	Dee_Decref(item);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tls_xchitem(TLS *self, DeeObject *value) {
	DREF DeeObject **pitem, *result;
again:
	pitem = thread_tls_get(self->t_index);
	if unlikely(!pitem)
		goto err;
	result = *pitem; /* Inherit */
	if unlikely(result == ITER_DONE) {
		/* The slot has been unbound and there's nothing we could return. */
		err_tls_unbound();
		goto err;
	}
	if (!result) {
		/* Cheat a bit by letting `tls_getvalue()' deal with the factory call. */
		result = tls_getvalue(self);
		if unlikely(!result)
			goto err;
		Dee_Decref(result);
		goto again;
	}
	if (value != ITER_DONE)
		Dee_Incref(value);
	*pitem = value; /* Inherit */

	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
tls_bool(TLS *__restrict self) {
	DREF DeeObject **slot;
	slot = thread_tls_tryget(self->t_index);
	return slot != NULL && ITER_ISOK(*slot);
}

#else /* !CONFIG_NO_THREADS */

typedef struct {
	OBJECT_HEAD
	DREF DeeObject *t_factory; /* [0..1][const] A factory function used to construct TLS default value. */
	DREF DeeObject *t_value;   /* [0..1] The stored TLS value. */
} TLS;

PRIVATE WUNUSED NONNULL((1)) int DCALL
tls_init(TLS *__restrict self,
         size_t argc, DeeObject *const *argv) {
	self->t_value   = NULL;
	self->t_factory = NULL;
	DeeArg_Unpack0Or1(err, argc, argv, "TLS", &self->t_factory);
	/* Save a reference for the factory. */
	Dee_XIncref(self->t_factory);
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
tls_fini(TLS *__restrict self) {
	if (ITER_ISOK(self->t_value))
		Dee_Decref(self->t_value);
	Dee_XDecref(self->t_factory);
}

PRIVATE NONNULL((1, 2)) void DCALL
tls_visit(TLS *__restrict self, Dee_visit_t proc, void *arg) {
	if (ITER_ISOK(self->t_value))
		Dee_Visit(self->t_value);
	Dee_XVisit(self->t_factory);
}


PRIVATE ATTR_COLD int DCALL err_tls_unbound(void) {
	return DeeError_Throwf(&DeeError_AttributeError,
	                       "The TLS variable has been unbound");
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
tls_getvalue(TLS *__restrict self) {
	DREF DeeObject *result;
	result = self->t_value;
	if unlikely(result == ITER_DONE) {
		err_tls_unbound();
		goto err;
	}
	if (result) {
		Dee_Incref(result);
	} else if (!self->t_factory) {
		err_tls_unbound();
		goto err;
	} else if (DeeNone_Check(self->t_factory)) {
		Dee_None->ob_refcnt += 2;
		result = self->t_value = Dee_None; /* Save and return `none'. */
	} else {
		/* Invoke the factory. */
		result = DeeObject_Call(self->t_factory, 0, NULL);
		if unlikely(!result)
			goto err;
		/* Must re-retrieve the TLS pointer in case the factory
		 * did some other TLS manipulations that changed the vector. */
		if unlikely(self->t_value) {
			/* Highly unlikely: The factory called the TLS recursively
			 *                  but actually returned a value in some
			 *                  inner callback.
			 *                  As a result of that, we now have 2 candidates
			 *                  for the TLS value. However in the interest
			 *                  of keeping things consistent, don't overwrite
			 *                  the existing value, but drop the new and
			 *                  re-use the existing one. */
			DREF DeeObject *new_result = self->t_value;
			/* Extract the existing value first, in case the decref()
			 * on the factory return value changes it again... */
			Dee_Incref(new_result);
			COMPILER_READ_BARRIER();
			Dee_Decref(result);
			result = new_result; /* Inherit reference. */
		} else {
			/* Save the factory return value in the TLS variable slot. */
			Dee_Incref(result);
			self->t_value = result; /* Inherit reference. */
		}
	}
	return result;
err:
	return NULL;
}

/* @return:  1: The TLS variable was previously unbound.
 * @return:  0: Successfully unbound the TLS variable.
 * @return: -1: An error occurred. */
PRIVATE WUNUSED NONNULL((1)) int DCALL
tls_dodelitem(TLS *__restrict self) {
	DREF DeeObject *item;
again:
	item = self->t_value; /* Inherit */
	/* Check if the variable had been manually unbound. */
	if unlikely(item == ITER_DONE)
		return 1;
	if (!item) {
		/* The variable had never been assigned. */
		if (self->t_factory &&
		    !DeeNone_Check(self->t_factory)) {
			/* Must still invoke the factory to remain consistent. */
			item = DeeObject_Call(self->t_factory, 0, NULL);
			if unlikely(!item)
				goto err;
			/* Check if the factory tinkered with the TLS variable. */
			if unlikely(self->t_value != NULL) {
				Dee_Decref(item);
				goto again;
			}
			self->t_value = ITER_DONE;
			COMPILER_BARRIER();
			Dee_Decref(item);
		} else {
			/* Mark the variable as unbound. */
			self->t_value = ITER_DONE;
		}
	} else {
		/* Mark the variable as unbound. */
		self->t_value = ITER_DONE;
		COMPILER_WRITE_BARRIER();
		Dee_Decref(item);
	}
/*done:*/
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
tls_delvalue(TLS *__restrict self) {
	int result = tls_dodelitem(self);
	if (result > 0)
		result = 0;
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
tls_setvalue(TLS *self, DeeObject *value) {
	DREF DeeObject *item;
	Dee_Incref(value);
	item          = self->t_value; /* Inherit */
	self->t_value = value;         /* Inherit */
	Dee_Decref(item);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tls_xchitem(TLS *self, DeeObject *value) {
	DREF DeeObject *result;
again:
	result = self->t_value; /* Inherit */
	if unlikely(result == ITER_DONE) {
		/* The slot has been unbound and there's nothing we could return. */
		err_tls_unbound();
		goto err;
	}
	if (!result) {
		/* Cheat a bit by letting `tls_getvalue()' deal with the factory call. */
		result = tls_getvalue(self);
		if unlikely(!result)
			goto err;
		Dee_Decref(result);
		goto again;
	}
	if (value != ITER_DONE)
		Dee_Incref(value);
	self->t_value = value; /* Inherit */
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
tls_hash(TLS *__restrict self) {
	return DeeObject_HashGeneric(self);
}
#define DEFINE_TLS_COMPARE(name, op)                      \
	PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL \
	name(TLS *self, TLS *other) {                         \
		if (DeeObject_AssertType(other, &DeeTLS_Type))    \
			goto err;                                     \
		return_bool(self op other);                      \
	err:                                                  \
		return NULL;                                      \
	}
DEFINE_TLS_COMPARE(tls_eq, ==)
DEFINE_TLS_COMPARE(tls_ne, !=)
DEFINE_TLS_COMPARE(tls_lo, <)
DEFINE_TLS_COMPARE(tls_le, <=)
DEFINE_TLS_COMPARE(tls_gr, >)
DEFINE_TLS_COMPARE(tls_ge, >=)
#undef DEFINE_TLS_COMPARE

PRIVATE WUNUSED NONNULL((1)) int DCALL tls_bool(TLS *__restrict self) {
	return ITER_ISOK(self->t_value);
}

#endif /* CONFIG_NO_THREADS */

PRIVATE WUNUSED NONNULL((1)) int DCALL
tls_bound(TLS *__restrict self) {
	int ok = tls_bool(self);
	ASSERT(ok == 0 || ok == 1);
	return Dee_BOUND_FROMBOOL(ok);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
tls_printrepr(TLS *__restrict self, Dee_formatprinter_t printer, void *arg) {
	DeeObject *factory = self->t_factory;
	return factory ? DeeFormat_Printf(printer, arg, "TLS(%r)", factory)
	               : DeeFormat_PRINT(printer, arg, "TLS()");
}



PRIVATE struct type_getset tpconst tls_getsets[] = {
	TYPE_GETSET_BOUND_F("value", &tls_getvalue, &tls_delvalue, &tls_setvalue, &tls_bound, METHOD_FNOREFESCAPE,
	                    "#tAttributeError{The TLS variable isn't bound}"
	                    "Read/write access to the object assigned to this TLS variable slot in the calling thread\n"
	                    "If a factory has been defined, it will be invoked upon first access, unless that access is setting the TLS value.\n"
	                    "If no factory has been defined, the TLS is initialized as unbound and attempting "
	                    /**/ "to read it will cause an :AttributeError to be thrown"),
	TYPE_GETSET_END
};


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
tls_xch(TLS *self, size_t argc, DeeObject *const *argv) {
	DeeObject *newval;
	DeeArg_Unpack1(err, argc, argv, "xch", &newval);
	return tls_xchitem(self, newval);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
tls_pop(TLS *self, size_t argc, DeeObject *const *argv) {
	DeeArg_Unpack0(err, argc, argv, "pop");
	return tls_xchitem(self, ITER_DONE);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
tls_get(TLS *self, size_t argc, DeeObject *const *argv) {
	DeeArg_Unpack0(err, argc, argv, "get");
	return tls_getvalue(self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
tls_delete(TLS *self, size_t argc, DeeObject *const *argv) {
	int result;
	DeeArg_Unpack0(err, argc, argv, "delete");
	result = tls_dodelitem(self);
	if unlikely(result < 0)
		goto err;
	return_bool(result == 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
tls_set(TLS *self, size_t argc, DeeObject *const *argv) {
	DeeObject *ob;
	DeeArg_Unpack1(err, argc, argv, "set", &ob);
	if (tls_setvalue(self, ob))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE struct type_method tpconst tls_methods[] = {
	TYPE_METHOD_F("get", &tls_get, METHOD_FNOREFESCAPE,
	              "->\n"
	              "#tUnboundAttribute{The TLS variable isn't bound}"
	              "Return the stored object. Same as ${this.item}"),
	TYPE_METHOD_F("delete", &tls_delete, METHOD_FNOREFESCAPE,
	              "->?Dbool\n"
	              "Unbind the TLS variable slot, returning ?f if "
	              /**/ "it had already been unbound and ?t otherwise"),
	TYPE_METHOD_F("set", &tls_set, METHOD_FNOREFESCAPE,
	              "(ob)\n"
	              "Set the TLS variable. Same as ${this.item = ob}"),
	TYPE_METHOD_F("xch", &tls_xch, METHOD_FNOREFESCAPE,
	              "(ob)->\n"
	              "#tAttributeError{The TLS variable had already been unbound}"
	              "Exchange the stored TLS value with @ob and return the old value"),
	TYPE_METHOD_F("pop", &tls_pop, METHOD_FNOREFESCAPE,
	              "->\n"
	              "#tAttributeError{The TLS variable had already been unbound}"
	              "Unbind the stored TLS object and return the previously stored object"),

	/* Deprecated functions. */
	TYPE_METHOD_F("exchange", &tls_xch, METHOD_FNOREFESCAPE,
	              "(ob)->\n"
	              "Deprecated alias for ?#xch"),
	TYPE_METHOD_END
};

INTERN DeeTypeObject DeeTLS_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "TLS",
	/* .tp_doc      = */ DOC("Thread-Local-Storage container. Instance of ?. objects behave similar to ?DCell, "
	                         /**/ "except that every thread has its own, private instance of a bound ?#value. "
	                         /**/ "You can also specify a $factory function in the constructor that will be "
	                         /**/ "used in order to produce the initially bound ?#value in new threads.\n"
	                         "\n"
	                         "()\n"
	                         "(factory:?DCallable)\n"
	                         "Construct a new tls descriptor using an optional @factory that "
	                         /**/ "is used to construct the default values of per-thread variables\n"
	                         "You may pass ?N for @factory to pre-initialize the TLS value to ?N\n"
	                         "When given, @factory is invoked as ${factory()} upon first access on a "
	                         /**/ "per-thread basis, using its return value as initial value for the TLS\n"
	                         "\n"

	                         "bool->\n"
	                         "Returns ?t if the TLS variable has been bound in the calling thread"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ TLS,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ NULL,
			/* tp_any_ctor:    */ &tls_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL /* Can't be serialized */
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&tls_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ (int (DCALL *)(DeeObject *__restrict))&tls_bool,
		/* .tp_print     = */ NULL,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&tls_printrepr,
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&tls_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &DeeObject_GenericCmpByAddr,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ tls_methods,
	/* .tp_getsets       = */ tls_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};


DECL_END


#endif /* !GUARD_DEX_THREADING_TLS_C */
