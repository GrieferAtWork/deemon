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
#ifndef GUARD_DEEMON_ACCU_H
#define GUARD_DEEMON_ACCU_H 1

#include "api.h"
/**/

#include "object.h"
/**/

#include "bytes.h"  /* struct Dee_bytes_printer */
#include "string.h" /* struct Dee_unicode_printer */
#include "tuple.h"  /* struct Dee_tuple_builder */
/**/

#include <stdint.h> /* uintptr_t */

DECL_BEGIN

#ifdef DEE_SOURCE
#define Dee_list_object  list_object
#endif /* DEE_SOURCE */

struct Dee_list_object;

/* Accumulator modes. */
enum {
	Dee_ACCU_FIRST,   /* Mode not yet determined (expecting first item) */
	Dee_ACCU_SECOND,  /* Always comes after `Dee_ACCU_FIRST' and selects which mode to use */
	Dee_ACCU_OBJECT,  /* Generic object sum mode (using "operator +") */
	Dee_ACCU_NONE,    /* First object was `none', so all other objects are ignored and result will also be `none' */
	Dee_ACCU_STRING,  /* Use a unicode printer */
	Dee_ACCU_BYTES,   /* Use a bytes printer */
	Dee_ACCU_INT,     /* First object was "int" */
#if __SIZEOF_SIZE_T__ < 8
	Dee_ACCU_INT64,   /* First object was "int" (and didn't fit into a Dee_ssize_t) */
#define HAVE_Dee_ACCU_INT64
#endif /* __SIZEOF_SIZE_T__ < 8 */
#ifndef CONFIG_NO_FPU
	Dee_ACCU_FLOAT,   /* First object was "float" */
#define HAVE_Dee_ACCU_FLOAT
#endif /* !CONFIG_NO_FPU */
	Dee_ACCU_TUPLE,   /* First object was "Tuple" */
	Dee_ACCU_LIST,    /* First object was "List" (!DeeObject_IsShared) */
//TODO:	Dee_ACCU_SEQ,     /* First object's `operator +' was identical to `Sequence.__add__' (SeqConcat_Type) */
//TODO:	Dee_ACCU_SET,     /* First object's `operator +' was identical to `Set.__add__'      (SetUnion_Type) */
//TODO:	Dee_ACCU_MAP,     /* First object's `operator +' was identical to `Mapping.__add__'  (MapUnion_Type) */
};

/* Helper structure for optimized, but still type-generic "operator +" accumulation. */
struct Dee_accu {
	uintptr_t acu_mode; /* Accumulator-mode (one of `Dee_ACCU_*') */
	union {
		DREF DeeObject              *v_object; /* Dee_ACCU_SECOND, Dee_ACCU_OBJECT */
		struct Dee_unicode_printer   v_string; /* Dee_ACCU_STRING */
		struct Dee_bytes_printer     v_bytes;  /* Dee_ACCU_BYTES */
		Dee_ssize_t                  v_int;    /* Dee_ACCU_INT */
#if __SIZEOF_SIZE_T__ < 8
		int64_t                      v_int64;  /* Dee_ACCU_INT64 */
#endif /* __SIZEOF_SIZE_T__ < 8 */
#ifndef CONFIG_NO_FPU
		double                       v_float;  /* Dee_ACCU_FLOAT */
#endif /* !CONFIG_NO_FPU */
		struct Dee_tuple_builder     v_tuple;  /* Dee_ACCU_TUPLE */
		DREF struct Dee_list_object *v_list;   /* Dee_ACCU_LIST */
	} acu_value;
};

/* Initialize the accumulator */
#define Dee_accu_init(self) (void)((self)->acu_mode = Dee_ACCU_FIRST)
#define Dee_accu_init_with_first_inherited(self, first)  \
	(void)((self)->acu_mode           = Dee_ACCU_SECOND, \
	       (self)->acu_value.v_object = (first))
#define Dee_accu_init_with_first(self, first)         \
	(Dee_accu_init_with_first_inherited(self, first), \
	 Dee_Incref((self)->acu_value.v_object))

/* Finalize the accumulator and dispose any previously calculated results. */
DFUNDEF NONNULL((1)) void DCALL
Dee_accu_fini(struct Dee_accu *__restrict self);

/* Pack the accumulator and return its final result as an object.
 * This function may only be called once, as it does an implicit
 * `Dee_accu_fini()'. Returns `NULL' if an error was thrown.
 *
 * Hint: if you want `self' to remain valid, you can just re-init it
 *       after the call using `Dee_accu_init_with_first_inherited()' */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
Dee_accu_pack(struct Dee_accu *__restrict self);

/* Add `item' into the accumulator.
 * HINT: This function is `Dee_foreach_t'-compatible. */
DFUNDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
Dee_accu_add(/*struct Dee_accu*/ void *self, DeeObject *item);

/* Add all elements of `items' into the accumulator. */
DFUNDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
Dee_accu_addall(/*struct Dee_accu*/ void *self, DeeObject *items);


DECL_END

#endif /* !GUARD_DEEMON_ACCU_H */
