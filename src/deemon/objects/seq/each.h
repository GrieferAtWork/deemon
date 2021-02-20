/* Copyright (c) 2018-2021 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2021 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_OBJECTS_SEQ_EACH_H
#define GUARD_DEEMON_OBJECTS_SEQ_EACH_H 1

#include <deemon/api.h>
#include <deemon/object.h>

DECL_BEGIN

/* Dynamic proxy sequence for applying transformations
 * to underlying sequence elements.
 *  - Transformations are applied by performing the same
 *    operation on the each-wrapper, with all operations
 *    with the exception of sequence (`type_seq') and
 *    cast (`type_cast') operators with continue to behave
 *    normally, and as would be expected for a sequence.
 *  - An exception to this is the first operator invoked
 *    after the `each' wrapper is accessed, which _always_
 *    is applied as a proxy operation, then also including
 *    any sequence or cast operator.
 *    Only the wrapper that _it_ then returns can be used
 *    as a sequence, without that use being translated
 *    into another proxy.
 *  - NOTE: Since some operators are required to return specific
 *          types of objects, those operators continue to behave
 *          like they would for any other generic sequence object.
 * >>
 * >> local x = { "foo", "bar", "foobar" };
 * >>
 * >> print repr x.each.upper(); // { "FOO", "BAR", "FOOBAR" }
 * >> print repr x.each.upper().center(8); // { "   FOO  ", "   BAR  ", " FOOBAR " }
 * >> // Same as this:
 * >> print repr x.transform([](x) -> x.upper());
 */

struct string_object;

#define SEQ_EACH_HEAD \
	OBJECT_HEAD       \
	DREF DeeObject *se_seq; /* [1..1][const] The sequence being accessed. */

typedef struct {
	/* `seq.each' -- The root wrapper descriptor which
		* has yet to be bound to any specific operation. */
	SEQ_EACH_HEAD
} SeqEachBase;

typedef struct {
	/* Iterator for any of the sequence-each wrappers. */
	OBJECT_HEAD
	DREF SeqEachBase *ei_each; /* [1..1][const] The sequence-each descriptor that is being used. */
	DREF DeeObject   *ei_iter; /* [1..1][const] The underlying iterator who's elements are being transformed. */
} SeqEachIterator;


/* General purpose operator proxy. */
typedef struct {
	SEQ_EACH_HEAD
	uint16_t        so_opname;    /* [const] Name of the operator that should be applied */
	uint16_t        so_opargc;    /* [const] Number of arguments to pass to the operator. */
#if __SIZEOF_POINTER__ > 4
	uint16_t        so_pad[(__SIZEOF_POINTER__ - 4) / 2]; /* ... */
#endif /* __SIZEOF_POINTER__ > 4 */
	DREF DeeObject *so_opargv[2]; /* [1..1][const][0..so_opargc] Vector of arguments passed to the operator. */
} SeqEachOperator;

#define SeqEachOperator_MALLOC(argc)                                                  \
	((DREF SeqEachOperator *)DeeObject_FMalloc(offsetof(SeqEachOperator, so_opargv) + \
	                                           (argc) * sizeof(DREF DeeObject *)))


#if 0
#define CONFIG_NO_SEQEACH_ATTRIBUTE_OPTIMIZATIONS 1
#endif
#ifndef CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS
#if (!defined(__OPTIMIZE_SIZE__) && \
     !defined(CONFIG_NO_SEQEACH_ATTRIBUTE_OPTIMIZATIONS))
#define CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS 1
#endif /* (!__OPTIMIZE_SIZE__ && !CONFIG_NO_SEQEACH_ATTRIBUTE_OPTIMIZATIONS) */
#endif /* !CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS */

#ifdef CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS

/* Special proxies for commonly used operators. */
typedef struct {
	SEQ_EACH_HEAD
	DREF struct string_object *sg_attr;       /* [1..1][const] The name of the attribute to access. */
} SeqEachGetAttr;

typedef struct {
	SEQ_EACH_HEAD
	DREF struct string_object                *sg_attr;  /* [1..1][const] The name of the attribute to access. */
	size_t                                    sg_argc;  /* [const] Amount of arguments to pass. */
	COMPILER_FLEXIBLE_ARRAY(DREF DeeObject *, sg_argv); /* [1..1][const][0..sg_argc] Vector of arguments to pass. */
} SeqEachCallAttr;

typedef struct {
	SEQ_EACH_HEAD
	DREF struct string_object                *sg_attr;  /* [1..1][const] The name of the attribute to access. */
	DREF DeeObject                           *sg_kw;    /* [1..1][const] Additional keyword to pass during invocation. */
	size_t                                    sg_argc;  /* [const] Amount of arguments to pass. */
	COMPILER_FLEXIBLE_ARRAY(DREF DeeObject *, sg_argv); /* [1..1][const][0..sg_argc] Vector of arguments to pass. */
} SeqEachCallAttrKw;
#endif /* CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS */


INTDEF DeeTypeObject SeqEach_Type;
INTDEF DeeTypeObject SeqEachOperator_Type;
INTDEF DeeTypeObject SeqEachOperatorIterator_Type;

#ifdef CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS
INTDEF DeeTypeObject SeqEachGetAttr_Type;
INTDEF DeeTypeObject SeqEachGetAttrIterator_Type;
INTDEF DeeTypeObject SeqEachCallAttr_Type;
INTDEF DeeTypeObject SeqEachCallAttrIterator_Type;
INTDEF DeeTypeObject SeqEachCallAttrKw_Type;
INTDEF DeeTypeObject SeqEachCallAttrKwIterator_Type;
#endif /* CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS */


#ifdef CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS
/* Hooks for callattr() invocation. */
INTDEF WUNUSED DREF DeeObject *DCALL DeeSeqEach_CallAttr(DeeObject *__restrict self, DeeObject *__restrict attr, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL DeeSeqEach_CallAttrString(DeeObject *__restrict self, char const *__restrict attr, dhash_t hash, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL DeeSeqEach_CallAttrStringLen(DeeObject *__restrict self, char const *__restrict attr, size_t attrlen, dhash_t hash, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL DeeSeqEach_CallAttrKw(DeeObject *__restrict self, DeeObject *__restrict attr, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED DREF DeeObject *DCALL DeeSeqEach_CallAttrStringKw(DeeObject *__restrict self, char const *__restrict attr, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED DREF DeeObject *DCALL DeeSeqEach_CallAttrStringLenKw(DeeObject *__restrict self, char const *__restrict attr, size_t attrlen, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
#define DeeSeqEach_CallAttrTuple(self, attr, args)       DeeSeqEach_CallAttr(self, attr, DeeTuple_SIZE(args), DeeTuple_ELEM(args))
#define DeeSeqEach_CallAttrTupleKw(self, attr, args, kw) DeeSeqEach_CallAttrKw(self, attr, DeeTuple_SIZE(args), DeeTuple_ELEM(args), kw)
INTDEF WUNUSED NONNULL((1, 2)) DREF SeqEachGetAttr *DCALL seqeach_getattr(SeqEachBase *__restrict self, struct string_object *__restrict attr);
INTDEF WUNUSED NONNULL((1, 2)) DREF SeqEachGetAttr *DCALL seqeachw_getattr(DeeObject *__restrict self, struct string_object *__restrict attr);
#endif /* CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS */


/* Construct an each-wrapper for `self' */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_Each(DeeObject *__restrict self);



DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_EACH_H */
