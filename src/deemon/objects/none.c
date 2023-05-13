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
#ifndef GUARD_DEEMON_OBJECTS_NONE_C
#define GUARD_DEEMON_OBJECTS_NONE_C 1

#include <deemon/api.h>
#include <deemon/bool.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/string.h>

#include "../runtime/strings.h"

DECL_BEGIN


PRIVATE WUNUSED DREF DeeObject *DCALL
none_0(void) {
	return_none;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
none_1(void *UNUSED(a)) {
	return_none;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
none_2(void *UNUSED(a), void *UNUSED(b)) {
	return_none;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
none_3(void *UNUSED(a), void *UNUSED(b), void *UNUSED(c)) {
	return_none;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
none_call0(size_t UNUSED(b), void *UNUSED(c)) {
	return_none;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
none_call(void *UNUSED(a), size_t UNUSED(b), void *UNUSED(c)) {
	return_none;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
none_call_kw(void *UNUSED(a), size_t UNUSED(b),
             void *UNUSED(c), void *UNUSED(d)) {
	return_none;
}

INTERN int DCALL
none_i1(void *UNUSED(a)) {
	return 0;
}

INTERN int DCALL
none_i2(void *UNUSED(a), void *UNUSED(b)) {
	return 0;
}

INTERN int DCALL
none_i3(void *UNUSED(a), void *UNUSED(b), void *UNUSED(c)) {
	return 0;
}

PRIVATE int DCALL
none_i4(void *UNUSED(a), void *UNUSED(b), void *UNUSED(c), void *UNUSED(d)) {
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
none_bool(DeeObject *__restrict UNUSED(a)) {
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeStringObject *DCALL
none_str(DeeObject *__restrict UNUSED(a)) {
	return_reference_(&str_none);
}

PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
none_print(DeeObject *__restrict UNUSED(a),
           dformatprinter printer, void *arg) {
	return DeeString_PrintAscii(&str_none, printer, arg);
}

PRIVATE WUNUSED NONNULL((1)) dhash_t DCALL
none_hash(DeeObject *__restrict UNUSED(self)) {
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
none_int32(DeeObject *__restrict UNUSED(self),
           int32_t *__restrict result) {
	*result = 0;
	return INT_UNSIGNED;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
none_int64(DeeObject *__restrict UNUSED(self),
           int64_t *__restrict result) {
	*result = 0;
	return INT_UNSIGNED;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
none_double(DeeObject *__restrict UNUSED(self),
            double *__restrict result) {
	*result = 0;
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
none_iternext(DeeObject *__restrict UNUSED(self)) {
	return ITER_DONE;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
none_int(DeeObject *__restrict UNUSED(self)) {
	return_reference_(DeeInt_Zero);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
none_eq(DeeObject *UNUSED(self), DeeObject *other) {
	return_bool(DeeNone_Check(other));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
none_ne(DeeObject *UNUSED(self), DeeObject *other) {
	return_bool(!DeeNone_Check(other));
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) dssize_t DCALL
none_enumattr(DeeTypeObject *UNUSED(tp_self), DeeObject *UNUSED(self),
              denum_t UNUSED(proc), void *UNUSED(arg)) {
	return 0;
}


PRIVATE struct type_math none_math = {
	/* .tp_int32       = */ &none_int32,
	/* .tp_int64       = */ &none_int64,
	/* .tp_double      = */ &none_double,
	/* .tp_int         = */ &none_int,
	/* .tp_inv         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&none_1,
	/* .tp_pos         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&none_1,
	/* .tp_neg         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&none_1,
	/* .tp_add         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&none_2,
	/* .tp_sub         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&none_2,
	/* .tp_mul         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&none_2,
	/* .tp_div         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&none_2,
	/* .tp_mod         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&none_2,
	/* .tp_shl         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&none_2,
	/* .tp_shr         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&none_2,
	/* .tp_and         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&none_2,
	/* .tp_or          = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&none_2,
	/* .tp_xor         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&none_2,
	/* .tp_pow         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&none_2,
	/* .tp_inc         = */ (int (DCALL *)(DeeObject **__restrict))&none_i1,
	/* .tp_dec         = */ (int (DCALL *)(DeeObject **__restrict))&none_i1,
	/* .tp_inplace_add = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))&none_i2,
	/* .tp_inplace_sub = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))&none_i2,
	/* .tp_inplace_mul = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))&none_i2,
	/* .tp_inplace_div = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))&none_i2,
	/* .tp_inplace_mod = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))&none_i2,
	/* .tp_inplace_shl = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))&none_i2,
	/* .tp_inplace_shr = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))&none_i2,
	/* .tp_inplace_and = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))&none_i2,
	/* .tp_inplace_or  = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))&none_i2,
	/* .tp_inplace_xor = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))&none_i2,
	/* .tp_inplace_pow = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))&none_i2
};

PRIVATE struct type_cmp none_cmp = {
	/* .tp_hash = */ &none_hash,
	/* .tp_eq   = */ &none_eq,
	/* .tp_ne   = */ &none_ne,
	/* .tp_lo   = */ &none_ne,
	/* .tp_le   = */ &none_eq,
	/* .tp_gr   = */ &none_ne,
	/* .tp_ge   = */ &none_eq
};

PRIVATE struct type_seq none_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&none_1,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&none_1,
	/* .tp_contains  = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&none_2,
	/* .tp_get       = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&none_2,
	/* .tp_del       = */ (int (DCALL *)(DeeObject *, DeeObject *))&none_i2,
	/* .tp_set       = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&none_i3,
	/* .tp_range_get = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *))&none_3,
	/* .tp_range_del = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&none_i3,
	/* .tp_range_set = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *, DeeObject *))&none_i4,
};

PRIVATE struct type_attr tpconst none_attr = {
	/* .tp_getattr  = */ (DREF DeeObject *(DCALL *)(DeeObject *, /*String*/ DeeObject *))&none_2,
	/* .tp_delattr  = */ (int (DCALL *)(DeeObject *, /*String*/ DeeObject *))&none_i2,
	/* .tp_setattr  = */ (int (DCALL *)(DeeObject *, /*String*/ DeeObject *, DeeObject *))&none_i3,
	/* .tp_enumattr = */ &none_enumattr
};

PRIVATE struct type_with none_with = {
	/* .tp_enter = */ (int (DCALL *)(DeeObject *__restrict))&none_i1,
	/* .tp_leave = */ (int (DCALL *)(DeeObject *__restrict))&none_i1
};

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
none_getbuf(DeeObject *__restrict UNUSED(self),
            DeeBuffer *__restrict info,
            unsigned int UNUSED(flags)) {
	info->bb_base = DeeObject_DATA(Dee_None);
	info->bb_size = 0;
	return 0;
}

PRIVATE struct type_buffer none_buffer = {
	/* .tp_getbuf       = */ &none_getbuf,
	/* .tp_putbuf       = */ NULL,
	/* .tp_buffer_flags = */ Dee_BUFFER_TYPE_FNORMAL
};

PUBLIC DeeTypeObject DeeNone_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ DeeString_STR(&str_none),
	/* .tp_doc      = */ DOC("None is a singleton object that can be used in any operation "
	                         /**/ "as either a placeholder, or as a no-op object. Besides being a "
	                         /**/ "no-op for everything, it has several special characteristics, "
	                         /**/ "such as its ability of being expanded in any number of itself "
	                         /**/ "without causing :{UnpackError}s to be thrown, only being one "
	                         /**/ "of them\n"

	                         "\n"
	                         "(args!)\n"
	                         "Taking any number of arguments, return the none-singleton\n"

	                         "\n"
	                         "str->\n"
	                         "repr->\n"
	                         "Always returns $\"none\"\n"

	                         "\n"
	                         "bool->\n"
	                         "Always returns $false\n"

	                         "\n"
	                         "int->\n"
	                         "Always returns $0\n"

	                         "\n"
	                         "float->\n"
	                         "Always returns $0.0\n"

	                         "\n"
	                         "copy->\n"
	                         "deepcopy->\n"
	                         ":=->\n"
	                         "move:=->\n"
	                         "next->?N\n"
	                         "call->?N\n"
	                         "~->\n"
	                         "pos->\n"
	                         "neg->\n"
	                         "+(other:?O)->\n"
	                         "-(other:?O)->\n"
	                         "*(other:?O)->\n"
	                         "/(other:?O)->\n"
	                         "%(other:?O)->\n"
	                         "<<(other:?O)->\n"
	                         ">>(other:?O)->\n"
	                         "&(other:?O)->\n"
	                         "|(other:?O)->\n"
	                         "^(other:?O)->\n"
	                         "**(other:?O)->\n"
	                         "++->\n"
	                         "--->\n"
	                         "+=(other:?O)->\n"
	                         "-=(other:?O)->\n"
	                         "*=(other:?O)->\n"
	                         "/=(other:?O)->\n"
	                         "%=(other:?O)->\n"
	                         "<<=(other:?O)->\n"
	                         ">>=(other:?O)->\n"
	                         "&=(other:?O)->\n"
	                         "|=(other:?O)->\n"
	                         "^=(other:?O)->\n"
	                         "**=(other:?O)->\n"
	                         "==(other:?O)->\n"
	                         "!=(other:?O)->\n"
	                         "<(other:?O)->\n"
	                         "<=(other:?O)->\n"
	                         ">(other:?O)->\n"
	                         ">=(other:?O)->\n"
	                         "iter->?N\n"
	                         "#->?N\n"
	                         "contains(other:?O)->?N\n"
	                         "[](index:?O)->?N\n"
	                         "del[](index:?O)->\n"
	                         "[]=(index:?O,value:?O)->\n"
	                         "[:](start:?O,end:?O)->?N\n"
	                         "del[:](start:?O,end:?O)->\n"
	                         "[:]=(start:?O,end:?O,value:?O)->\n"
	                         ".(attr:?Dstring)->?N\n"
	                         "del.(attr:?Dstring)->?N\n"
	                         ".=(attr:?Dstring)->?N\n"
	                         "No-op that ignores all arguments and always re-returns ?N"),
	/* .tp_flags    = */ TP_FVARIABLE|TP_FNORMAL | TP_FNAMEOBJECT | TP_FABSTRACT,
	/* .tp_weakrefs = */ WEAKREF_SUPPORT_ADDR(DeeNoneObject),
	/* .tp_features = */ TF_SINGLETON,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_var = */ {
				/* .tp_ctor      = */ (dfunptr_t)&none_0,
				/* .tp_copy_ctor = */ (dfunptr_t)&none_1,
				/* .tp_deep_ctor = */ (dfunptr_t)&none_1,
				/* .tp_any_ctor  = */ (dfunptr_t)&none_call0
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ (int (DCALL *)(DeeObject *, DeeObject *))&none_i2,
		/* .tp_move_assign = */ (int (DCALL *)(DeeObject *, DeeObject *))&none_i2
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&none_str,
		/* .tp_repr      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&none_str,
		/* .tp_bool      = */ &none_bool,
		/* .tp_print     = */ &none_print,
		/* .tp_printrepr = */ &none_print
	},
	/* .tp_call          = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&none_call,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ &none_math,
	/* .tp_cmp           = */ &none_cmp,
	/* .tp_seq           = */ &none_seq,
	/* .tp_iter_next     = */ &none_iternext,
	/* .tp_attr          = */ &none_attr,
	/* .tp_with          = */ &none_with,
	/* .tp_buffer        = */ &none_buffer,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_call_kw       = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *, DeeObject *))&none_call_kw
};

PUBLIC DeeNoneObject DeeNone_Singleton = {
	OBJECT_HEAD_INIT(&DeeNone_Type),
	WEAKREF_SUPPORT_INIT
};

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_NONE_C */
