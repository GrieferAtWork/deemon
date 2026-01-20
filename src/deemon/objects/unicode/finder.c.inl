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
#ifndef GUARD_DEEMON_OBJECTS_UNICODE_ISINDER_C_INL
#define GUARD_DEEMON_OBJECTS_UNICODE_ISINDER_C_INL 1

#include <deemon/api.h>

#include <deemon/alloc.h>
#include <deemon/arg.h>
#include <deemon/computed-operators.h>
#include <deemon/int.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/serial.h>
#include <deemon/string.h>
#include <deemon/util/atomic.h>

#include <hybrid/overflow.h> /* OVERFLOW_USUB */

#include "../../runtime/strings.h"
#include "../generic-proxy.h"
#include "string_functions.h"

#include <stdbool.h> /* bool, false */
#include <stddef.h>  /* NULL, offsetof, size_t */
#include <stdint.h>  /* uintptr_t */

DECL_BEGIN

typedef struct {
	PROXY_OBJECT_HEAD2_EX(String, sf_str,    /* [1..1][const] The string that is being searched. */
	                      String, sf_needle) /* [1..1][const] The needle being searched for. */
	size_t                        sf_start;  /* [const] Starting search index. */
	size_t                        sf_end;    /* [const] End search index. */
	bool                          sf_ovrlap; /* [const] When true, "sfi_find_delta = 1", else "sfi_find_delta = max(#sf_needle, 1)" */
} StringFind;

typedef struct {
	PROXY_OBJECT_HEAD_EX(StringFind, sfi_find)       /* [1..1][const] The underlying find-controller. */
	union dcharptr_const             sfi_start;      /* [1..1][const] Starting pointer. */
	DWEAK union dcharptr_const       sfi_ptr;        /* [1..1] Pointer to the start of data left to be searched. */
	union dcharptr_const             sfi_end;        /* [1..1][const] End pointer. */
	union dcharptr_const             sfi_needle_ptr; /* [1..1][const] Starting pointer of the needle being searched. */
	size_t                           sfi_needle_len; /* [const] Length of the needle being searched. */
	size_t                           sfi_find_delta; /* [const] Delta added to `sfi_ptr' after each match */
	unsigned int                     sfi_width;      /* [const] The common width of the searched, and needle string. */
} StringFindIterator;

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeString_FindAll(String *self, String *other,
                  size_t start, size_t end,
                  bool overlapping);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeString_CaseFindAll(String *self, String *other,
                      size_t start, size_t end,
                      bool overlapping);
#define READ_PTR(x) atomic_read(&(x)->sfi_ptr.ptr)


INTDEF DeeTypeObject StringFindIterator_Type;
INTDEF DeeTypeObject StringFind_Type;
INTDEF DeeTypeObject StringCaseFindIterator_Type;
INTDEF DeeTypeObject StringCaseFind_Type;

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
sf_serialize(StringFind *__restrict self,
             DeeSerial *__restrict writer, Dee_seraddr_t addr) {
#define ADDROF(field) (addr + offsetof(StringFind, field))
	StringFind *out = DeeSerial_Addr2Mem(writer, addr, StringFind);
	out->sf_start  = self->sf_start;
	out->sf_end    = self->sf_end;
	out->sf_ovrlap = self->sf_ovrlap;
	if (DeeSerial_PutObject(writer, ADDROF(sf_str), self->sf_str))
		goto err;
	return DeeSerial_PutObject(writer, ADDROF(sf_needle), self->sf_needle);
err:
	return -1;
#undef ADDROF
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
sfi_serialize(StringFindIterator *__restrict self,
              DeeSerial *__restrict writer, Dee_seraddr_t addr) {
#define ADDROF(field) (addr + offsetof(StringFindIterator, field))
	StringFindIterator *out = DeeSerial_Addr2Mem(writer, addr, StringFindIterator);
	out->sfi_needle_len = self->sfi_needle_len;
	out->sfi_find_delta = self->sfi_find_delta;
	out->sfi_width      = self->sfi_width;
	if (DeeSerial_PutObject(writer, ADDROF(sfi_find), self->sfi_find))
		goto err;
	if (DeeSerial_PutPointer(writer, ADDROF(sfi_start.ptr), self->sfi_start.ptr))
		goto err;
	if (DeeSerial_PutPointer(writer, ADDROF(sfi_ptr.ptr), atomic_read(&self->sfi_ptr.ptr)))
		goto err;
	if (DeeSerial_PutPointer(writer, ADDROF(sfi_end.ptr), self->sfi_end.ptr))
		goto err;
	return DeeSerial_PutPointer(writer, ADDROF(sfi_needle_ptr.ptr), self->sfi_needle_ptr.ptr);
err:
	return -1;
#undef ADDROF
}


PRIVATE WUNUSED NONNULL((1)) int DCALL
sfi_ctor(StringFindIterator *__restrict self) {
	self->sfi_find = (DREF StringFind *)DeeString_FindAll((String *)Dee_EmptyString,
	                                                      (String *)Dee_EmptyString,
	                                                      0, 0, false);
	if unlikely(!self->sfi_find)
		goto err;
	self->sfi_start.cp8      = DeeString_As1Byte(Dee_EmptyString);
	self->sfi_ptr.cp8        = DeeString_As1Byte(Dee_EmptyString);
	self->sfi_end.cp8        = DeeString_As1Byte(Dee_EmptyString);
	self->sfi_needle_ptr.cp8 = DeeString_As1Byte(Dee_EmptyString);
	self->sfi_needle_len     = 0;
	self->sfi_find_delta     = 1;
	self->sfi_width          = STRING_WIDTH_1BYTE;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
scfi_ctor(StringFindIterator *__restrict self) {
	self->sfi_find = (DREF StringFind *)DeeString_CaseFindAll((String *)Dee_EmptyString,
	                                                          (String *)Dee_EmptyString,
	                                                          0, 0, false);
	if unlikely(!self->sfi_find)
		goto err;
	self->sfi_start.cp8      = DeeString_As1Byte(Dee_EmptyString);
	self->sfi_ptr.cp8        = DeeString_As1Byte(Dee_EmptyString);
	self->sfi_end.cp8        = DeeString_As1Byte(Dee_EmptyString);
	self->sfi_needle_ptr.cp8 = DeeString_As1Byte(Dee_EmptyString);
	self->sfi_needle_len     = 0;
	self->sfi_find_delta     = 1;
	self->sfi_width          = STRING_WIDTH_1BYTE;
	return 0;
err:
	return -1;
}

#define scfi_copy sfi_copy
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
sfi_copy(StringFindIterator *__restrict self,
         StringFindIterator *__restrict other) {
	self->sfi_find           = other->sfi_find;
	self->sfi_start.ptr      = other->sfi_start.ptr;
	self->sfi_ptr.ptr        = READ_PTR(other);
	self->sfi_end.ptr        = other->sfi_end.ptr;
	self->sfi_needle_ptr.ptr = other->sfi_needle_ptr.ptr;
	self->sfi_needle_len     = other->sfi_needle_len;
	self->sfi_find_delta     = other->sfi_find_delta;
	self->sfi_width          = other->sfi_width;
	Dee_Incref(self->sfi_find);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
sfi_setup(StringFindIterator *__restrict self,
          StringFind *__restrict find) {
	size_t my_start, my_end;
	self->sfi_width = STRING_WIDTH_COMMON(DeeString_WIDTH(find->sf_str),
	                                      DeeString_WIDTH(find->sf_needle));
	my_start        = find->sf_start;
	my_end          = find->sf_end;
	SWITCH_SIZEOF_WIDTH(self->sfi_width) {

	CASE_WIDTH_1BYTE:
		self->sfi_ptr.cp8   = DeeString_As1Byte((DeeObject *)find->sf_str);
		self->sfi_start.cp8 = self->sfi_ptr.cp8;
		if (my_end > WSTR_LENGTH(self->sfi_ptr.cp8))
			my_end = WSTR_LENGTH(self->sfi_ptr.cp8);
		if (my_start > my_end)
			my_start = my_end;
		self->sfi_end.cp8 = self->sfi_ptr.cp8 + my_end;
		self->sfi_ptr.cp8 += my_start;
		self->sfi_needle_ptr.cp8 = DeeString_As1Byte((DeeObject *)find->sf_needle);
		self->sfi_needle_len     = WSTR_LENGTH(self->sfi_needle_ptr.cp8);
		break;

	CASE_WIDTH_2BYTE:
		self->sfi_ptr.cp16 = DeeString_As2Byte((DeeObject *)find->sf_str);
		if unlikely(!self->sfi_ptr.cp16)
			goto err;
		self->sfi_start.cp16 = self->sfi_ptr.cp16;
		if (my_end > WSTR_LENGTH(self->sfi_ptr.cp16))
			my_end = WSTR_LENGTH(self->sfi_ptr.cp16);
		if (my_start > my_end)
			my_start = my_end;
		self->sfi_end.cp16 = self->sfi_ptr.cp16 + my_end;
		self->sfi_ptr.cp16 += my_start;
		self->sfi_needle_ptr.cp16 = DeeString_As2Byte((DeeObject *)find->sf_needle);
		if unlikely(!self->sfi_needle_ptr.cp16)
			goto err;
		self->sfi_needle_len = WSTR_LENGTH(self->sfi_needle_ptr.cp16);
		break;

	CASE_WIDTH_4BYTE:
		self->sfi_ptr.cp32 = DeeString_As4Byte((DeeObject *)find->sf_str);
		if unlikely(!self->sfi_ptr.cp32)
			goto err;
		self->sfi_start.cp32 = self->sfi_ptr.cp32;
		if (my_end > WSTR_LENGTH(self->sfi_ptr.cp32))
			my_end = WSTR_LENGTH(self->sfi_ptr.cp32);
		if (my_start > my_end)
			my_start = my_end;
		self->sfi_end.cp32 = self->sfi_ptr.cp32 + my_end;
		self->sfi_ptr.cp32 += my_start;
		self->sfi_needle_ptr.cp32 = DeeString_As4Byte((DeeObject *)find->sf_needle);
		if unlikely(!self->sfi_needle_ptr.cp32)
			goto err;
		self->sfi_needle_len = WSTR_LENGTH(self->sfi_needle_ptr.cp32);
		break;
	}
	self->sfi_find_delta = self->sfi_needle_len;
	if (self->sfi_find_delta == 0 || find->sf_ovrlap)
		self->sfi_find_delta = 1;
	self->sfi_find = find;
	Dee_Incref(find);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sfi_init(StringFindIterator *__restrict self,
         size_t argc, DeeObject *const *argv) {
	StringFind *find;
	DeeArg_Unpack1(err, argc, argv, "_StringFindIterator", &find);
	if (DeeObject_AssertTypeExact(find, &StringFind_Type))
		goto err;
	return sfi_setup(self, find);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
scfi_init(StringFindIterator *__restrict self,
          size_t argc, DeeObject *const *argv) {
	StringFind *find;
	DeeArg_Unpack1(err, argc, argv, "_StringCaseFindIterator", &find);
	if (DeeObject_AssertTypeExact(find, &StringCaseFind_Type))
		goto err;
	return sfi_setup(self, find);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
sfi_next(StringFindIterator *__restrict self) {
	union dcharptr_const ptr, new_ptr;
	size_t scan_size;
again:
	ptr.ptr = atomic_read(&self->sfi_ptr.ptr);
	if (OVERFLOW_USUB((uintptr_t)self->sfi_end.ptr, (uintptr_t)ptr.ptr, &scan_size))
		goto iter_done;
	scan_size = Dee_STRING_DIV_SIZEOF_WIDTH(scan_size, self->sfi_width);
	SWITCH_SIZEOF_WIDTH(self->sfi_width) {

	CASE_WIDTH_1BYTE:
		new_ptr.cp8 = memmemb(ptr.cp8, scan_size, self->sfi_needle_ptr.cp8, self->sfi_needle_len);
		if (new_ptr.cp8) {
			if (!atomic_cmpxch_weak(&self->sfi_ptr.cp8, ptr.cp8, new_ptr.cp8 + self->sfi_find_delta))
				goto again;
			return DeeInt_NewSize((size_t)(new_ptr.cp8 - self->sfi_start.cp8));
		}
		break;

	CASE_WIDTH_2BYTE:
		new_ptr.cp16 = memmemw(ptr.cp16, scan_size, self->sfi_needle_ptr.cp16, self->sfi_needle_len);
		if (new_ptr.cp16) {
			if (!atomic_cmpxch_weak(&self->sfi_ptr.cp16, ptr.cp16, new_ptr.cp16 + self->sfi_find_delta))
				goto again;
			return DeeInt_NewSize((size_t)(new_ptr.cp16 - self->sfi_start.cp16));
		}
		break;

	CASE_WIDTH_4BYTE:
		new_ptr.cp32 = memmeml(ptr.cp32, scan_size, self->sfi_needle_ptr.cp32, self->sfi_needle_len);
		if (new_ptr.cp32) {
			if (!atomic_cmpxch_weak(&self->sfi_ptr.cp32, ptr.cp32, new_ptr.cp32 + self->sfi_find_delta))
				goto again;
			return DeeInt_NewSize((size_t)(new_ptr.cp32 - self->sfi_start.cp32));
		}
		break;
	}
iter_done:
	return ITER_DONE;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
scfi_nextpair(StringFindIterator *__restrict self, DREF DeeObject *pair[2]) {
	union dcharptr_const ptr, new_ptr;
	size_t match_length, result, find_delta;
	size_t scan_size;
again:
	ptr.ptr = atomic_read(&self->sfi_ptr.ptr);
	if (OVERFLOW_USUB((uintptr_t)self->sfi_end.ptr, (uintptr_t)ptr.ptr, &scan_size))
		goto iter_done;
	scan_size = Dee_STRING_DIV_SIZEOF_WIDTH(scan_size, self->sfi_width);
	SWITCH_SIZEOF_WIDTH(self->sfi_width) {

	CASE_WIDTH_1BYTE:
		new_ptr.cp8 = dee_memcasememb(ptr.cp8, scan_size,
		                              self->sfi_needle_ptr.cp8,
		                              self->sfi_needle_len,
		                              &match_length);
		if (!new_ptr.cp8)
			goto iter_done;
		find_delta = match_length;
		if (find_delta == 0 || self->sfi_find->sf_ovrlap)
			find_delta = 1;
		if (!atomic_cmpxch_weak(&self->sfi_ptr.cp8, ptr.cp8, new_ptr.cp8 + find_delta))
			goto again;
		result = (size_t)(new_ptr.cp8 - self->sfi_start.cp8);
		break;

	CASE_WIDTH_2BYTE:
		new_ptr.cp16 = dee_memcasememw(ptr.cp16, scan_size,
		                               self->sfi_needle_ptr.cp16,
		                               self->sfi_needle_len,
		                               &match_length);
		if (!new_ptr.cp16)
			goto iter_done;
		find_delta = match_length;
		if (find_delta == 0 || self->sfi_find->sf_ovrlap)
			find_delta = 1;
		if (!atomic_cmpxch_weak(&self->sfi_ptr.cp16, ptr.cp16, new_ptr.cp16 + find_delta))
			goto again;
		result = (size_t)(new_ptr.cp16 - self->sfi_start.cp16);
		break;

	CASE_WIDTH_4BYTE:
		new_ptr.cp32 = dee_memcasememl(ptr.cp32, scan_size,
		                               self->sfi_needle_ptr.cp32,
		                               self->sfi_needle_len,
		                               &match_length);
		if (!new_ptr.cp32)
			goto iter_done;
		find_delta = match_length;
		if (find_delta == 0 || self->sfi_find->sf_ovrlap)
			find_delta = 1;
		if (!atomic_cmpxch_weak(&self->sfi_ptr.cp32, ptr.cp32, new_ptr.cp32 + find_delta))
			goto again;
		result = (size_t)(new_ptr.cp32 - self->sfi_start.cp32);
		break;
	}
	pair[0] = DeeInt_NewSize(result);
	if unlikely(!pair[0])
		goto err;
	pair[1] = DeeInt_NewSize(result + match_length);
	if unlikely(!pair[1])
		goto err_pair_0;
	return 0;
iter_done:
	return 1;
err_pair_0:
	Dee_Decref(pair[0]);
err:
	return -1;
}

PRIVATE struct type_iterator scfi_iterator = {
	/* .tp_nextpair = */ (int (DCALL *)(DeeObject *__restrict, DREF DeeObject *[2]))&scfi_nextpair,
	/* .tp_nextkey   = */ DEFIMPL(&default__nextkey__with__nextpair),
	/* .tp_nextvalue = */ DEFIMPL(&default__nextvalue__with__nextpair),
	/* .tp_advance   = */ DEFIMPL(&default__advance__with__nextpair),
};

STATIC_ASSERT(offsetof(StringFindIterator, sfi_find) == offsetof(ProxyObject, po_obj));
#define sfi_fini  generic_proxy__fini
#define sfi_visit generic_proxy__visit

PRIVATE WUNUSED NONNULL((1)) int DCALL
sfi_bool(StringFindIterator *__restrict self) {
	union dcharptr_const ptr;
	size_t scan_size;
	ptr.ptr = atomic_read(&self->sfi_ptr.ptr);
	if (OVERFLOW_USUB((uintptr_t)self->sfi_end.ptr, (uintptr_t)ptr.ptr, &scan_size))
		goto iter_done;
	scan_size = Dee_STRING_DIV_SIZEOF_WIDTH(scan_size, self->sfi_width);
	SWITCH_SIZEOF_WIDTH(self->sfi_width) {

	CASE_WIDTH_1BYTE:
		ptr.cp8 = memmemb(ptr.cp8, scan_size, self->sfi_needle_ptr.cp8, self->sfi_needle_len);
		break;

	CASE_WIDTH_2BYTE:
		ptr.cp16 = memmemw(ptr.cp16, scan_size, self->sfi_needle_ptr.cp16, self->sfi_needle_len);
		break;

	CASE_WIDTH_4BYTE:
		ptr.cp32 = memmeml(ptr.cp32, scan_size, self->sfi_needle_ptr.cp32, self->sfi_needle_len);
		break;
	}
	return ptr.ptr != NULL;
iter_done:
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
scfi_bool(StringFindIterator *__restrict self) {
	union dcharptr_const ptr;
	size_t scan_size;
	ptr.ptr = atomic_read(&self->sfi_ptr.ptr);
	if (OVERFLOW_USUB((uintptr_t)self->sfi_end.ptr, (uintptr_t)ptr.ptr, &scan_size))
		goto iter_done;
	scan_size = Dee_STRING_DIV_SIZEOF_WIDTH(scan_size, self->sfi_width);
	SWITCH_SIZEOF_WIDTH(self->sfi_width) {

	CASE_WIDTH_1BYTE:
		ptr.cp8 = dee_memcasememb(ptr.cp8, scan_size,
		                          self->sfi_needle_ptr.cp8,
		                          self->sfi_needle_len, NULL);
		break;

	CASE_WIDTH_2BYTE:
		ptr.cp16 = dee_memcasememw(ptr.cp16, scan_size,
		                           self->sfi_needle_ptr.cp16,
		                           self->sfi_needle_len, NULL);
		break;

	CASE_WIDTH_4BYTE:
		ptr.cp32 = dee_memcasememl(ptr.cp32, scan_size,
		                           self->sfi_needle_ptr.cp32,
		                           self->sfi_needle_len, NULL);
		break;
	}
	return ptr.ptr != NULL;
iter_done:
	return 0;
}

PRIVATE struct type_member tpconst sfi_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT,
	                      offsetof(StringFindIterator, sfi_find),
	                      "->?Ert:StringFind"),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst scfi_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT,
	                      offsetof(StringFindIterator, sfi_find),
	                      "->?Ert:StringCaseFind"),
	TYPE_MEMBER_END
};


PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
sfi_hash(StringFindIterator *self) {
	return Dee_HashPointer(READ_PTR(self));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
sfi_compare(StringFindIterator *self, StringFindIterator *other) {
	if (DeeObject_AssertTypeExact(other, Dee_TYPE(self)))
		goto err;
	Dee_return_compareT(void const *, READ_PTR(self),
	                    /*         */ READ_PTR(other));
err:
	return Dee_COMPARE_ERR;
}

#define scfi_cmp sfi_cmp
PRIVATE struct type_cmp sfi_cmp = {
	/* .tp_hash       = */ (Dee_hash_t (DCALL *)(DeeObject *))&sfi_hash,
	/* .tp_compare_eq = */ DEFIMPL(&default__compare_eq__with__compare),
	/* .tp_compare    = */ (int (DCALL *)(DeeObject *, DeeObject *))&sfi_compare,
	/* .tp_trycompare_eq = */ DEFIMPL(&default__trycompare_eq__with__compare_eq),
	/* .tp_eq            = */ DEFIMPL(&default__eq__with__compare_eq),
	/* .tp_ne            = */ DEFIMPL(&default__ne__with__compare_eq),
	/* .tp_lo            = */ DEFIMPL(&default__lo__with__compare),
	/* .tp_le            = */ DEFIMPL(&default__le__with__compare),
	/* .tp_gr            = */ DEFIMPL(&default__gr__with__compare),
	/* .tp_ge            = */ DEFIMPL(&default__ge__with__compare),
};


INTERN DeeTypeObject StringFindIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_StringFindIterator",
	/* .tp_doc      = */ DOC("(find:?Ert:StringFind)\n"
	                         "\n"
	                         "next->?Dint"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ StringFindIterator,
			/* tp_ctor:        */ &sfi_ctor,
			/* tp_copy_ctor:   */ &sfi_copy,
			/* tp_deep_ctor:   */ NULL,
			/* tp_any_ctor:    */ &sfi_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &sfi_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&sfi_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&sfi_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&sfi_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &sfi_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&sfi_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ sfi_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};

INTERN DeeTypeObject StringCaseFindIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_StringCaseFindIterator",
	/* .tp_doc      = */ DOC("(find:?Ert:StringCaseFind)\n"
	                         "\n"
	                         "next->?X2?Dint?Dint"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ StringFindIterator,
			/* tp_ctor:        */ &scfi_ctor,
			/* tp_copy_ctor:   */ &scfi_copy,
			/* tp_deep_ctor:   */ NULL,
			/* tp_any_ctor:    */ &scfi_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &sfi_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&sfi_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&scfi_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&sfi_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &scfi_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ DEFIMPL(&default__iter_next__with__nextpair),
	/* .tp_iterator      = */ &scfi_iterator,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ scfi_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};
#undef READ_PTR





#define scf_ctor sf_ctor
PRIVATE WUNUSED NONNULL((1)) int DCALL
sf_ctor(StringFind *__restrict self) {
	self->sf_str    = (DREF String *)Dee_EmptyString;
	self->sf_needle = (DREF String *)Dee_EmptyString;
	self->sf_start  = 0;
	self->sf_end    = 0;
	Dee_Incref_n(Dee_EmptyString, 2);
	return 0;
}

#define scf_copy sf_copy
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
sf_copy(StringFind *__restrict self,
        StringFind *__restrict other) {
	self->sf_str    = other->sf_str;
	self->sf_needle = other->sf_needle;
	self->sf_start  = other->sf_start;
	self->sf_end    = other->sf_end;
	Dee_Incref(self->sf_str);
	Dee_Incref(self->sf_needle);
	return 0;
}

#define scf_init sf_init
PRIVATE WUNUSED NONNULL((1)) int DCALL
sf_init(StringFind *__restrict self,
        size_t argc, DeeObject *const *argv) {
	self->sf_start = 0;
	self->sf_end   = (size_t)-1;
	if (DeeArg_Unpack(argc, argv, "oo|" UNPuSIZ UNPxSIZ ":_StringFind",
	                  &self->sf_str, &self->sf_needle,
	                  &self->sf_start, &self->sf_end))
		goto err;
	if (DeeObject_AssertTypeExact(self->sf_str, &DeeString_Type))
		goto err;
	if (DeeObject_AssertTypeExact(self->sf_needle, &DeeString_Type))
		goto err;
	Dee_Incref(self->sf_str);
	Dee_Incref(self->sf_needle);
	return 0;
err:
	return -1;
}

STATIC_ASSERT(offsetof(StringFind, sf_str) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(StringFind, sf_str) == offsetof(ProxyObject2, po_obj2));
STATIC_ASSERT(offsetof(StringFind, sf_needle) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(StringFind, sf_needle) == offsetof(ProxyObject2, po_obj2));
#define sf_fini   generic_proxy2__fini
#define scf_fini  generic_proxy2__fini
#define sf_visit  generic_proxy2__visit
#define scf_visit generic_proxy2__visit

PRIVATE WUNUSED NONNULL((1)) DREF StringFindIterator *DCALL
sf_iter(StringFind *__restrict self) {
	DREF StringFindIterator *result;
	result = DeeObject_MALLOC(StringFindIterator);
	if unlikely(!result)
		goto done;
	if (sfi_setup(result, self))
		goto err_r;
	DeeObject_Init(result, &StringFindIterator_Type);
done:
	return result;
err_r:
	DeeObject_FREE(result);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF StringFindIterator *DCALL
scf_iter(StringFind *__restrict self) {
	DREF StringFindIterator *result;
	result = DeeObject_MALLOC(StringFindIterator);
	if unlikely(!result)
		goto done;
	if (sfi_setup(result, self))
		goto err_r;
	DeeObject_Init(result, &StringCaseFindIterator_Type);
done:
	return result;
err_r:
	DeeObject_FREE(result);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
string_contains_f_impl(String *self, String *needle,
                       size_t start, size_t end);
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
string_casecontains_impl_f(String *self, String *needle,
                           size_t start, size_t end);

PRIVATE WUNUSED NONNULL((1)) int DCALL
sf_bool(StringFind *__restrict self) {
	return string_contains_f_impl(self->sf_str, self->sf_needle,
	                              self->sf_start, self->sf_end);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
scf_bool(StringFind *__restrict self) {
	return string_casecontains_impl_f(self->sf_str, self->sf_needle,
	                                  self->sf_start, self->sf_end);
}


PRIVATE struct type_seq sf_seq = {
	/* .tp_iter     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&sf_iter,
	/* .tp_sizeob   = */ DEFIMPL(&default__seq_operator_sizeob__with__seq_operator_size),
	/* .tp_contains = */ DEFIMPL(&default__seq_operator_contains__with__seq_contains), /* TODO: string.substr() == needle */
	/* .tp_getitem                    = */ DEFIMPL(&default__seq_operator_getitem__with__seq_operator_getitem_index),
	/* .tp_delitem                    = */ DEFIMPL(&default__seq_operator_delitem__unsupported),
	/* .tp_setitem                    = */ DEFIMPL(&default__seq_operator_setitem__unsupported),
	/* .tp_getrange                   = */ DEFIMPL(&default__seq_operator_getrange__with__seq_operator_getrange_index__and__seq_operator_getrange_index_n),
	/* .tp_delrange                   = */ DEFIMPL(&default__seq_operator_delrange__unsupported),
	/* .tp_setrange                   = */ DEFIMPL(&default__seq_operator_setrange__unsupported),
	/* .tp_foreach                    = */ DEFIMPL(&default__foreach__with__iter),
	/* .tp_foreach_pair               = */ DEFIMPL(&default__foreach_pair__with__iter),
	/* .tp_bounditem                  = */ DEFIMPL(&default__seq_operator_bounditem__with__seq_operator_getitem),
	/* .tp_hasitem                    = */ DEFIMPL(&default__seq_operator_hasitem__with__seq_operator_getitem),
	/* .tp_size                       = */ DEFIMPL(&default__seq_operator_size__with__seq_operator_iter), /* TODO: string.count() */
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ DEFIMPL(&default__seq_operator_getitem_index__with__seq_operator_foreach),
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ DEFIMPL(&default__seq_operator_delitem_index__unsupported),
	/* .tp_setitem_index              = */ DEFIMPL(&default__seq_operator_setitem_index__unsupported),
	/* .tp_bounditem_index            = */ DEFIMPL(&default__seq_operator_bounditem_index__with__seq_operator_getitem_index),
	/* .tp_hasitem_index              = */ DEFIMPL(&default__seq_operator_hasitem_index__with__seq_operator_getitem_index),
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

PRIVATE struct type_seq scf_seq = {
	/* .tp_iter     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&scf_iter,
	/* .tp_sizeob   = */ DEFIMPL(&default__seq_operator_sizeob__with__seq_operator_size),
	/* .tp_contains = */ DEFIMPL(&default__seq_operator_contains__with__seq_contains), /* TODO: string.substr(...).casecompare(needle) == 0 */
	/* .tp_getitem                    = */ DEFIMPL(&default__seq_operator_getitem__with__seq_operator_getitem_index),
	/* .tp_delitem                    = */ DEFIMPL(&default__seq_operator_delitem__unsupported),
	/* .tp_setitem                    = */ DEFIMPL(&default__seq_operator_setitem__unsupported),
	/* .tp_getrange                   = */ DEFIMPL(&default__seq_operator_getrange__with__seq_operator_getrange_index__and__seq_operator_getrange_index_n),
	/* .tp_delrange                   = */ DEFIMPL(&default__seq_operator_delrange__unsupported),
	/* .tp_setrange                   = */ DEFIMPL(&default__seq_operator_setrange__unsupported),
	/* .tp_foreach                    = */ DEFIMPL(&default__foreach__with__iter),
	/* .tp_foreach_pair               = */ DEFIMPL(&default__foreach_pair__with__iter),
	/* .tp_bounditem                  = */ DEFIMPL(&default__seq_operator_bounditem__with__seq_operator_getitem),
	/* .tp_hasitem                    = */ DEFIMPL(&default__seq_operator_hasitem__with__seq_operator_getitem),
	/* .tp_size                       = */ DEFIMPL(&default__seq_operator_size__with__seq_operator_iter), /* TODO: string.casecount() */
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ DEFIMPL(&default__seq_operator_getitem_index__with__seq_operator_foreach),
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ DEFIMPL(&default__seq_operator_delitem_index__unsupported),
	/* .tp_setitem_index              = */ DEFIMPL(&default__seq_operator_setitem_index__unsupported),
	/* .tp_bounditem_index            = */ DEFIMPL(&default__seq_operator_bounditem_index__with__seq_operator_getitem_index),
	/* .tp_hasitem_index              = */ DEFIMPL(&default__seq_operator_hasitem_index__with__seq_operator_getitem_index),
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


PRIVATE struct type_member tpconst sf_members[] = {
#define scf_members sf_members
	TYPE_MEMBER_FIELD_DOC("__str__", STRUCT_OBJECT, offsetof(StringFind, sf_str), "->?Dstring"),
	TYPE_MEMBER_FIELD_DOC("__needle__", STRUCT_OBJECT, offsetof(StringFind, sf_needle), "->?Dstring"),
	TYPE_MEMBER_FIELD("__start__", STRUCT_SIZE_T | STRUCT_CONST, offsetof(StringFind, sf_start)),
	TYPE_MEMBER_FIELD("__end__", STRUCT_SIZE_T | STRUCT_CONST, offsetof(StringFind, sf_end)),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst sf_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &StringFindIterator_Type),
	TYPE_MEMBER_CONST(STR_ItemType, &DeeInt_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst scf_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &StringCaseFindIterator_Type),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject StringFind_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_StringFind",
	/* .tp_doc      = */ DOC("(s:?Dstring,needle:?Dstring,start=!0,end=!-1)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL | TP_FDEEPIMMUTABLE,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ StringFind,
			/* tp_ctor:        */ &sf_ctor,
			/* tp_copy_ctor:   */ &sf_copy,
			/* tp_deep_ctor:   */ &sf_copy,
			/* tp_any_ctor:    */ &sf_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &sf_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&sf_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&sf_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_seq_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&sf_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__B8EC3298B952DF3A),
	/* .tp_seq           = */ &sf_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ sf_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ sf_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};

INTERN DeeTypeObject StringCaseFind_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_StringCaseFind",
	/* .tp_doc      = */ DOC("(s:?Dstring,needle:?Dstring,start=!0,end=!-1)\n"
	                         "\n"
	                         "[](index:?Dint)->?T2?Dint?Dint"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL | TP_FDEEPIMMUTABLE,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ StringFind,
			/* tp_ctor:        */ &scf_ctor,
			/* tp_copy_ctor:   */ &scf_copy,
			/* tp_deep_ctor:   */ &scf_copy,
			/* tp_any_ctor:    */ &scf_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &sf_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&scf_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&scf_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_seq_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&scf_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__B8EC3298B952DF3A),
	/* .tp_seq           = */ &scf_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ scf_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ scf_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};


INTERN WUNUSED DREF DeeObject *DCALL
DeeString_FindAll(String *self, String *other,
                  size_t start, size_t end,
                  bool overlapping) {
	DREF StringFind *result;
	result = DeeObject_MALLOC(StringFind);
	if unlikely(!result)
		goto done;
	result->sf_str    = self;
	result->sf_needle = other;
	result->sf_start  = start;
	result->sf_end    = end;
	result->sf_ovrlap = overlapping;
	Dee_Incref(self);
	Dee_Incref(other);
	DeeObject_Init(result, &StringFind_Type);
done:
	return Dee_AsObject(result);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeString_CaseFindAll(String *self, String *other,
                      size_t start, size_t end,
                      bool overlapping) {
	DREF StringFind *result;
	result = DeeObject_MALLOC(StringFind);
	if unlikely(!result)
		goto done;
	result->sf_str    = self;
	result->sf_needle = other;
	result->sf_start  = start;
	result->sf_end    = end;
	result->sf_ovrlap = overlapping;
	Dee_Incref(self);
	Dee_Incref(other);
	DeeObject_Init(result, &StringCaseFind_Type);
done:
	return Dee_AsObject(result);
}


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_UNICODE_ISINDER_C_INL */
