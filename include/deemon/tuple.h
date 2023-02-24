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
#ifndef GUARD_DEEMON_TUPLE_H
#define GUARD_DEEMON_TUPLE_H 1

#include "api.h"

#include <stdarg.h>
#include <stddef.h>

#include "object.h"

DECL_BEGIN

#ifdef DEE_SOURCE
#define Dee_tuple_object   tuple_object
#define DEFINE_TUPLE       Dee_DEFINE_TUPLE
#define return_empty_tuple Dee_return_empty_tuple
#endif /* DEE_SOURCE */

typedef struct Dee_tuple_object DeeTupleObject;

struct Dee_tuple_object {
	/* WARNING: Changes must be mirrored in `/src/deemon/execute/asm/exec.gas-386.S' */
	Dee_OBJECT_HEAD
	size_t                                    t_size;  /* [const] Tuple size. */
	COMPILER_FLEXIBLE_ARRAY(DREF DeeObject *, t_elem); /* [1..1][const][t_size] Tuple elements. */
};

#define DeeTuple_SIZEOF(n_items) (COMPILER_OFFSETOF(DeeTupleObject, t_elem) + (n_items) * sizeof(DREF DeeObject *))
#define DeeTuple_IsEmpty(ob)     ((DeeObject *)Dee_REQUIRES_OBJECT(ob) == Dee_EmptyTuple)
#define DeeTuple_SIZE(ob)        ((DeeTupleObject *)Dee_REQUIRES_OBJECT(ob))->t_size
#define DeeTuple_ELEM(ob)        ((DeeTupleObject *)Dee_REQUIRES_OBJECT(ob))->t_elem
#define DeeTuple_END(ob)         (((DeeTupleObject *)Dee_REQUIRES_OBJECT(ob))->t_elem + ((DeeTupleObject *)(ob))->t_size)
#define DeeTuple_GET(ob, i)      ((DeeTupleObject *)Dee_REQUIRES_OBJECT(ob))->t_elem[i]
#define DeeTuple_SET(ob, i, v)   ((DeeTupleObject *)Dee_REQUIRES_OBJECT(ob))->t_elem[i] = (v)

/* Define a statically allocated tuple:
 * >> PRIVATE DREF DeeObject *DCALL get_my_tuple(void) {
 * >>     PRIVATE DEFINE_TUPLE(my_tuple, 2, { Dee_EmptyString, Dee_EmptyString });
 * >>     return_reference((DeeObject *)&my_tuple);
 * >> }
 */
#define Dee_DEFINE_TUPLE(name, elemc, ...)    \
	struct {                                  \
		Dee_OBJECT_HEAD                       \
		size_t     t_size;                    \
		DeeObject *t_elem[elemc];             \
	} name = {                                \
		Dee_OBJECT_HEAD_INIT(&DeeTuple_Type), \
		elemc,                                \
		__VA_ARGS__                           \
	}


#ifdef GUARD_DEEMON_OBJECTS_TUPLE_C
struct empty_tuple_object {
	Dee_OBJECT_HEAD
	size_t t_size;
};
DDATDEF struct empty_tuple_object DeeTuple_Empty;
#define Dee_EmptyTuple ((DeeObject *)&DeeTuple_Empty)
#else /* GUARD_DEEMON_OBJECTS_TUPLE_C */
DDATDEF DeeObject        DeeTuple_Empty;
#define Dee_EmptyTuple (&DeeTuple_Empty)
#endif /* !GUARD_DEEMON_OBJECTS_TUPLE_C */
#define Dee_return_empty_tuple  Dee_return_reference_(Dee_EmptyTuple)

DDATDEF DeeTypeObject DeeTuple_Type;
#define DeeTuple_Check(x)       DeeObject_InstanceOfExact(x, &DeeTuple_Type) /* `Tuple' is final */
#define DeeTuple_CheckExact(x)  DeeObject_InstanceOfExact(x, &DeeTuple_Type)


/* Create new tuple objects. */
DFUNDEF WUNUSED DREF DeeObject *DCALL
DeeTuple_NewUninitialized(size_t n);

/* Same as `DeeTuple_NewUninitialized()', but
 * doesn't throw an exception when returning `NULL' */
DFUNDEF WUNUSED DREF DeeObject *DCALL
DeeTuple_TryNewUninitialized(size_t n);

DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeTuple_ResizeUninitialized(/*inherit(on_success)*/ DREF DeeObject *__restrict self, size_t n);

DFUNDEF WUNUSED ATTR_RETNONNULL NONNULL((1)) DREF DeeObject *DCALL
DeeTuple_TruncateUninitialized(/*inherit(always)*/ DREF DeeObject *__restrict self, size_t n);

DFUNDEF NONNULL((1)) void DCALL
DeeTuple_FreeUninitialized(DREF DeeObject *__restrict self);


/* Decrement the reference counter of a tuple object filled with symbolic references.
 * >> If the reference counter hits ZERO(0), simply free() the tuple object
 *    without decrementing the reference counters of contained objects.
 *    Otherwise (In case the tuple is being used elsewhere), increment
 *    the reference counters of all contained objects.
 * >> This function is used to safely clean up temporary, local
 *    tuples that are not initialized to contain ~real~ references.
 *    Using this function such tuples can be released with regards
 *    to fixing incorrect reference counters of contained objects.
 *    NOTE: Doing this is still ok, because somewhere further up
 *          the call chain, a caller owns another reference to each
 *          contained object, even before we fix reference counters. */
DFUNDEF NONNULL((1)) void DCALL
DeeTuple_DecrefSymbolic(DeeObject *__restrict self);

/* Create a new tuple object from a sequence or iterator. */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeTuple_FromSequence(DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeTuple_FromIterator(DeeObject *__restrict self);

/* Return a new tuple object containing the types of each object of the given tuple. */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeTuple_Types(DeeObject *__restrict self);

/* Create new tuple objects. */
DFUNDEF WUNUSED DREF DeeObject *DeeTuple_Pack(size_t n, ...);
DFUNDEF WUNUSED DREF DeeObject *DCALL DeeTuple_VPack(size_t n, va_list args);
DFUNDEF WUNUSED DREF DeeObject *DeeTuple_PackSymbolic(size_t n, /*DREF*/ ...);
DFUNDEF WUNUSED DREF DeeObject *DCALL DeeTuple_VPackSymbolic(size_t n, /*DREF*/ va_list args);

/* Create a new tuple from a given vector. */
DFUNDEF WUNUSED NONNULL((2)) DREF DeeObject *DCALL DeeTuple_NewVector(size_t objc, DeeObject *const *__restrict objv);
DFUNDEF WUNUSED NONNULL((2)) DREF DeeObject *DCALL DeeTuple_NewVectorSymbolic(size_t objc, /*inherit(on_success)*/ DREF DeeObject *const *__restrict objv);

/* Similar to `Dee_Packf', but parse any number of formated values and
 * put them in a tuple, essentially doing the same as `Dee_Packf' when
 * the entire `format' string was surrounded by `(' and `)'. */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DeeTuple_Newf(char const *__restrict format, ...);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeTuple_VNewf(char const *__restrict format, va_list args);

/* Concat a tuple and some generic sequence,
 * inheriting a reference from `self' in the process. */
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeTuple_ConcatInherited(/*inherit(on_success)*/ DREF DeeObject *self, DeeObject *sequence);
DFUNDEF WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
DeeTuple_ExtendInherited(/*inherit(on_success)*/ DREF DeeObject *self, size_t argc,
                         /*inherit(on_success)*/ DREF DeeObject *const *argv);

/* Append all elements from an iterator to a tuple.
 * @assume(DeeTuple_IsEmpty(*pself) || !DeeObject_IsShared(*pself)); */
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeTuple_AppendIterator(/*inherit(on_success)*/ DREF DeeObject *__restrict self,
                        DeeObject *__restrict iterator);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeTuple_Append(/*inherit(on_success)*/ DREF DeeObject *__restrict self,
                DeeObject *__restrict item);

#ifdef CONFIG_BUILDING_DEEMON
/* Print all elements of the given tuple without any separators in-between
 * elements. This is equivalent to `Tuple.operator str' and is related to
 * the change introduced for handling `print("foo", "bar");'-like statements */
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
DeeTuple_Print(DeeObject *__restrict self,
               Dee_formatprinter_t printer, void *arg);
#else /* CONFIG_BUILDING_DEEMON */
#define DeeTuple_Print(self, printer, arg) \
	DeeObject_Print(self, printer, arg)
#endif /* !CONFIG_BUILDING_DEEMON */


DECL_END

#endif /* !GUARD_DEEMON_TUPLE_H */
