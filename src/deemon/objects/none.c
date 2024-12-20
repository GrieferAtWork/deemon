/* Copyright (c) 2018-2024 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2024 Griefer@Work                           *
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
#include <deemon/seq.h>
#include <deemon/string.h>

#include "../runtime/runtime_error.h"
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
none_4(void *UNUSED(a), void *UNUSED(b), void *UNUSED(c), void *UNUSED(d)) {
	return_none;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
none_5(void *UNUSED(a), void *UNUSED(b), void *UNUSED(c), void *UNUSED(d), void *UNUSED(e)) {
	return_none;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
none_6(void *UNUSED(a), void *UNUSED(b), void *UNUSED(c), void *UNUSED(d), void *UNUSED(e), void *UNUSED(f)) {
	return_none;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
none_7(void *UNUSED(a), void *UNUSED(b), void *UNUSED(c), void *UNUSED(d), void *UNUSED(e), void *UNUSED(f), void *UNUSED(g)) {
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
none_i2_1(void *UNUSED(a), void *UNUSED(b)) {
	return 1;
}

INTERN int DCALL
none_i3(void *UNUSED(a), void *UNUSED(b), void *UNUSED(c)) {
	return 0;
}

INTERN int DCALL
none_i3_1(void *UNUSED(a), void *UNUSED(b), void *UNUSED(c)) {
	return 1;
}

INTERN int DCALL
none_i4(void *UNUSED(a), void *UNUSED(b), void *UNUSED(c), void *UNUSED(d)) {
	return 0;
}

INTERN int DCALL
none_i4_1(void *UNUSED(a), void *UNUSED(b), void *UNUSED(c), void *UNUSED(d)) {
	return 1;
}

INTERN int DCALL
none_i5(void *UNUSED(a), void *UNUSED(b), void *UNUSED(c), void *UNUSED(d), void *UNUSED(e)) {
	return 0;
}

INTERN int DCALL
none_i6(void *UNUSED(a), void *UNUSED(b), void *UNUSED(c), void *UNUSED(d), void *UNUSED(e), void *UNUSED(f)) {
	return 0;
}

#if __SIZEOF_SIZE_T__ == __SIZEOF_INT__
#define none_s1 none_i1
#define none_s3 none_i3
#define none_s5 none_i5
#else /* __SIZEOF_SIZE_T__ == __SIZEOF_INT__ */
PRIVATE size_t DCALL
none_s1(void *UNUSED(a)) {
	return 0;
}
PRIVATE size_t DCALL
none_s3(void *UNUSED(a), void *UNUSED(b), void *UNUSED(c)) {
	return 0;
}
PRIVATE size_t DCALL
none_s5(void *UNUSED(a), void *UNUSED(b), void *UNUSED(c), void *UNUSED(d), void *UNUSED(e)) {
	return 0;
}
#endif /* __SIZEOF_SIZE_T__ != __SIZEOF_INT__ */

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

PRIVATE struct type_iterator none_iterator = {
	/* .tp_nextpair  = */ (int (DCALL *)(DeeObject *__restrict, DREF DeeObject *[2]))&none_i2_1,
	/* .tp_nextkey   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&none_iternext,
	/* .tp_nextvalue = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&none_iternext,
	/* .tp_advance   = */ (size_t (DCALL *)(DeeObject *__restrict, size_t))&none_i2,
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
none_int(DeeObject *__restrict UNUSED(self)) {
	return_reference_(DeeInt_Zero);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
none_eq(DeeObject *UNUSED(self), DeeObject *other) {
	return_bool(DeeNone_Check(other));
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
none_compare(DeeObject *UNUSED(self), DeeObject *other) {
	return DeeNone_Check(other) ? 0 : -1;
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

PRIVATE WUNUSED NONNULL((1, 2)) DeeObject *DCALL
none_getitemnr(DeeObject *__restrict UNUSED(self),
               /*string*/ DeeObject *__restrict key) {
	err_unknown_key(Dee_None, key);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DeeObject *DCALL
none_getitemnr_string_hash(DeeObject *__restrict UNUSED(self),
                           char const *__restrict key,
                           Dee_hash_t UNUSED(hash)) {
	err_unknown_key_str(Dee_None, key);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DeeObject *DCALL
none_getitemnr_string_len_hash(DeeObject *__restrict UNUSED(self),
                               char const *__restrict key,
                               size_t keylen, Dee_hash_t UNUSED(hash)) {
	err_unknown_key_str_len(Dee_None, key, keylen);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DeeObject *DCALL
none_trygetitemnr(DeeObject *__restrict UNUSED(self),
                  /*string*/ DeeObject *__restrict UNUSED(key)) {
	return ITER_DONE;
}

PRIVATE WUNUSED NONNULL((1, 2)) DeeObject *DCALL
none_trygetitemnr_string_hash(DeeObject *__restrict UNUSED(self),
                              char const *__restrict UNUSED(key),
                              Dee_hash_t UNUSED(hash)) {
	return ITER_DONE;
}

PRIVATE WUNUSED NONNULL((1, 2)) DeeObject *DCALL
none_trygetitemnr_string_len_hash(DeeObject *__restrict UNUSED(self),
                                  char const *__restrict UNUSED(key),
                                  size_t UNUSED(keylen),
                                  Dee_hash_t UNUSED(hash)) {
	return ITER_DONE;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
none_unpack(DeeObject *UNUSED(self), size_t dst_length, /*out*/ DREF DeeObject **dst) {
	Dee_Setrefv(dst, Dee_None, dst_length);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
none_unpack_ex(DeeObject *UNUSED(self), size_t dst_length_min,
               size_t dst_length_max, /*out*/ DREF DeeObject **dst) {
	(void)dst_length_min;
	/* "none" always turns everything into more "none", so unpack to the max # of objects. */
	Dee_Setrefv(dst, Dee_None, dst_length_max);
	return dst_length_max;
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
	/* .tp_hash          = */ &none_hash,
	/* .tp_compare_eq    = */ &none_compare,
	/* .tp_compare       = */ &none_compare,
	/* .tp_trycompare_eq = */ &none_compare,
	/* .tp_eq            = */ &none_eq,
	/* .tp_ne            = */ &none_ne,
	/* .tp_lo            = */ &none_ne,
	/* .tp_le            = */ &none_eq,
	/* .tp_gr            = */ &none_ne,
	/* .tp_ge            = */ &none_eq,
};

PRIVATE struct type_seq none_seq = {
	/* .tp_iter                         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&none_1,
	/* .tp_sizeob                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&none_1,
	/* .tp_contains                     = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&none_2,
	/* .tp_getitem                      = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&none_2,
	/* .tp_delitem                      = */ (int (DCALL *)(DeeObject *, DeeObject *))&none_i2,
	/* .tp_setitem                      = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&none_i3,
	/* .tp_getrange                     = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *))&none_3,
	/* .tp_delrange                     = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&none_i3,
	/* .tp_setrange                     = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *, DeeObject *))&none_i4,
	/* .tp_nsi                          = */ NULL,
	/* .tp_foreach                      = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&none_s3, /* TODO: Infinite loop */
	/* .tp_foreach_pair                 = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&none_s3, /* TODO: Infinite loop */
	/* .tp_enumerate                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_enumerate_t, void *))&none_s3, /* TODO: Infinite loop */
	/* .tp_enumerate_index              = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_enumerate_index_t, void *, size_t, size_t))&none_s5, /* TODO: Infinite loop */
	/* .tp_iterkeys                     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&none_1,
	/* .tp_bounditem                    = */ (int (DCALL *)(DeeObject *, DeeObject *))&none_i2_1,
	/* .tp_hasitem                      = */ (int (DCALL *)(DeeObject *, DeeObject *))&none_i2_1,
	/* .tp_size                         = */ (size_t (DCALL *)(DeeObject *__restrict))&none_s1,
	/* .tp_size_fast                    = */ (size_t (DCALL *)(DeeObject *__restrict))&none_s1,
	/* .tp_getitem_index                = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&none_2,
	/* .tp_getitem_index_fast           = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&none_2,
	/* .tp_delitem_index                = */ (int (DCALL *)(DeeObject *, size_t))&none_i2,
	/* .tp_setitem_index                = */ (int (DCALL *)(DeeObject *, size_t, DeeObject *))&none_i3,
	/* .tp_bounditem_index              = */ (int (DCALL *)(DeeObject *, size_t))&none_i2_1,
	/* .tp_hasitem_index                = */ (int (DCALL *)(DeeObject *, size_t))&none_i2_1,
	/* .tp_getrange_index               = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&none_3,
	/* .tp_delrange_index               = */ (int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&none_i3,
	/* .tp_setrange_index               = */ (int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t, DeeObject *))&none_i4,
	/* .tp_getrange_index_n             = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t))&none_2,
	/* .tp_delrange_index_n             = */ (int (DCALL *)(DeeObject *, Dee_ssize_t))&none_i2,
	/* .tp_setrange_index_n             = */ (int (DCALL *)(DeeObject *, Dee_ssize_t, DeeObject *))&none_i3,
	/* .tp_trygetitem                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&none_2,
	/* .tp_trygetitem_index             = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&none_2,
	/* .tp_trygetitem_string_hash       = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&none_3,
	/* .tp_getitem_string_hash          = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&none_3,
	/* .tp_delitem_string_hash          = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&none_i3,
	/* .tp_setitem_string_hash          = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t, DeeObject *))&none_i4,
	/* .tp_bounditem_string_hash        = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&none_i3_1,
	/* .tp_hasitem_string_hash          = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&none_i3_1,
	/* .tp_trygetitem_string_len_hash   = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&none_4,
	/* .tp_getitem_string_len_hash      = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&none_4,
	/* .tp_delitem_string_len_hash      = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&none_i4,
	/* .tp_setitem_string_len_hash      = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t, DeeObject *))&none_i5,
	/* .tp_bounditem_string_len_hash    = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&none_i4_1,
	/* .tp_hasitem_string_len_hash      = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&none_i4_1,
	/* .tp_asvector                     = */ (size_t (DCALL *)(DeeObject *, size_t, DREF DeeObject **))&none_s3,
	/* .tp_asvector_nothrow             = */ (size_t (DCALL *)(DeeObject *, size_t, DREF DeeObject **))&none_s3,
	/* .tp_unpack                       = */ (int (DCALL *)(DeeObject *, size_t, DREF DeeObject **))&none_unpack,
	/* .tp_unpack_ex                    = */ (size_t (DCALL *)(DeeObject *, size_t, size_t, DREF DeeObject **))&none_unpack_ex,
	/* .tp_unpack_ub                    = */ (int (DCALL *)(DeeObject *, size_t, DREF DeeObject **))&none_unpack,
	/* .tp_getitemnr                    = */ (DeeObject *(DCALL *)(DeeObject *__restrict, /*string*/ DeeObject *__restrict))&none_getitemnr,
	/* .tp_getitemnr_string_hash        = */ (DeeObject *(DCALL *)(DeeObject *__restrict, char const *__restrict, Dee_hash_t))&none_getitemnr_string_hash,
	/* .tp_getitemnr_string_len_hash    = */ (DeeObject *(DCALL *)(DeeObject *__restrict, char const *__restrict, size_t, Dee_hash_t))&none_getitemnr_string_len_hash,
	/* .tp_trygetitemnr                 = */ (DeeObject *(DCALL *)(DeeObject *__restrict, /*string*/ DeeObject *__restrict))&none_trygetitemnr,
	/* .tp_trygetitemnr_string_hash     = */ (DeeObject *(DCALL *)(DeeObject *__restrict, char const *__restrict, Dee_hash_t))&none_trygetitemnr_string_hash,
	/* .tp_trygetitemnr_string_len_hash = */ (DeeObject *(DCALL *)(DeeObject *__restrict, char const *__restrict, size_t, Dee_hash_t))&none_trygetitemnr_string_len_hash,
};

PRIVATE struct type_attr none_attr = {
	/* .tp_getattr                       = */ (DREF DeeObject *(DCALL *)(DeeObject *, /*String*/ DeeObject *))&none_2,
	/* .tp_delattr                       = */ (int (DCALL *)(DeeObject *, /*String*/ DeeObject *))&none_i2,
	/* .tp_setattr                       = */ (int (DCALL *)(DeeObject *, /*String*/ DeeObject *, DeeObject *))&none_i3,
	/* .tp_enumattr                      = */ &none_enumattr,
	/* .tp_findattr                      = */ (int (DCALL *)(DeeTypeObject *, DeeObject *, struct Dee_attribute_info *__restrict, struct Dee_attribute_lookup_rules const *__restrict))&none_i4_1,
	/* .tp_hasattr                       = */ (int (DCALL *)(DeeObject *, /*String*/ DeeObject *))&none_i2_1,
	/* .tp_boundattr                     = */ (int (DCALL *)(DeeObject *, /*String*/ DeeObject *))&none_i2_1,
	/* .tp_callattr                      = */ (DREF DeeObject *(DCALL *)(DeeObject *, /*String*/ DeeObject *, size_t, DeeObject *const *))&none_4,
	/* .tp_callattr_kw                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, /*String*/ DeeObject *, size_t, DeeObject *const *, DeeObject *))&none_5,
	/* .tp_vcallattrf                    = */ (DREF DeeObject *(DCALL *)(DeeObject *, /*String*/ DeeObject *, char const *, va_list))&none_4,
	/* .tp_getattr_string_hash           = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&none_3,
	/* .tp_delattr_string_hash           = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&none_i3,
	/* .tp_setattr_string_hash           = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t, DeeObject *))&none_i4,
	/* .tp_hasattr_string_hash           = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&none_i3_1,
	/* .tp_boundattr_string_hash         = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&none_i3_1,
	/* .tp_callattr_string_hash          = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t, size_t, DeeObject *const *))&none_5,
	/* .tp_callattr_string_hash_kw       = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t, size_t, DeeObject *const *, DeeObject *))&none_6,
	/* .tp_vcallattr_string_hashf        = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t, char const *, va_list))&none_5,
	/* .tp_getattr_string_len_hash       = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&none_4,
	/* .tp_delattr_string_len_hash       = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&none_i4,
	/* .tp_setattr_string_len_hash       = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t, DeeObject *))&none_i5,
	/* .tp_hasattr_string_len_hash       = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&none_i4_1,
	/* .tp_boundattr_string_len_hash     = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&none_i4_1,
	/* .tp_callattr_string_len_hash      = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t, size_t, DeeObject *const *))&none_6,
	/* .tp_callattr_string_len_hash_kw   = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t, size_t, DeeObject *const *, DeeObject *))&none_7,
	/* .tp_findattr_info_string_len_hash = */ (bool (DCALL *)(DeeTypeObject *, DeeObject *, char const *__restrict, size_t, Dee_hash_t, struct Dee_attrinfo *__restrict))&none_i6,
#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
	/* .tp_callattr_tuple                = */ (DREF DeeObject *(DCALL *)(DeeObject *, /*String*/ DeeObject *, DeeObject *))&none_3,
	/* .tp_callattr_tuple_kw             = */ (DREF DeeObject *(DCALL *)(DeeObject *, /*String*/ DeeObject *, DeeObject *, DeeObject *))&none_4,
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
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

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
invoke_none_file_char_operator(DeeTypeObject *tp_self, DeeObject *self,
                               /*0..1*/ DeeObject **p_self,
                               size_t argc, DeeObject *const *argv,
                               Dee_operator_t opname) {
	(void)tp_self;
	(void)self;
	(void)p_self;
	(void)argc;
	(void)argv;
	(void)opname;
	return DeeInt_NewInt8(GETC_EOF);
}


/* All operators implemented by "none" can be constant propagated.
 * They also never throw any exceptions. */
PRIVATE struct type_operator const none_operators[] = {
	TYPE_OPERATOR_FLAGS(OPERATOR_0000_CONSTRUCTOR, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0001_COPY, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0002_DEEPCOPY, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0003_DESTRUCTOR, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0004_ASSIGN, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0005_MOVEASSIGN, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0006_STR, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0007_REPR, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0008_BOOL, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0009_ITERNEXT, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_000A_CALL, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_000B_INT, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_000C_FLOAT, METHOD_FCONSTCALL),
	TYPE_OPERATOR_FLAGS(OPERATOR_000D_INV, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_000E_POS, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_000F_NEG, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0010_ADD, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0011_SUB, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0012_MUL, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0013_DIV, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0014_MOD, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0015_SHL, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0016_SHR, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0017_AND, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0018_OR, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0019_XOR, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_001A_POW, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_001B_INC, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_001C_DEC, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_001D_INPLACE_ADD, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_001E_INPLACE_SUB, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_001F_INPLACE_MUL, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0020_INPLACE_DIV, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0021_INPLACE_MOD, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0022_INPLACE_SHL, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0023_INPLACE_SHR, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0024_INPLACE_AND, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0025_INPLACE_OR, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0026_INPLACE_XOR, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0027_INPLACE_POW, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0028_HASH, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0029_EQ, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_002A_NE, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_002B_LO, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_002C_LE, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_002D_GR, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_002E_GE, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_002F_ITER, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0030_SIZE, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0031_CONTAINS, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0032_GETITEM, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0033_DELITEM, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0034_SETITEM, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0035_GETRANGE, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0036_DELRANGE, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0037_SETRANGE, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0038_GETATTR, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0039_DELATTR, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_003A_SETATTR, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_003B_ENUMATTR, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_003C_ENTER, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_003D_LEAVE, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	/**/

	/* Implement char-related file operators such that we always return `GETC_EOF'.
	 * Without this, `DeeType_GetCustomOperatorById()' would pick noop_custom_operator_cb,
	 * which would re-return "none", which would then evaluate to "0" (which isn't,
	 * and can't be the value of `GETC_EOF') */
	TYPE_OPERATOR_CUSTOM(OPERATOR_FILE_0008_GETC, &invoke_none_file_char_operator, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_CUSTOM(OPERATOR_FILE_0009_UNGETC, &invoke_none_file_char_operator, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_CUSTOM(OPERATOR_FILE_000A_PUTC, &invoke_none_file_char_operator, METHOD_FCONSTCALL | METHOD_FNOTHROW),
};

PUBLIC DeeTypeObject DeeNone_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ DeeString_STR(&str_none),
	/* .tp_doc      = */ DOC("None is a singleton object that can be used in any operation "
	                         /**/ "as either a placeholder, or as a no-op object. Besides being a "
	                         /**/ "no-op for everything, it has several special characteristics, "
	                         /**/ "like its ability to be expanded to any number of itself without "
	                         /**/ "causing :{UnpackError}s to be thrown.\n"

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
	/* .tp_flags    = */ TP_FVARIABLE | TP_FNORMAL | TP_FNAMEOBJECT | TP_FABSTRACT | TP_FFINAL,
	/* .tp_weakrefs = */ WEAKREF_SUPPORT_ADDR(DeeNoneObject),
	/* .tp_features = */ TF_SINGLETON | TF_KW,
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
	/* .tp_iterator      = */ &none_iterator,
	/* .tp_attr          = */ &none_attr,
	/* .tp_with          = */ &none_with,
	/* .tp_buffer        = */ &none_buffer,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call_kw       = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *, DeeObject *))&none_call_kw,
	/* .tp_mro           = */ NULL,
	/* .tp_operators     = */ none_operators,
	/* .tp_operators_size= */ COMPILER_LENOF(none_operators)
};

PUBLIC DeeNoneObject DeeNone_Singleton = {
	OBJECT_HEAD_INIT(&DeeNone_Type),
	WEAKREF_SUPPORT_INIT
};

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_NONE_C */
