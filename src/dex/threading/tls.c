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
#ifndef GUARD_DEX_THREADING_TLS_C
#define GUARD_DEX_THREADING_TLS_C 1
#define CONFIG_BUILDING_LIBTHREADING
#define DEE_SOURCE

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/dex.h>
#include <deemon/error.h>
#include <deemon/none.h>
#include <deemon/system-features.h> /* bzeroc(), ... */
#include <deemon/thread.h>
#include <deemon/util/lock.h>

#include "libthreading.h"

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
	DeeThreadObject *caller = DeeThread_Self();
	desc                    = (struct tls_descriptor *)caller->t_tlsdata;
	if unlikely(!desc || index >= desc->td_size) {
		size_t old_size = desc ? desc->td_size : 0;
		desc = (struct tls_descriptor *)Dee_Realloc(desc,
		                                            offsetof(struct tls_descriptor, td_elem) +
		                                            (index + 1) * sizeof(DREF DeeObject *));
		if unlikely(!desc)
			goto err;
		/* Save the new descriptor length. */
		desc->td_size = index + 1;
		/* ZERO-initialize all newly allocated indices. */
		bzeroc(desc->td_elem + old_size,
		       (index + 1) - old_size,
		       sizeof(DREF DeeObject *));
		/* Save the new descriptor. */
		caller->t_tlsdata = (void *)desc;
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
	desc   = (struct tls_descriptor *)caller->t_tlsdata;
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

INTERN NONNULL((1, 2)) void DCALL
thread_tls_visit(struct tls_descriptor *__restrict data,
                 dvisit_t proc, void *arg) {
	size_t i, count = data->td_size;
	/* Visit all allocated objects. */
	for (i = 0; i < count; ++i) {
		DREF DeeObject *ob = data->td_elem[i];
		if (ITER_ISOK(ob))
			Dee_Visit(ob);
	}
}


/* Lock for registering TLS objects. */
PRIVATE atomic_lock_t tls_reglock = ATOMIC_LOCK_INIT;
PRIVATE uint8_t      *tls_inuse   = NULL; /* [lock(tls_reglock)][0..(tls_nexti+7)/8][owned] Bitset of all TLS indices currently in use. */
PRIVATE size_t        tls_nexti   = 0;    /* [lock(tls_reglock)] The next TLS index used when all others are already in use. */
PRIVATE size_t        tls_count   = 0;    /* [lock(tls_reglock)] The total number of TLS indices currently assigned. */

/* Allocate a new TLS index.
 * @return: * :         The newly allocated TLS index.
 * @return: (size_t)-1: Not enough available memory. */
PRIVATE WUNUSED size_t DCALL tls_alloc(void);
/* Free a previously allocated TLS index. */
PRIVATE void DCALL tls_free(size_t index);





PRIVATE WUNUSED size_t DCALL tls_alloc(void) {
	size_t result;
again:
	atomic_lock_acquire(&tls_reglock);
	ASSERTF(tls_nexti >= tls_count, "Inconsistent Tls state");
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
			atomic_lock_release(&tls_reglock);
			/* Try to collect some memory. */
			if (Dee_CollectMemory(((tls_nexti / 8) + 1) * sizeof(uint8_t)))
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
	atomic_lock_release(&tls_reglock);
	return result;
}

PRIVATE void DCALL tls_free(size_t index) {
	atomic_lock_acquire(&tls_reglock);
	ASSERTF(tls_count, "No indices allocated");
	ASSERTF(tls_nexti >= tls_count, "Inconsistent Tls state");
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
	atomic_lock_release(&tls_reglock);
}




typedef struct {
	OBJECT_HEAD
	DREF DeeObject *t_factory; /* [0..1][const] A factory function used to construct TLS default value. */
	size_t t_index;            /* [owned(tls_free)][const] The per-thread TLS index used by this controller. */
} Tls;

PRIVATE WUNUSED NONNULL((1)) int DCALL
tls_init(Tls *__restrict self,
         size_t argc, DeeObject *const *argv) {
	self->t_factory = NULL;
	if (DeeArg_Unpack(argc, argv, "|o:Tls", &self->t_factory))
		goto err;
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
tls_fini(Tls *__restrict self) {
	/* Free the allocated TLS index. */
	tls_free(self->t_index);
	/* Destroy the associated factory. */
	Dee_XDecref(self->t_factory);
}

PRIVATE NONNULL((1, 2)) void DCALL
tls_visit(Tls *__restrict self, dvisit_t proc, void *arg) {
	Dee_XVisit(self->t_factory);
}


PRIVATE ATTR_COLD int DCALL err_tls_unbound(void) {
	return DeeError_Throwf(&DeeError_UnboundAttribute,
	                       "The Tls variable has been unbound");
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
tls_getvalue(Tls *__restrict self) {
	DREF DeeObject **presult, *result;
	presult = thread_tls_get(self->t_index);
	if unlikely(!presult)
		goto err;
	result = *presult;
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
		Dee_Incref_n(Dee_None, 2);
		result = *presult = Dee_None; /* Save and return `none'. */
	} else {
		/* Invoke the factory. */
		result = DeeObject_Call(self->t_factory, 0, NULL);
		if unlikely(!result)
			goto err;
		/* Must re-retrieve the TLS pointer in case the factory
		 * did some other TLS manipulations that changed the vector. */
		presult = thread_tls_get(self->t_index);
		ASSERTF(presult, "Since we've already allocated this index "
		                 "before, it should have already existed");
		if unlikely(*presult) {
			/* Highly unlikely: The factory called the TLS recursively
			 *                  but actually returned a value in some
			 *                  inner callback.
			 *                  As a result of that, we now have 2 candidates
			 *                  for the TLS value. However in the interest
			 *                  of keeping things consistent, don't overwrite
			 *                  the existing value, but drop the new and
			 *                  re-use the existing one. */
			DREF DeeObject *new_result = *presult;
			/* Extract the existing value first, in case the decref()
			 * on the factory return value changes it again... */
			Dee_Incref(new_result);
			COMPILER_READ_BARRIER();
			Dee_Decref(result);
			result = new_result; /* Inherit reference. */
		} else {
			/* Save the factory return value in the TLS variable slot. */
			Dee_Incref(result);
			*presult = result; /* Inherit reference. */
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
tls_dodelitem(Tls *__restrict self) {
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
			;
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
tls_delvalue(Tls *__restrict self) {
	int result = tls_dodelitem(self);
	if (result > 0)
		result = err_tls_unbound();
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
tls_setvalue(Tls *self, DeeObject *value) {
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
tls_xchitem(Tls *self, DeeObject *value) {
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

PRIVATE WUNUSED NONNULL((1)) dhash_t DCALL
tls_hash(Tls *__restrict self) {
	/* Since TLS indices are unique, they're perfect for hasing. */
	return (dhash_t)self->t_index;
}

#define DEFINE_TLS_COMPARE(name, op)                      \
	PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL \
	name(Tls *self, Tls *other) {                         \
		if (DeeObject_AssertType(other, &DeeTls_Type))    \
			return NULL;                                  \
		return_bool_(self->t_index op other->t_index);    \
	}
DEFINE_TLS_COMPARE(tls_eq, ==)
DEFINE_TLS_COMPARE(tls_ne, !=)
DEFINE_TLS_COMPARE(tls_lo, <)
DEFINE_TLS_COMPARE(tls_le, <=)
DEFINE_TLS_COMPARE(tls_gr, >)
DEFINE_TLS_COMPARE(tls_ge, >=)
#undef DEFINE_TLS_COMPARE

PRIVATE WUNUSED NONNULL((1)) int DCALL tls_bool(Tls *__restrict self) {
	DREF DeeObject **slot;
	slot = thread_tls_tryget(self->t_index);
	return slot != NULL && ITER_ISOK(*slot);
}

#else /* !CONFIG_NO_THREADS */

typedef struct {
	OBJECT_HEAD
	DREF DeeObject *t_factory; /* [0..1][const] A factory function used to construct TLS default value. */
	DREF DeeObject *t_value;   /* [0..1] The stored TLS value. */
} Tls;

PRIVATE WUNUSED NONNULL((1)) int DCALL
tls_init(Tls *__restrict self,
         size_t argc, DeeObject *const *argv) {
	self->t_value   = NULL;
	self->t_factory = NULL;
	if (DeeArg_Unpack(argc, argv, "|o:Tls", &self->t_factory))
		goto err;
	/* Save a reference for the factory. */
	Dee_XIncref(self->t_factory);
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
tls_fini(Tls *__restrict self) {
	if (ITER_ISOK(self->t_value))
		Dee_Decref(self->t_value);
	Dee_XDecref(self->t_factory);
}

PRIVATE NONNULL((1, 2)) void DCALL
tls_visit(Tls *__restrict self, dvisit_t proc, void *arg) {
	if (ITER_ISOK(self->t_value))
		Dee_Visit(self->t_value);
	Dee_XVisit(self->t_factory);
}


PRIVATE ATTR_COLD int DCALL err_tls_unbound(void) {
	return DeeError_Throwf(&DeeError_AttributeError,
	                       "The TLS variable has been unbound");
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
tls_getvalue(Tls *__restrict self) {
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
tls_dodelitem(Tls *__restrict self) {
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
tls_delvalue(Tls *__restrict self) {
	int result = tls_dodelitem(self);
	if (result > 0)
		result = err_tls_unbound();
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
tls_setvalue(Tls *self, DeeObject *value) {
	DREF DeeObject *item;
	Dee_Incref(value);
	item          = self->t_value; /* Inherit */
	self->t_value = value;         /* Inherit */
	Dee_Decref(item);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
tls_xchitem(Tls *self, DeeObject *value) {
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

PRIVATE WUNUSED NONNULL((1)) dhash_t DCALL
tls_hash(Tls *__restrict self) {
	return DeeObject_HashGeneric(self);
}
#define DEFINE_TLS_COMPARE(name, op)                      \
	PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL \
	name(Tls *self, Tls *other) {                         \
		if (DeeObject_AssertType(other, &DeeTls_Type))    \
			return NULL;                                  \
		return_bool_(self op other);                      \
	}
DEFINE_TLS_COMPARE(tls_eq, ==)
DEFINE_TLS_COMPARE(tls_ne, !=)
DEFINE_TLS_COMPARE(tls_lo, <)
DEFINE_TLS_COMPARE(tls_le, <=)
DEFINE_TLS_COMPARE(tls_gr, >)
DEFINE_TLS_COMPARE(tls_ge, >=)
#undef DEFINE_TLS_COMPARE

PRIVATE WUNUSED NONNULL((1)) int DCALL tls_bool(Tls *__restrict self) {
	return ITER_ISOK(self->t_value);
}

#endif /* CONFIG_NO_THREADS */


PRIVATE struct type_getset tpconst tls_getsets[] = {
	TYPE_GETSET("value", &tls_getvalue, &tls_delvalue, &tls_setvalue,
	            "@throw AttributeError The Tls variable isn't bound, or has already been unbound\n"
	            "Read/write access to the object assigned to this Tls variable slot in the calling thread\n"
	            "If a factory has been defined, it will be invoked upon first access, unless that access is setting the Tls value.\n"
	            "If no factory has been defined, the Tls is initialized as unbound and any attempt "
	            /**/ "to read or delete it will cause an :AttributeError to be thrown"),
	TYPE_GETSET_END
};


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
tls_xch(Tls *self, size_t argc, DeeObject *const *argv) {
	DeeObject *newval;
	if (DeeArg_Unpack(argc, argv, "o:xch", &newval))
		goto err;
	return tls_xchitem(self, newval);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
tls_pop(Tls *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":pop"))
		goto err;
	return tls_xchitem(self, ITER_DONE);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
tls_get(Tls *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":get"))
		goto err;
	return tls_getvalue(self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
tls_delete(Tls *self, size_t argc, DeeObject *const *argv) {
	int result;
	if (DeeArg_Unpack(argc, argv, ":delete"))
		goto err;
	result = tls_dodelitem(self);
	if unlikely(result < 0)
		goto err;
	return_bool_(result == 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
tls_set(Tls *self, size_t argc, DeeObject *const *argv) {
	DeeObject *ob;
	if (DeeArg_Unpack(argc, argv, "o:set", &ob))
		goto err;
	if (tls_setvalue(self, ob))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE struct type_method tpconst tls_methods[] = {
	TYPE_METHOD("get", &tls_get,
	            "->\n"
	            "@throw UnboundAttribute The Tls variable isn't bound\n"
	            "Return the stored object. Same as ${this.item}"),
	TYPE_METHOD("delete", &tls_delete,
	            "->?Dbool\n"
	            "Unbind the Tls variable slot, returning ?f if "
	            "it had already been unbound and ?t otherwise"),
	TYPE_METHOD("set", &tls_set,
	            "(ob)\n"
	            "Set the Tls variable. Same as ${this.item = ob}"),
	TYPE_METHOD("xch", &tls_xch,
	            "(ob)->\n"
	            "@throw AttributeError The Tls variable had already been unbound\n"
	            "Exchange the stored Tls value with @ob and return the old value"),
	TYPE_METHOD("pop", &tls_pop,
	            "->\n"
	            "@throw AttributeError The Tls variable had already been unbound\n"
	            "Unbind the stored Tls object and return the previously stored object"),

	/* Deprecated functions. */
	TYPE_METHOD("exchange", &tls_xch,
	            "(ob)->\n"
	            "Deprecated alias for ?#xch"),
	TYPE_METHOD_END
};

PRIVATE struct type_cmp tls_cmp = {
	/* .tp_hash = */ (dhash_t (DCALL *)(DeeObject *__restrict))&tls_hash,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&tls_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&tls_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&tls_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&tls_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&tls_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&tls_ge,
};

INTERN DeeTypeObject DeeTls_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "Tls",
	/* .tp_doc      = */ DOC("()\n"
	                         "(factory:?DCallable)\n"
	                         "Construct a new tls descriptor using an optional @factory that "
	                         /**/ "is used to construct the default values of per-thread variables\n"
	                         "You may pass ?N for @factory to pre-initialize the Tls value to ?N\n"
	                         "When given, @factory is invoked as ${factory()} upon first access on a "
	                         /**/ "per-thread basis, using its return value as initial value for the Tls\n"
	                         "\n"

	                         "bool->\n"
	                         "Returns ?t if the Tls variable has been bound in the calling thread"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&tls_init,
				TYPE_FIXED_ALLOCATOR(Tls)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&tls_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&tls_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&tls_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &tls_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
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
