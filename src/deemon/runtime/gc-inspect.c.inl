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
#ifndef GUARD_DEEMON_RUNTIME_GC_INSPECT_C_INL
#define GUARD_DEEMON_RUNTIME_GC_INSPECT_C_INL 1
#define DEE_SOURCE

#include "gc.c"
/**/

#include <deemon/api.h>

#if defined(CONFIG_EXPERIMENTAL_REWORKED_GC) || defined(__DEEMON__)
DECL_BEGIN

typedef struct {
} GC;




PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
gcenum_iter(DeeObject *__restrict UNUSED(self)) {
	DeeError_NOTIMPLEMENTED(); /* TODO */
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
gcenum_size(DeeObject *__restrict UNUSED(self)) {
	DeeError_NOTIMPLEMENTED(); /* TODO */
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
gcenum_contains(DeeObject *__restrict UNUSED(self),
                DeeObject *__restrict ob) {
	return_bool(DeeType_IsGC(Dee_TYPE(ob)));
}

PRIVATE struct type_seq gcenum_seq = {
	/* .tp_iter     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&gcenum_iter,
	/* .tp_sizeob   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&gcenum_size,
	/* .tp_contains = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&gcenum_contains,
	/* .tp_getitem                    = */ DEFIMPL(&default__seq_operator_getitem__with__seq_operator_getitem_index),
	/* .tp_delitem                    = */ DEFIMPL(&default__seq_operator_delitem__unsupported),
	/* .tp_setitem                    = */ DEFIMPL(&default__seq_operator_setitem__unsupported),
	/* .tp_getrange                   = */ DEFIMPL(&default__seq_operator_getrange__with__seq_operator_getrange_index__and__seq_operator_getrange_index_n),
	/* .tp_delrange                   = */ DEFIMPL(&default__seq_operator_delrange__unsupported),
	/* .tp_setrange                   = */ DEFIMPL(&default__seq_operator_setrange__unsupported),
	/* .tp_foreach                    = */ DEFIMPL(&default__foreach__with__iter),
	/* .tp_foreach_pair               = */ DEFIMPL(&default__foreach_pair__with__iter),
	/* .tp_bounditem                  = */ DEFIMPL(&default__seq_operator_bounditem__with__seq_operator_getitem),
	/* .tp_hasitem                    = */ DEFIMPL(&default__seq_operator_hasitem__with__seq_operator_sizeob),
	/* .tp_size                       = */ DEFIMPL(&default__size__with__sizeob),
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ DEFIMPL(&default__seq_operator_getitem_index__with__seq_operator_foreach),
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ DEFIMPL(&default__seq_operator_delitem_index__unsupported),
	/* .tp_setitem_index              = */ DEFIMPL(&default__seq_operator_setitem_index__unsupported),
	/* .tp_bounditem_index            = */ DEFIMPL(&default__seq_operator_bounditem_index__with__seq_operator_getitem_index),
	/* .tp_hasitem_index              = */ DEFIMPL(&default__seq_operator_hasitem_index__with__seq_operator_size),
	/* .tp_getrange_index             = */ DEFIMPL(&default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_iter),
	/* .tp_delrange_index             = */ DEFIMPL(&default__seq_operator_delrange_index__unsupported),
	/* .tp_setrange_index             = */ DEFIMPL(&default__seq_operator_setrange_index__unsupported),
	/* .tp_getrange_index_n           = */ DEFIMPL(&default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_iter),
	/* .tp_delrange_index_n           = */ DEFIMPL(&default__seq_operator_delrange_index_n__unsupported),
	/* .tp_setrange_index_n           = */ DEFIMPL(&default__seq_operator_setrange_index_n__unsupported),
	/* .tp_trygetitem                 = */ DEFIMPL(&default__seq_operator_trygetitem__with__seq_operator_trygetitem_index),
	/* .tp_trygetitem_index           = */ DEFIMPL(&default__seq_operator_trygetitem_index__with__seq_operator_foreach),
	/* .tp_trygetitem_string_hash     = */ DEFIMPL(&default__trygetitem_string_hash__with__trygetitem),
	/* .tp_getitem_string_hash        = */ DEFIMPL(&default__getitem_string_hash__with__getitem),
	/* .tp_delitem_string_hash        = */ DEFIMPL(&default__delitem_string_hash__with__delitem),
	/* .tp_setitem_string_hash        = */ DEFIMPL(&default__setitem_string_hash__with__setitem),
	/* .tp_bounditem_string_hash      = */ DEFIMPL(&default__bounditem_string_hash__with__bounditem),
	/* .tp_hasitem_string_hash        = */ DEFIMPL(&default__hasitem_string_hash__with__hasitem),
	/* .tp_trygetitem_string_len_hash = */ DEFIMPL(&default__trygetitem_string_len_hash__with__trygetitem),
	/* .tp_getitem_string_len_hash    = */ DEFIMPL(&default__getitem_string_len_hash__with__getitem),
	/* .tp_delitem_string_len_hash    = */ DEFIMPL(&default__delitem_string_len_hash__with__delitem),
	/* .tp_setitem_string_len_hash    = */ DEFIMPL(&default__setitem_string_len_hash__with__setitem),
	/* .tp_bounditem_string_len_hash  = */ DEFIMPL(&default__bounditem_string_len_hash__with__bounditem),
	/* .tp_hasitem_string_len_hash    = */ DEFIMPL(&default__hasitem_string_len_hash__with__hasitem),
};

PRIVATE struct type_member tpconst gcenum_class_members[] = {
//TODO	TYPE_MEMBER_CONST(STR_Iterator, &GCIter_Type),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
gcenum_collect(DeeObject *UNUSED(self),
               size_t argc, DeeObject *const *argv) {
	size_t result;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("collect", params: """
	size_t max = (size_t)-1;
""", docStringPrefix: "gcenum");]]]*/
#define gcenum_collect_params "max=!-1"
	struct {
		size_t max_;
	} args;
	args.max_ = (size_t)-1;
	DeeArg_Unpack0Or1X(err, argc, argv, "collect", &args.max_, UNPxSIZ, DeeObject_AsSizeM1);
/*[[[end]]]*/
	result = DeeGC_Collect(args.max_);
	if (result == (size_t)-1)
		goto err;
	return DeeInt_NewSize(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
gcenum_reachable(DeeObject *UNUSED(self),
                 size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("reachable", params: """
	DeeObject *from;
	bool transitive = true;
""", docStringPrefix: "gcenum");]]]*/
#define gcenum_reachable_params "from,transitive=!t"
	struct {
		DeeObject *from;
		bool transitive;
	} args;
	args.transitive = true;
	DeeArg_UnpackStruct1XOr2X(err, argc, argv, "reachable", &args, &args.from, "o", _DeeArg_AsObject, &args.transitive, "b", DeeObject_AsBool);
/*[[[end]]]*/
	DeeError_NOTIMPLEMENTED(); /* TODO */
	return NULL;
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
gcenum_referring(DeeObject *UNUSED(self),
                 size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("referring", params: """
	DeeObject *to;
	bool transitive = true;
""", docStringPrefix: "gcenum");]]]*/
#define gcenum_referring_params "to,transitive=!t"
	struct {
		DeeObject *to;
		bool transitive;
	} args;
	args.transitive = true;
	DeeArg_UnpackStruct1XOr2X(err, argc, argv, "referring", &args, &args.to, "o", _DeeArg_AsObject, &args.transitive, "b", DeeObject_AsBool);
/*[[[end]]]*/
	DeeError_NOTIMPLEMENTED(); /* TODO */
	return NULL;
err:
	return NULL;
}


PRIVATE struct type_method tpconst gcenum_methods[] = {
	TYPE_METHOD_F("collect", &gcenum_collect, METHOD_FNOREFESCAPE,
	              "(" gcenum_collect_params ")->?Dint\n"
	              "Try to collect at most @max GC objects and return the actual number collected\n"
	              "Note that more than @max objects may be collected if sufficiently large reference cycles exist"),
	TYPE_METHOD_F("reachable", &gcenum_reachable, METHOD_FNOREFESCAPE,
	              "(" gcenum_reachable_params ")->?DSet\n"
	              "Returns a set of objects that are reachable from @from. "
	              /**/ "When @transitive is !f, only direct references are included"),
	TYPE_METHOD_F("referring", &gcenum_referring, METHOD_FNOREFESCAPE,
	              "(" gcenum_referring_params ")->?DSet\n"
	              "Returns a set of GC objects (acting as roots) and normal objects that "
	              /**/ "somehow end up referencing @to. When @transitive is !f, only GC "
	              /**/ "objects that directly reference @to are included.\n"
	              "It is a technical impossibility to find #Ball objects or places that "
	              /**/ "may hold a reference to some object, since only GC objects are "
	              /**/ "tracked. Additionally, this sort of #I{reverse search} is fairly "
	              /**/ "expensive, meaning this method should be avoided if possible."),
	TYPE_METHOD_END
};

PRIVATE DeeTypeObject GCEnum_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_GCEnum",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL | TP_FABSTRACT,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_SINGLETON,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED_S(
			/* T:              */ DeeObject,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ NULL,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL /* Static singleton, so no serial needed */
		),
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ DEFIMPL(&default__seq_operator_bool__with__seq_operator_sizeob),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_seq_printrepr),
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__96C7A3207FAE93E2),
	/* .tp_seq           = */ &gcenum_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__C6F8E138F179B5AD),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ gcenum_methods,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ gcenum_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};


/* An generic sequence singleton that can be
 * iterated to yield all tracked GC objects.
 * This object also offers a hand full of member functions
 * that user-space an invoke to trigger various GC-related
 * functionality:
 *   - collect(max: int = -1): int;
 * Also: remember that this derives from `Sequence', so you
 *       can use all its attributes, like `empty', etc.
 * NOTE: This object is exported as `gc from deemon' */
PUBLIC DeeObject DeeGCEnumTracked_Singleton = {
	OBJECT_HEAD_INIT(&GCEnum_Type)
};

DECL_END
#endif /* CONFIG_EXPERIMENTAL_REWORKED_GC */

#endif /* !GUARD_DEEMON_RUNTIME_GC_INSPECT_C_INL */
