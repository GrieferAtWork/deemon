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
#ifndef GUARD_DEEMON_TUPLE_H
#define GUARD_DEEMON_TUPLE_H 1

#include "api.h"
/**/

#include "types.h"
#ifndef __INTELLISENSE__
#include "object.h"
#endif /* !__INTELLISENSE__ */
/**/

#include <stdbool.h> /* bool */
#include <stdarg.h>  /* va_list */
#include <stddef.h>  /* size_t */

DECL_BEGIN

#ifdef DEE_SOURCE
#define Dee_tuple_object tuple_object
#define DEFINE_TUPLE     Dee_DEFINE_TUPLE
#endif /* DEE_SOURCE */

typedef struct Dee_tuple_object DeeTupleObject;

struct Dee_tuple_object {
	/* WARNING: Changes must be mirrored in `/src/deemon/execute/asm/exec.gas-386.S' */
	Dee_OBJECT_HEAD
	size_t                                    t_size;  /* [const] Tuple size. */
	COMPILER_FLEXIBLE_ARRAY(DREF DeeObject *, t_elem); /* [1..1][const][t_size] Tuple elements. */
};

#define DeeTuple_SIZEOF(n_items)                                    \
	_Dee_MallococBufsize(COMPILER_OFFSETOF(DeeTupleObject, t_elem), \
	                     n_items, sizeof(DREF DeeObject *))
#define DeeTuple_IsEmpty(ob)   ((DeeObject *)Dee_REQUIRES_OBJECT(ob) == Dee_EmptyTuple)
#define DeeTuple_SIZE(ob)      ((DeeTupleObject *)Dee_REQUIRES_OBJECT(ob))->t_size
#define DeeTuple_ELEM(ob)      ((DeeTupleObject *)Dee_REQUIRES_OBJECT(ob))->t_elem
#define DeeTuple_END(ob)       (((DeeTupleObject *)Dee_REQUIRES_OBJECT(ob))->t_elem + ((DeeTupleObject *)(ob))->t_size)
#define DeeTuple_GET(ob, i)    ((DeeTupleObject *)Dee_REQUIRES_OBJECT(ob))->t_elem[i]
#define DeeTuple_SET(ob, i, v) (void)(((DeeTupleObject *)Dee_REQUIRES_OBJECT(ob))->t_elem[i] = (DeeObject *)Dee_REQUIRES_OBJECT(v))

/* Same as `DeeTuple_SIZEOF()', but makes sure that no overflow takes place. */
#define DeeTuple_SIZEOF_SAFE(n_items)                                   \
	_Dee_MallococBufsizeSafe(COMPILER_OFFSETOF(DeeTupleObject, t_elem), \
	                         n_items, sizeof(DREF DeeObject *))


/* Define a statically allocated tuple:
 * >> PRIVATE WUNUSED DREF DeeObject *DCALL get_my_tuple(void) {
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


struct Dee_empty_tuple_struct {
	Dee_OBJECT_HEAD
	size_t t_size;
};
DDATDEF struct Dee_empty_tuple_struct DeeTuple_Empty;
#define Dee_EmptyTuple      ((DeeObject *)&DeeTuple_Empty)
#ifdef __INTELLISENSE__
#define DeeTuple_NewEmpty() Dee_EmptyTuple
#else /* __INTELLISENSE__ */
#define DeeTuple_NewEmpty() (Dee_Incref(&DeeTuple_Empty), (DeeObject *)&DeeTuple_Empty)
#endif /* !__INTELLISENSE__ */

DDATDEF DeeTypeObject DeeTuple_Type;
DDATDEF DeeTypeObject DeeNullableTuple_Type; /* Same as "DeeTuple_Type", but items are allowed to be NULL (meaning unbound) */
#define DeeTuple_Check(x)       DeeObject_InstanceOfExact(x, &DeeTuple_Type) /* `Tuple' is final */
#define DeeTuple_CheckExact(x)  DeeObject_InstanceOfExact(x, &DeeTuple_Type)


/* Create new tuple objects. */
DFUNDEF WUNUSED DREF DeeTupleObject *DCALL
DeeTuple_NewUninitialized(size_t n);

/* Same as `DeeTuple_NewUninitialized()', but
 * doesn't throw an exception when returning `NULL' */
DFUNDEF WUNUSED DREF DeeTupleObject *DCALL
DeeTuple_TryNewUninitialized(size_t n);

DFUNDEF WUNUSED NONNULL((1)) DREF DeeTupleObject *DCALL
DeeTuple_ResizeUninitialized(/*inherit(on_success)*/ DREF DeeTupleObject *__restrict self, size_t n);

/* Same as `DeeTuple_ResizeUninitialized()', but
 * doesn't throw an exception when returning `NULL' */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeTupleObject *DCALL
DeeTuple_TryResizeUninitialized(/*inherit(on_success)*/ DREF DeeTupleObject *__restrict self, size_t n);

DFUNDEF WUNUSED ATTR_RETNONNULL NONNULL((1)) DREF DeeTupleObject *DCALL
DeeTuple_TruncateUninitialized(/*inherit(always)*/ DREF DeeTupleObject *__restrict self, size_t n);

DFUNDEF NONNULL((1)) void DCALL
DeeTuple_FreeUninitialized(DREF DeeTupleObject *__restrict self);


/* Create a new tuple object from a sequence or iterator. */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeTuple_FromList(DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeTuple_FromSequence(DeeObject *__restrict self);

/* Return a new tuple object containing the types of each object of the given tuple. */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeTuple_Types(DeeObject *__restrict self);

/* Create new tuple objects. */
DFUNDEF WUNUSED DREF DeeObject *DeeTuple_Pack(size_t n, ...);
DFUNDEF WUNUSED DREF DeeObject *DCALL DeeTuple_VPack(size_t n, va_list args);
DFUNDEF WUNUSED DREF DeeObject *DeeTuple_PackSymbolic(size_t n, /*inherit(on_success)*/ /*DREF*/ ...);
DFUNDEF WUNUSED DREF DeeObject *DCALL DeeTuple_VPackSymbolic(size_t n, /*inherit(on_success)*/ /*DREF*/ va_list args);
DFUNDEF WUNUSED DREF DeeObject *DeeTuple_TryPack(size_t n, ...);
DFUNDEF WUNUSED DREF DeeObject *DCALL DeeTuple_VTryPack(size_t n, va_list args);
DFUNDEF WUNUSED DREF DeeObject *DeeTuple_TryPackSymbolic(size_t n, /*inherit(on_success)*/ /*DREF*/ ...);
DFUNDEF WUNUSED DREF DeeObject *DCALL DeeTuple_VTryPackSymbolic(size_t n, /*inherit(on_success)*/ /*DREF*/ va_list args);

/* Create a new tuple from a given vector. */
DFUNDEF WUNUSED ATTR_INS(2, 1) DREF DeeObject *DCALL DeeTuple_NewVector(size_t objc, DeeObject *const *__restrict objv);
DFUNDEF WUNUSED ATTR_INS(2, 1) DREF DeeObject *DCALL DeeTuple_NewVectorSymbolic(size_t objc, /*inherit(on_success)*/ DREF DeeObject *const *__restrict objv);
DFUNDEF WUNUSED ATTR_INS(2, 1) DREF DeeObject *DCALL DeeTuple_TryNewVector(size_t objc, DeeObject *const *__restrict objv);
DFUNDEF WUNUSED ATTR_INS(2, 1) DREF DeeObject *DCALL DeeTuple_TryNewVectorSymbolic(size_t objc, /*inherit(on_success)*/ DREF DeeObject *const *__restrict objv);

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
 *          contained object, even before we fix reference counters.
 * Semantically speaking, you can think of this function as:
 * >> Dee_Increfv(DeeTuple_ELEM(self), DeeTuple_SIZE(self));
 * >> Dee_Decref(self); */
DFUNDEF NONNULL((1)) void DCALL
DeeTuple_DecrefSymbolic(DeeObject *__restrict self);

/* Similar to `Dee_Packf', but parse any number of formated values and
 * put them in a tuple, essentially doing the same as `Dee_Packf' when
 * the entire `format' string was surrounded by `(' and `)'. */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DeeTuple_Newf(char const *__restrict format, ...);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeTuple_VNewf(char const *__restrict format, va_list args);

/* Concat a tuple and some generic sequence,
 * inheriting a reference from `self' in the process. */
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeTuple_ConcatInherited(/*inherit(always)*/ DREF DeeObject *self, DeeObject *sequence);
DFUNDEF WUNUSED ATTR_INS(3, 2) NONNULL((1, 3)) DREF DeeObject *DCALL
DeeTuple_ExtendInherited(/*inherit(always)*/ DREF DeeObject *self, size_t argc,
                         /*inherit(always)*/ DREF DeeObject *const *argv);

/* Helpers for pairs. */
#define DeeTuple_NewUninitializedPair()      DeeTuple_NewUninitialized(2)
#define DeeTuple_TryNewUninitializedPair()   DeeTuple_TryNewUninitialized(2)
#define DeeTuple_FreeUninitializedPair(self) DeeTuple_FreeUninitialized(self)
#define DeeTuple_PackPair(a, b)              DeeTuple_Pack(2, a, b)
#define DeeTuple_TryPackPair(a, b)           DeeTuple_TryPack(2, a, b)
#define DeeTuple_PackPairSymbolic(a, b)      DeeTuple_PackSymbolic(2, a, b)
#define DeeTuple_TryPackPairSymbolic(a, b)   DeeTuple_TryPackSymbolic(2, a, b)


struct Dee_tuple_builder {
	size_t               tb_size;  /* Used size (allocated size is stored in `tb_tuple->t_size') */
	DREF DeeTupleObject *tb_tuple; /* [0..1][owned] Result tuple (guarantied to not be shared) */
};

#define Dee_tuple_builder_init(self) \
	(void)((self)->tb_size = 0, (self)->tb_tuple = NULL)
#define Dee_tuple_builder_cinit(self)        \
	(void)(Dee_ASSERT((self)->tb_size == 0), \
	       Dee_ASSERT((self)->tb_tuple == NULL))
#define Dee_tuple_builder_init_with_hint(self, hint) \
	(void)((self)->tb_size  = 0,                     \
	       (self)->tb_tuple = DeeTuple_TryNewUninitialized(hint))
#define Dee_tuple_builder_cinit_with_hint(self, hint) \
	(void)(Dee_ASSERT((self)->tb_size == 0),          \
	       (self)->tb_tuple = DeeTuple_TryNewUninitialized(hint))
#define Dee_tuple_builder_fini(self)                                \
	(void)(!(self)->tb_tuple ||                                     \
	       (Dee_Decrefv((self)->tb_tuple->t_elem, (self)->tb_size), \
	        DeeTuple_FreeUninitialized((self)->tb_tuple), 0))
#define Dee_tuple_builder_visit(self)                              \
	do {                                                           \
		if ((self)->tb_tuple)                                      \
			Dee_Visitv((self)->tb_tuple->t_elem, (self)->tb_size); \
	}	__WHILE0
DFUNDEF ATTR_RETNONNULL WUNUSED DREF /*Tuple*/DeeObject *DCALL
Dee_tuple_builder_pack(struct Dee_tuple_builder *__restrict self);
DFUNDEF WUNUSED NONNULL((2)) Dee_ssize_t DCALL
Dee_tuple_builder_append(/*struct Dee_tuple_builder*/ void *self,
                         DeeObject *item);
DFUNDEF WUNUSED NONNULL((2)) Dee_ssize_t DCALL
Dee_tuple_builder_append_inherited(/*struct Dee_tuple_builder*/ void *self,
                                   /*inherit(always)*/ DREF DeeObject *item);
DFUNDEF WUNUSED NONNULL((2)) Dee_ssize_t DCALL
Dee_tuple_builder_appenditems(/*struct Dee_tuple_builder*/ void *self, DeeObject *items);
DFUNDEF WUNUSED ATTR_INS(3, 2) NONNULL((1)) int DCALL
Dee_tuple_builder_extend(struct Dee_tuple_builder *self, size_t objc,
                         DeeObject *const *__restrict objv);
DFUNDEF WUNUSED ATTR_INS(3, 2) NONNULL((1)) int DCALL
Dee_tuple_builder_extend_inherited(struct Dee_tuple_builder *self, size_t objc,
                                   /*inherit(always)*/ DREF DeeObject *const *__restrict objv);

/* Ensure that space for at least `n' items is allocated, and return
 * a pointer to a buffer where those `n' items can be written. Once
 * written, commit the write using `Dee_tuple_builder_commit' */
DFUNDEF WUNUSED NONNULL((1)) DeeObject **DCALL
Dee_tuple_builder_alloc(struct Dee_tuple_builder *__restrict self, size_t n);
DFUNDEF WUNUSED NONNULL((1)) DeeObject **DCALL
Dee_tuple_builder_alloc1(struct Dee_tuple_builder *__restrict self);
#define Dee_tuple_builder_commit(self, n) (void)((self)->tb_size += (n))
#define Dee_tuple_builder_commit1(self)   Dee_tuple_builder_commit(self, 1)

/* Try to ensure that space for at least `n' extra items is available.
 * Returns indicate of that much space now being pre-allocated. */
DFUNDEF NONNULL((1)) bool DCALL
Dee_tuple_builder_reserve(struct Dee_tuple_builder *__restrict self, size_t n);


/* Same as above, but produced tuple's items must either be incref'd,
 * or the reference returned by `Dee_tuple_builder_pack_symbolic' must
 * be decref'd by `DeeTuple_DecrefSymbolic' */
#define Dee_tuple_builder_init_symbolic            Dee_tuple_builder_init
#define Dee_tuple_builder_cinit_symbolic           Dee_tuple_builder_cinit
#define Dee_tuple_builder_init_with_hint_symbolic  Dee_tuple_builder_init_with_hint
#define Dee_tuple_builder_cinit_with_hint_symbolic Dee_tuple_builder_cinit_with_hint
#define Dee_tuple_builder_fini_symbolic(self) \
	(void)(!(self)->tb_tuple || (DeeTuple_FreeUninitialized((self)->tb_tuple), 0))
#define Dee_tuple_builder_pack_symbolic   Dee_tuple_builder_pack
#define Dee_tuple_builder_append_symbolic Dee_tuple_builder_append_inherited
#define Dee_tuple_builder_extend_symbolic Dee_tuple_builder_extend_inherited

DECL_END

#endif /* !GUARD_DEEMON_TUPLE_H */
