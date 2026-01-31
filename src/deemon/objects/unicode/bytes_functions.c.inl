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
#ifndef GUARD_DEEMON_OBJECTS_UNICODE_BYTES_FUNCTIONS_C_INL
#define GUARD_DEEMON_OBJECTS_UNICODE_BYTES_FUNCTIONS_C_INL 1

#include <deemon/api.h>

#include <deemon/arg.h>             /* DeeArg_BadArgcEx, DeeArg_Unpack*, UNPuSIZ, UNPxSIZ, _DeeArg_AsObject */
#include <deemon/bool.h>            /* return_bool, return_false */
#include <deemon/bytes.h>           /* DeeBytes*, Dee_BYTES_PRINTER_INIT, Dee_BYTES_PRINTER_SIZE, Dee_EmptyBytes, Dee_bytes_printer, Dee_bytes_printer_* */
#include <deemon/error-rt.h>        /* DeeRT_Err* */
#include <deemon/error.h>           /* DeeError_Throwf, DeeError_ValueError */
#include <deemon/format.h>          /* PCKuSIZ, PRFuSIZ */
#include <deemon/int.h>             /* DeeInt_* */
#include <deemon/method-hints.h>    /* TYPE_METHOD_HINT*, type_method_hint */
#include <deemon/none.h>            /* DeeNone_NewRef, return_none */
#include <deemon/object.h>
#include <deemon/regex.h>           /* DeeRegex*, DeeString_GetRegex, Dee_RE_* */
#include <deemon/seq.h>             /* DeeSeq_NewEmpty */
#include <deemon/serial.h>          /* DeeSerial*, Dee_seraddr_t */
#include <deemon/string.h>          /* DeeAscii_ItoaLowerDigit, DeeString*, DeeUni_*, Dee_EmptyString, Dee_UNICODE_*, Dee_uniflag_t, WSTR_LENGTH */
#include <deemon/system-features.h> /* DeeSystem_DEFINE_memrev, DeeSystem_DEFINE_memsetp, bzero, close, isalnum, isalpha, isdigit, islower, isupper, memcasecmp, memchr, memcmp, memcpy*, mempcpy, memset, open */
#include <deemon/tuple.h>           /* DeeTuple* */
#include <deemon/util/atomic.h>     /* atomic_xch */

#include <hybrid/typecore.h> /* __BYTE_TYPE__, __UINTPTR_TYPE__ */

#include "../../runtime/kwlist.h"
#include "../../runtime/runtime_error.h"
#include "../../runtime/strings.h"
#include "regroups.h"
#include "string_functions.h"

#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* NULL, offsetof, size_t */
#include <stdint.h>  /* uintptr_t */

DECL_BEGIN

#undef byte_t
#define byte_t __BYTE_TYPE__

#ifndef CONFIG_HAVE_memrev
#define CONFIG_HAVE_memrev
#undef memrev
#define memrev dee_memrev
DeeSystem_DEFINE_memrev(dee_memrev)
#endif /* !CONFIG_HAVE_memrev */

typedef DeeBytesObject Bytes;

#ifndef DeeTuple_NewII_DEFINED
#define DeeTuple_NewII_DEFINED
#ifdef __OPTIMIZE_SIZE__
#define DeeTuple_NewII(a, b) DeeTuple_Newf(PCKuSIZ PCKuSIZ, a, b)
#else /* __OPTIMIZE_SIZE__ */
PRIVATE WUNUSED DREF DeeObject *DCALL
DeeTuple_NewII(size_t a, size_t b) {
	DREF DeeObject *aval, *bval;
	DREF DeeTupleObject *result;
	aval = DeeInt_NewSize(a);
	if unlikely(!aval)
		goto err;
	bval = DeeInt_NewSize(b);
	if unlikely(!bval)
		goto err_aval;
	result = DeeTuple_NewUninitializedPair();
	if unlikely(!result)
		goto err_aval_bval;
	DeeTuple_SET(result, 0, aval); /* Inherit reference */
	DeeTuple_SET(result, 1, bval); /* Inherit reference */
	return Dee_AsObject(result);
err_aval_bval:
	Dee_Decref(bval);
err_aval:
	Dee_Decref(aval);
err:
	return NULL;
}
#endif /* !__OPTIMIZE_SIZE__ */
#endif /* !DeeTuple_NewII_DEFINED */


#ifndef __NO_builtin_expect
#define acquire_needle(self, ob) __builtin_expect(acquire_needle(self, ob), 0)
#endif /* !__NO_builtin_expect */

typedef struct {
	byte_t const *n_data;
	size_t        n_size;
	byte_t       _n_buf[sizeof(size_t)];
} Needle;

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
Needle_Serialize(Needle *__restrict self,
                 DeeSerial *__restrict writer,
                 Dee_seraddr_t addr) {
#define ADDROF(field) (addr + offsetof(Needle, field))
	Needle *out = DeeSerial_Addr2Mem(writer, addr, Needle);
	out->n_size = self->n_size;
	memcpy(out->_n_buf, self->_n_buf, sizeof(self->_n_buf));
	if (self->n_data == self->_n_buf)
		return DeeSerial_PutAddr(writer, ADDROF(n_data), ADDROF(_n_buf));
	return DeeSerial_PutPointer(writer, ADDROF(n_data), self->n_data);
#undef ADDROF
}

PRIVATE WUNUSED NONNULL((1, 2)) int
(DCALL acquire_needle)(/*out*/ Needle *__restrict self,
                       /*in*/ DeeObject *__restrict ob) {
	if (DeeString_Check(ob)) {
		self->n_data = DeeString_AsBytes(ob, false);
		if unlikely(!self->n_data)
			goto err;
		self->n_size = WSTR_LENGTH(self->n_data) * sizeof(char);
	} else if (DeeBytes_Check(ob)) {
		self->n_data = DeeBytes_DATA(ob);
		self->n_size = DeeBytes_SIZE(ob);
	} else {
		/* TODO: Support for SeqSome */

		/* Convert to an integer (to-be used as a single byte). */
		if (DeeObject_AsByte(ob, self->_n_buf))
			goto err;
		self->n_data = self->_n_buf;
		self->n_size = 1;
	}
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bytes_contains(Bytes *self, DeeObject *needle_ob) {
	bool result;
	Needle needle;
	if (acquire_needle(&needle, needle_ob))
		goto err;
	result = memmemb(DeeBytes_DATA(self),
	                 DeeBytes_SIZE(self),
	                 needle.n_data,
	                 needle.n_size) != NULL;
	return_bool(result);
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_find(Bytes *self, size_t argc,
           DeeObject *const *argv, DeeObject *kw) {
	size_t mylen;
	Needle needle;
	byte_t *result;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("find", params: "
	DeeObject *needle: ?X3?DBytes?Dstring?Dint, size_t start = 0, size_t end = (size_t)-1
", docStringPrefix: "bytes");]]]*/
#define bytes_find_params "needle:?X3?.?Dstring?Dint,start=!0,end=!-1"
	struct {
		DeeObject *needle;
		size_t start;
		size_t end;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__needle_start_end, "o|" UNPuSIZ UNPxSIZ ":find", &args))
		goto err;
/*[[[end]]]*/
	mylen = DeeBytes_SIZE(self);
	CLAMP_SUBSTR(&args.start, &args.end, &mylen, not_found);
	if (acquire_needle(&needle, args.needle))
		goto err;
	result = (byte_t *)memmemb(DeeBytes_DATA(self) + args.start, mylen,
	                           needle.n_data, needle.n_size);
	if (result)
		return DeeInt_NewSize((size_t)(result - DeeBytes_DATA(self)));
not_found:
	return DeeInt_NewMinusOne();
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_index(Bytes *self, size_t argc,
            DeeObject *const *argv, DeeObject *kw) {
	size_t mylen;
	Needle needle;
	byte_t *result;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("index", params: "
	DeeObject *needle: ?X3?DBytes?Dstring?Dint, size_t start = 0, size_t end = (size_t)-1
", docStringPrefix: "bytes");]]]*/
#define bytes_index_params "needle:?X3?.?Dstring?Dint,start=!0,end=!-1"
	struct {
		DeeObject *needle;
		size_t start;
		size_t end;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__needle_start_end, "o|" UNPuSIZ UNPxSIZ ":index", &args))
		goto err;
/*[[[end]]]*/
	mylen = DeeBytes_SIZE(self);
	CLAMP_SUBSTR(&args.start, &args.end, &mylen, not_found);
	if (acquire_needle(&needle, args.needle))
		goto err;
	result = (byte_t *)memmemb(DeeBytes_DATA(self) + args.start, mylen,
	                           needle.n_data, needle.n_size);
	if likely(result)
		return DeeInt_NewSize((size_t)(result - DeeBytes_DATA(self)));
not_found:
	DeeRT_ErrSubstringNotFound(Dee_AsObject(self), args.needle, args.start, args.end);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_casefind(Bytes *self, size_t argc,
               DeeObject *const *argv, DeeObject *kw) {
	size_t mylen;
	Needle needle;
	byte_t *result;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("casefind", params: "
	DeeObject *needle: ?X3?DBytes?Dstring?Dint, size_t start = 0, size_t end = (size_t)-1
", docStringPrefix: "bytes");]]]*/
#define bytes_casefind_params "needle:?X3?.?Dstring?Dint,start=!0,end=!-1"
	struct {
		DeeObject *needle;
		size_t start;
		size_t end;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__needle_start_end, "o|" UNPuSIZ UNPxSIZ ":casefind", &args))
		goto err;
/*[[[end]]]*/
	mylen = DeeBytes_SIZE(self);
	CLAMP_SUBSTR(&args.start, &args.end, &mylen, not_found);
	if (acquire_needle(&needle, args.needle))
		goto err;
	result = (byte_t *)memasciicasemem(DeeBytes_DATA(self) + args.start, mylen,
	                                   needle.n_data, needle.n_size);
	if (result) {
		size_t index = (size_t)(result - DeeBytes_DATA(self));
		return DeeTuple_NewII(index, index + needle.n_size);
	}
not_found:
	return DeeInt_NewMinusOne();
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_caseindex(Bytes *self, size_t argc,
                DeeObject *const *argv, DeeObject *kw) {
	size_t mylen;
	Needle needle;
	byte_t *result;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("caseindex", params: "
	DeeObject *needle: ?X3?DBytes?Dstring?Dint, size_t start = 0, size_t end = (size_t)-1
", docStringPrefix: "bytes");]]]*/
#define bytes_caseindex_params "needle:?X3?.?Dstring?Dint,start=!0,end=!-1"
	struct {
		DeeObject *needle;
		size_t start;
		size_t end;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__needle_start_end, "o|" UNPuSIZ UNPxSIZ ":caseindex", &args))
		goto err;
/*[[[end]]]*/
	mylen = DeeBytes_SIZE(self);
	CLAMP_SUBSTR(&args.start, &args.end, &mylen, not_found);
	if (acquire_needle(&needle, args.needle))
		goto err;
	result = (byte_t *)memasciicasemem(DeeBytes_DATA(self) + args.start, mylen,
	                                   needle.n_data, needle.n_size);
	if likely(result) {
		size_t index = (size_t)(result - DeeBytes_DATA(self));
		return DeeTuple_NewII(index, index + needle.n_size);
	}
not_found:
	DeeRT_ErrSubstringNotFound(Dee_AsObject(self), args.needle, args.start, args.end);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_rfind(Bytes *self, size_t argc,
            DeeObject *const *argv, DeeObject *kw) {
	size_t mylen;
	Needle needle;
	byte_t *result;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("rfind", params: "
	DeeObject *needle: ?X3?DBytes?Dstring?Dint, size_t start = 0, size_t end = (size_t)-1
", docStringPrefix: "bytes");]]]*/
#define bytes_rfind_params "needle:?X3?.?Dstring?Dint,start=!0,end=!-1"
	struct {
		DeeObject *needle;
		size_t start;
		size_t end;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__needle_start_end, "o|" UNPuSIZ UNPxSIZ ":rfind", &args))
		goto err;
/*[[[end]]]*/
	mylen = DeeBytes_SIZE(self);
	CLAMP_SUBSTR(&args.start, &args.end, &mylen, not_found);
	if (acquire_needle(&needle, args.needle))
		goto err;
	result = (byte_t *)memrmemb(DeeBytes_DATA(self) + args.start, mylen,
	                            needle.n_data, needle.n_size);
	if (result)
		return DeeInt_NewSize((size_t)(result - DeeBytes_DATA(self)));
not_found:
	return DeeInt_NewMinusOne();
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_rindex(Bytes *self, size_t argc,
             DeeObject *const *argv, DeeObject *kw) {
	size_t mylen;
	Needle needle;
	byte_t *result;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("rindex", params: "
	DeeObject *needle: ?X3?DBytes?Dstring?Dint, size_t start = 0, size_t end = (size_t)-1
", docStringPrefix: "bytes");]]]*/
#define bytes_rindex_params "needle:?X3?.?Dstring?Dint,start=!0,end=!-1"
	struct {
		DeeObject *needle;
		size_t start;
		size_t end;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__needle_start_end, "o|" UNPuSIZ UNPxSIZ ":rindex", &args))
		goto err;
/*[[[end]]]*/
	mylen = DeeBytes_SIZE(self);
	CLAMP_SUBSTR(&args.start, &args.end, &mylen, not_found);
	if (acquire_needle(&needle, args.needle))
		goto err;
	result = (byte_t *)memrmemb(DeeBytes_DATA(self) + args.start, mylen,
	                            needle.n_data, needle.n_size);
	if likely(result)
		return DeeInt_NewSize((size_t)(result - DeeBytes_DATA(self)));
not_found:
	DeeRT_ErrSubstringNotFound(Dee_AsObject(self), args.needle, args.start, args.end);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_caserfind(Bytes *self, size_t argc,
                DeeObject *const *argv, DeeObject *kw) {
	size_t mylen;
	Needle needle;
	byte_t *result;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("caserfind", params: "
	DeeObject *needle: ?X3?DBytes?Dstring?Dint, size_t start = 0, size_t end = (size_t)-1
", docStringPrefix: "bytes");]]]*/
#define bytes_caserfind_params "needle:?X3?.?Dstring?Dint,start=!0,end=!-1"
	struct {
		DeeObject *needle;
		size_t start;
		size_t end;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__needle_start_end, "o|" UNPuSIZ UNPxSIZ ":caserfind", &args))
		goto err;
/*[[[end]]]*/
	mylen = DeeBytes_SIZE(self);
	CLAMP_SUBSTR(&args.start, &args.end, &mylen, not_found);
	if (acquire_needle(&needle, args.needle))
		goto err;
	result = (byte_t *)memasciicasermem(DeeBytes_DATA(self) + args.start, mylen,
	                                    needle.n_data, needle.n_size);
	if (result) {
		size_t index = (size_t)(result - DeeBytes_DATA(self));
		return DeeTuple_NewII(index, index + needle.n_size);
	}
not_found:
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_caserindex(Bytes *self, size_t argc,
                 DeeObject *const *argv, DeeObject *kw) {
	size_t mylen;
	Needle needle;
	byte_t *result;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("caserindex", params: "
	DeeObject *needle: ?X3?DBytes?Dstring?Dint, size_t start = 0, size_t end = (size_t)-1
", docStringPrefix: "bytes");]]]*/
#define bytes_caserindex_params "needle:?X3?.?Dstring?Dint,start=!0,end=!-1"
	struct {
		DeeObject *needle;
		size_t start;
		size_t end;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__needle_start_end, "o|" UNPuSIZ UNPxSIZ ":caserindex", &args))
		goto err;
/*[[[end]]]*/
	mylen = DeeBytes_SIZE(self);
	CLAMP_SUBSTR(&args.start, &args.end, &mylen, not_found);
	if (acquire_needle(&needle, args.needle))
		goto err;
	result = (byte_t *)memasciicasermem(DeeBytes_DATA(self) + args.start, mylen,
	                                    needle.n_data, needle.n_size);
	if likely(result) {
		size_t index = (size_t)(result - DeeBytes_DATA(self));
		return DeeTuple_NewII(index, index + needle.n_size);
	}
not_found:
	DeeRT_ErrSubstringNotFound(Dee_AsObject(self), args.needle, args.start, args.end);
err:
	return NULL;
}


struct bytes_findany_data {
	byte_t *bfad_base;   /* [1..1][const] Search range start. */
	size_t  bfad_size;   /* [const] Search range size (in bytes). */
	size_t  bfad_result; /* [<= bfad_size] Offset from `bfad_base' of best match. */
};

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
bytes_findany_cb(void *arg, DeeObject *elem) {
	struct bytes_findany_data *data = (struct bytes_findany_data *)arg;
	Needle needle;
	byte_t *result;
	size_t search_size;
	if (acquire_needle(&needle, elem))
		goto err;
	search_size = data->bfad_result + needle.n_size - 1;
	if (search_size > data->bfad_size)
		search_size = data->bfad_size;
	result = (byte_t *)memmemb(data->bfad_base, search_size,
	                           needle.n_data, needle.n_size);
	if (result != NULL) {
		size_t hit = (size_t)(result - data->bfad_base);
		ASSERT(hit <= data->bfad_result);
		data->bfad_result = hit;
		if (hit == 0)
			return -2; /* Found hit at offset=0 -> can stop enumerating needles. */
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_findany(Bytes *self, size_t argc,
              DeeObject *const *argv, DeeObject *kw) {
	Dee_ssize_t status;
	struct bytes_findany_data data;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("findany", params: "
	DeeObject *needles: ?S?X3?DBytes?Dstring?Dint, size_t start = 0, size_t end = (size_t)-1
", docStringPrefix: "bytes");]]]*/
#define bytes_findany_params "needles:?S?X3?.?Dstring?Dint,start=!0,end=!-1"
	struct {
		DeeObject *needles;
		size_t start;
		size_t end;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__needles_start_end, "o|" UNPuSIZ UNPxSIZ ":findany", &args))
		goto err;
/*[[[end]]]*/
	data.bfad_size = DeeBytes_SIZE(self);
	CLAMP_SUBSTR(&args.start, &args.end, &data.bfad_size, not_found);
	data.bfad_base   = DeeBytes_DATA(self) + args.start;
	data.bfad_result = data.bfad_size;
	status = DeeObject_Foreach(args.needles, &bytes_findany_cb, &data);
	if unlikely(status == -1)
		goto err;
	if (data.bfad_result < data.bfad_size || status == -2)
		return DeeInt_NewSize(args.start + data.bfad_result);
not_found:
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_indexany(Bytes *self, size_t argc,
               DeeObject *const *argv, DeeObject *kw) {
	Dee_ssize_t status;
	struct bytes_findany_data data;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("indexany", params: "
	DeeObject *needles: ?S?X3?DBytes?Dstring?Dint, size_t start = 0, size_t end = (size_t)-1
", docStringPrefix: "bytes");]]]*/
#define bytes_indexany_params "needles:?S?X3?.?Dstring?Dint,start=!0,end=!-1"
	struct {
		DeeObject *needles;
		size_t start;
		size_t end;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__needles_start_end, "o|" UNPuSIZ UNPxSIZ ":indexany", &args))
		goto err;
/*[[[end]]]*/
	data.bfad_size = DeeBytes_SIZE(self);
	CLAMP_SUBSTR(&args.start, &args.end, &data.bfad_size, not_found);
	data.bfad_base   = DeeBytes_DATA(self) + args.start;
	data.bfad_result = data.bfad_size;
	status = DeeObject_Foreach(args.needles, &bytes_findany_cb, &data);
	if unlikely(status == -1)
		goto err;
	if (data.bfad_result < data.bfad_size || status == -2)
		return DeeInt_NewSize(args.start + data.bfad_result);
not_found:
	DeeRT_ErrSubstringNotFound(Dee_AsObject(self), args.needles, args.start, args.end);
err:
	return NULL;
}

struct bytes_casefindany_data {
	byte_t *bcfad_base;   /* [1..1] Search range start. */
	size_t  bcfad_size;   /* Search range size (in bytes). */
	size_t  bcfad_result; /* [(. + bcfad_reslen) <= bcfad_size] Offset from `bcfad_base' of best match. */
	size_t  bcfad_reslen; /* # of matched bytes at `bcfad_result'. */
};

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
bytes_casefindany_cb(void *arg, DeeObject *elem) {
	struct bytes_casefindany_data *data = (struct bytes_casefindany_data *)arg;
	Needle needle;
	byte_t *result;
	size_t search_size;
	if (acquire_needle(&needle, elem))
		goto err;
	search_size = data->bcfad_result + needle.n_size - 1;
	if (search_size > data->bcfad_size)
		search_size = data->bcfad_size;
	result = (byte_t *)memasciicasemem(data->bcfad_base, search_size,
	                                   needle.n_data, needle.n_size);
	if (result != NULL) {
		size_t hit = (size_t)(result - data->bcfad_base);
		ASSERT(hit <= data->bcfad_result);
		data->bcfad_result = hit;
		data->bcfad_reslen = needle.n_size;
		if (hit == 0 && needle.n_size <= 0)
			return -2; /* Found hit at offset=0 -> can stop enumerating needles. */
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_casefindany(Bytes *self, size_t argc,
                  DeeObject *const *argv, DeeObject *kw) {
	Dee_ssize_t status;
	struct bytes_casefindany_data data;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("casefindany", params: "
	DeeObject *needles: ?S?X3?DBytes?Dstring?Dint, size_t start = 0, size_t end = (size_t)-1
", docStringPrefix: "bytes");]]]*/
#define bytes_casefindany_params "needles:?S?X3?.?Dstring?Dint,start=!0,end=!-1"
	struct {
		DeeObject *needles;
		size_t start;
		size_t end;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__needles_start_end, "o|" UNPuSIZ UNPxSIZ ":casefindany", &args))
		goto err;
/*[[[end]]]*/
	data.bcfad_size = DeeBytes_SIZE(self);
	CLAMP_SUBSTR(&args.start, &args.end, &data.bcfad_size, not_found);
	data.bcfad_base   = DeeBytes_DATA(self) + args.start;
	data.bcfad_result = data.bcfad_size;
	data.bcfad_reslen = 0;
	status = DeeObject_Foreach(args.needles, &bytes_casefindany_cb, &data);
	if unlikely(status == -1)
		goto err;
	if (data.bcfad_result < data.bcfad_size || status == -2) {
		return DeeTuple_NewII(args.start + data.bcfad_result,
		                      args.start + data.bcfad_result + data.bcfad_reslen);
	}
not_found:
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_caseindexany(Bytes *self, size_t argc,
                   DeeObject *const *argv, DeeObject *kw) {
	Dee_ssize_t status;
	struct bytes_casefindany_data data;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("caseindexany", params: "
	DeeObject *needles: ?S?X3?DBytes?Dstring?Dint, size_t start = 0, size_t end = (size_t)-1
", docStringPrefix: "bytes");]]]*/
#define bytes_caseindexany_params "needles:?S?X3?.?Dstring?Dint,start=!0,end=!-1"
	struct {
		DeeObject *needles;
		size_t start;
		size_t end;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__needles_start_end, "o|" UNPuSIZ UNPxSIZ ":caseindexany", &args))
		goto err;
/*[[[end]]]*/
	data.bcfad_size = DeeBytes_SIZE(self);
	CLAMP_SUBSTR(&args.start, &args.end, &data.bcfad_size, not_found);
	data.bcfad_base   = DeeBytes_DATA(self) + args.start;
	data.bcfad_result = data.bcfad_size;
	data.bcfad_reslen = 0;
	status = DeeObject_Foreach(args.needles, &bytes_casefindany_cb, &data);
	if unlikely(status == -1)
		goto err;
	if (data.bcfad_result < data.bcfad_size || status == -2) {
		return DeeTuple_NewII(args.start + data.bcfad_result,
		                      args.start + data.bcfad_result + data.bcfad_reslen);
	}
not_found:
	DeeRT_ErrSubstringNotFound(Dee_AsObject(self), args.needles, args.start, args.end);
err:
	return NULL;
}

struct bytes_rfindany_data {
	byte_t *brfad_base; /* [1..1] Search range start. */
	size_t  brfad_size; /* Search range size (in bytes). */
};

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
bytes_rfindany_cb(void *arg, DeeObject *elem) {
	struct bytes_rfindany_data *data = (struct bytes_rfindany_data *)arg;
	Needle needle;
	byte_t *result;
	if (acquire_needle(&needle, elem))
		goto err;
	result = (byte_t *)memrmemb(data->brfad_base, data->brfad_size,
	                            needle.n_data, needle.n_size);
	if (result != NULL) {
		size_t hit = (size_t)(result - data->brfad_base) + 1;
		ASSERT((hit - 1) <= data->brfad_size);
		data->brfad_base += hit;
		data->brfad_size -= hit;
		if (data->brfad_size == (size_t)-1)
			return -2; /* Found hit at the very end -> can stop enumerating needles. */
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_rfindany(Bytes *self, size_t argc,
               DeeObject *const *argv, DeeObject *kw) {
	byte_t *orig_base;
	Dee_ssize_t status;
	struct bytes_rfindany_data data;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("rfindany", params: "
	DeeObject *needles: ?S?X3?DBytes?Dstring?Dint, size_t start = 0, size_t end = (size_t)-1
", docStringPrefix: "bytes");]]]*/
#define bytes_rfindany_params "needles:?S?X3?.?Dstring?Dint,start=!0,end=!-1"
	struct {
		DeeObject *needles;
		size_t start;
		size_t end;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__needles_start_end, "o|" UNPuSIZ UNPxSIZ ":rfindany", &args))
		goto err;
/*[[[end]]]*/
	data.brfad_size = DeeBytes_SIZE(self);
	CLAMP_SUBSTR(&args.start, &args.end, &data.brfad_size, not_found);
	data.brfad_base = orig_base = DeeBytes_DATA(self) + args.start;
	status = DeeObject_Foreach(args.needles, &bytes_rfindany_cb, &data);
	if unlikely(status == -1)
		goto err;
	if (data.brfad_base > orig_base || status == -2) {
		size_t index = args.start + ((data.brfad_base - 1) - orig_base);
		return DeeInt_NewSize(index);
	}
not_found:
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_rindexany(Bytes *self, size_t argc,
                DeeObject *const *argv, DeeObject *kw) {
	byte_t *orig_base;
	Dee_ssize_t status;
	struct bytes_rfindany_data data;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("rindexany", params: "
	DeeObject *needles: ?S?X3?DBytes?Dstring?Dint, size_t start = 0, size_t end = (size_t)-1
", docStringPrefix: "bytes");]]]*/
#define bytes_rindexany_params "needles:?S?X3?.?Dstring?Dint,start=!0,end=!-1"
	struct {
		DeeObject *needles;
		size_t start;
		size_t end;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__needles_start_end, "o|" UNPuSIZ UNPxSIZ ":rindexany", &args))
		goto err;
/*[[[end]]]*/
	data.brfad_size = DeeBytes_SIZE(self);
	CLAMP_SUBSTR(&args.start, &args.end, &data.brfad_size, not_found);
	data.brfad_base = orig_base = DeeBytes_DATA(self) + args.start;
	status = DeeObject_Foreach(args.needles, &bytes_rfindany_cb, &data);
	if unlikely(status == -1)
		goto err;
	if (data.brfad_base > orig_base || status == -2)
		return DeeInt_NewSize(args.start + ((data.brfad_base - 1) - orig_base));
not_found:
	DeeRT_ErrSubstringNotFound(Dee_AsObject(self), args.needles, args.start, args.end);
err:
	return NULL;
}

struct bytes_caserfindany_data {
	byte_t *bcrfad_base;   /* [1..1] Search range start. */
	size_t  bcrfad_size;   /* Search range size (in bytes). */
	size_t  bcrfad_reslen; /* # of matched bytes at last match. */
};

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
bytes_caserfindany_cb(void *arg, DeeObject *elem) {
	struct bytes_caserfindany_data *data = (struct bytes_caserfindany_data *)arg;
	Needle needle;
	byte_t *result;
	if (acquire_needle(&needle, elem))
		goto err;
	result = (byte_t *)memasciicasermem(data->bcrfad_base, data->bcrfad_size,
	                                    needle.n_data, needle.n_size);
	if (result != NULL) {
		size_t hit = (size_t)(result - data->bcrfad_base) + 1;
		ASSERT((hit - 1) <= data->bcrfad_size);
		data->bcrfad_base += hit;
		data->bcrfad_size -= hit;
		data->bcrfad_reslen = needle.n_size;
		if (data->bcrfad_size == (size_t)-1)
			return -2; /* Found hit at the very end -> can stop enumerating needles. */
	}
	return 0;
err:
	return -1;
}
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_caserfindany(Bytes *self, size_t argc,
                   DeeObject *const *argv, DeeObject *kw) {
	byte_t *orig_base;
	Dee_ssize_t status;
	struct bytes_caserfindany_data data;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("caserfindany", params: "
	DeeObject *needles: ?S?X3?DBytes?Dstring?Dint, size_t start = 0, size_t end = (size_t)-1
", docStringPrefix: "bytes");]]]*/
#define bytes_caserfindany_params "needles:?S?X3?.?Dstring?Dint,start=!0,end=!-1"
	struct {
		DeeObject *needles;
		size_t start;
		size_t end;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__needles_start_end, "o|" UNPuSIZ UNPxSIZ ":caserfindany", &args))
		goto err;
/*[[[end]]]*/
	data.bcrfad_size = DeeBytes_SIZE(self);
	CLAMP_SUBSTR(&args.start, &args.end, &data.bcrfad_size, not_found);
	data.bcrfad_base = orig_base = DeeBytes_DATA(self) + args.start;
	status = DeeObject_Foreach(args.needles, &bytes_caserfindany_cb, &data);
	if unlikely(status == -1)
		goto err;
	if (data.bcrfad_base > orig_base || status == -2) {
		size_t index = args.start + ((data.bcrfad_base - 1) - orig_base);
		return DeeTuple_NewII(index, index + data.bcrfad_reslen);
	}
not_found:
	return_none;
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_caserindexany(Bytes *self, size_t argc,
                    DeeObject *const *argv, DeeObject *kw) {
	byte_t *orig_base;
	Dee_ssize_t status;
	struct bytes_caserfindany_data data;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("caserindexany", params: "
	DeeObject *needles: ?S?X3?DBytes?Dstring?Dint, size_t start = 0, size_t end = (size_t)-1
", docStringPrefix: "bytes");]]]*/
#define bytes_caserindexany_params "needles:?S?X3?.?Dstring?Dint,start=!0,end=!-1"
	struct {
		DeeObject *needles;
		size_t start;
		size_t end;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__needles_start_end, "o|" UNPuSIZ UNPxSIZ ":caserindexany", &args))
		goto err;
/*[[[end]]]*/
	data.bcrfad_size = DeeBytes_SIZE(self);
	CLAMP_SUBSTR(&args.start, &args.end, &data.bcrfad_size, not_found);
	data.bcrfad_base = orig_base = DeeBytes_DATA(self) + args.start;
	status = DeeObject_Foreach(args.needles, &bytes_caserfindany_cb, &data);
	if unlikely(status == -1)
		goto err;
	if (data.bcrfad_base > orig_base || status == -2) {
		size_t index = args.start + ((data.bcrfad_base - 1) - orig_base);
		return DeeTuple_NewII(index, index + data.bcrfad_reslen);
	}
not_found:
	DeeRT_ErrSubstringNotFound(Dee_AsObject(self), args.needles, args.start, args.end);
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_count(Bytes *self, size_t argc,
            DeeObject *const *argv, DeeObject *kw) {
	Needle needle;
	size_t result, size;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("count", params: "
	DeeObject *needle: ?X3?DBytes?Dstring?Dint, size_t start = 0, size_t end = (size_t)-1
", docStringPrefix: "bytes");]]]*/
#define bytes_count_params "needle:?X3?.?Dstring?Dint,start=!0,end=!-1"
	struct {
		DeeObject *needle;
		size_t start;
		size_t end;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__needle_start_end, "o|" UNPuSIZ UNPxSIZ ":count", &args))
		goto err;
/*[[[end]]]*/
	size = DeeBytes_SIZE(self);
	CLAMP_SUBSTR(&args.start, &args.end, &size, not_found);
	if (acquire_needle(&needle, args.needle))
		goto err;
	result = memcnt(DeeBytes_DATA(self) + args.start,
	                size, needle.n_data, needle.n_size);
	return DeeInt_NewSize(result);
not_found:
	return DeeInt_NewZero();
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_casecount(Bytes *self, size_t argc,
                DeeObject *const *argv, DeeObject *kw) {
	Needle needle;
	size_t result, size;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("casecount", params: "
	DeeObject *needle: ?X3?DBytes?Dstring?Dint, size_t start = 0, size_t end = (size_t)-1
", docStringPrefix: "bytes");]]]*/
#define bytes_casecount_params "needle:?X3?.?Dstring?Dint,start=!0,end=!-1"
	struct {
		DeeObject *needle;
		size_t start;
		size_t end;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__needle_start_end, "o|" UNPuSIZ UNPxSIZ ":casecount", &args))
		goto err;
/*[[[end]]]*/
	size = DeeBytes_SIZE(self);
	CLAMP_SUBSTR(&args.start, &args.end, &size, not_found);
	if (acquire_needle(&needle, args.needle))
		goto err;
	result = memcasecnt(DeeBytes_DATA(self) + args.start,
	                    size, needle.n_data, needle.n_size);
	return DeeInt_NewSize(result);
not_found:
	return DeeInt_NewZero();
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_contains_f(Bytes *self, size_t argc,
                 DeeObject *const *argv, DeeObject *kw) {
	bool result;
	Needle needle;
	size_t size;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("contains", params: "
	DeeObject *needle: ?X3?DBytes?Dstring?Dint, size_t start = 0, size_t end = (size_t)-1
", docStringPrefix: "bytes");]]]*/
#define bytes_contains_params "needle:?X3?.?Dstring?Dint,start=!0,end=!-1"
	struct {
		DeeObject *needle;
		size_t start;
		size_t end;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__needle_start_end, "o|" UNPuSIZ UNPxSIZ ":contains", &args))
		goto err;
/*[[[end]]]*/
	size = DeeBytes_SIZE(self);
	CLAMP_SUBSTR(&args.start, &args.end, &size, not_found);
	if (acquire_needle(&needle, args.needle))
		goto err;
	result = memmemb(DeeBytes_DATA(self) + args.start, size,
	                 needle.n_data, needle.n_size) != NULL;
	return_bool(result);
not_found:
	return_false;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_casecontains_f(Bytes *self, size_t argc,
                     DeeObject *const *argv, DeeObject *kw) {
	bool result;
	Needle needle;
	size_t size;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("casecontains", params: "
	DeeObject *needle: ?X3?DBytes?Dstring?Dint, size_t start = 0, size_t end = (size_t)-1
", docStringPrefix: "bytes");]]]*/
#define bytes_casecontains_params "needle:?X3?.?Dstring?Dint,start=!0,end=!-1"
	struct {
		DeeObject *needle;
		size_t start;
		size_t end;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__needle_start_end, "o|" UNPuSIZ UNPxSIZ ":casecontains", &args))
		goto err;
/*[[[end]]]*/
	size = DeeBytes_SIZE(self);
	CLAMP_SUBSTR(&args.start, &args.end, &size, not_found);
	if (acquire_needle(&needle, args.needle))
		goto err;
	result = memasciicasemem(DeeBytes_DATA(self) + args.start, size,
	                         needle.n_data, needle.n_size) != NULL;
	return_bool(result);
not_found:
	return_false;
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_format(Bytes *self, size_t argc, DeeObject *const *argv) {
	struct Dee_bytes_printer printer;
	Dee_ssize_t status;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("format", params: "
	args:?S?O
", docStringPrefix: "bytes");]]]*/
#define bytes_format_params "args:?S?O"
	struct {
		DeeObject *args;
	} args;
	DeeArg_Unpack1(err, argc, argv, "format", &args.args);
/*[[[end]]]*/
	Dee_bytes_printer_init(&printer);
	status = DeeString_FormatPrinter((char const *)DeeBytes_DATA(self), DeeBytes_SIZE(self), args.args,
	                                 (Dee_formatprinter_t)&Dee_bytes_printer_append, &Dee_bytes_printer_print,
	                                 &printer);
	if unlikely(status < 0)
		goto err_printer;
	return Dee_bytes_printer_pack(&printer);
err_printer:
	Dee_bytes_printer_fini(&printer);
err:
	return NULL;
}



PRIVATE WUNUSED NONNULL((1)) DREF Bytes *DCALL
bytes_getsubstr_locked(Bytes *__restrict self,
                       size_t start, size_t end) {
	if (end >= DeeBytes_SIZE(self)) {
		if (start == 0)
			return_reference_(self);
		end = DeeBytes_SIZE(self);
	}
	if (start >= end)
		return_reference_((DREF Bytes *)Dee_EmptyBytes);
	return (DREF Bytes *)DeeBytes_NewSubView(self,
	                                         (byte_t *)DeeBytes_DATA(self) + (size_t)start,
	                                         (size_t)(end - start));
}

PRIVATE WUNUSED NONNULL((1)) DREF Bytes *DCALL
bytes_getsubstr(Bytes *__restrict self, size_t start, size_t end) {
	DREF Bytes *result;
	result = bytes_getsubstr_locked(self, start, end);
	return result;
}

PRIVATE WUNUSED DREF Bytes *DCALL
bytes_substr(Bytes *self, size_t argc,
             DeeObject *const *argv, DeeObject *kw) {
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("substr", params: "
	size_t start = 0, size_t end = (size_t)-1
", docStringPrefix: "bytes");]]]*/
#define bytes_substr_params "start=!0,end=!-1"
	struct {
		size_t start;
		size_t end;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__start_end, "|" UNPuSIZ UNPxSIZ ":substr", &args))
		goto err;
/*[[[end]]]*/
	return bytes_getsubstr(self, args.start, args.end);
err:
	return NULL;
}

PRIVATE WUNUSED DREF Bytes *DCALL
bytes_resized(Bytes *self, size_t argc, DeeObject *const *argv) {
	DREF Bytes *result;
	size_t new_size;
	if (argc == 1) {
		size_t old_size;
		if (DeeObject_AsSize(argv[0], &new_size))
			goto err;
		result = DeeBytes_NewBufferUninitialized(new_size);
		if unlikely(!result)
			goto err;
		old_size = DeeBytes_SIZE(self);
		if (new_size > old_size) {
			void *p;
			p = mempcpy(DeeBytes_BUFFER_DATA(result), DeeBytes_DATA(self), old_size);
			bzero(p, new_size - old_size);
		} else {
			memcpy(DeeBytes_BUFFER_DATA(result), DeeBytes_DATA(self), new_size);
		}
	} else if (argc == 2) {
		byte_t init;
		if (DeeObject_AsSize(argv[0], &new_size))
			goto err;
		if (DeeObject_AsByte(argv[1], &init))
			goto err;
		result = DeeBytes_NewBufferUninitialized(new_size);
		if unlikely(!result)
			goto err;
		if (new_size <= DeeBytes_SIZE(self)) {
			memcpy(DeeBytes_BUFFER_DATA(result), DeeBytes_DATA(self), new_size);
		} else {
			void *endptr;
			size_t old_size = DeeBytes_SIZE(self);
			endptr = mempcpy(DeeBytes_BUFFER_DATA(result), DeeBytes_DATA(self), old_size);
			memset(endptr, init, new_size - old_size);
		}
	} else {
		DeeArg_BadArgcEx("resized", argc, 1, 2);
		goto err;
	}
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED DREF Bytes *DCALL
bytes_reversed(Bytes *self, size_t argc,
               DeeObject *const *argv, DeeObject *kw) {
	DREF Bytes *result;
	byte_t *src, *dst;
	size_t size;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("reversed", params: "
	size_t start = 0, size_t end = (size_t)-1
", docStringPrefix: "bytes");]]]*/
#define bytes_reversed_params "start=!0,end=!-1"
	struct {
		size_t start;
		size_t end;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__start_end, "|" UNPuSIZ UNPxSIZ ":reversed", &args))
		goto err;
/*[[[end]]]*/
	size = DeeBytes_SIZE(self);
	CLAMP_SUBSTR(&args.start, &args.end, &size, empty);
	result = DeeBytes_NewBufferUninitialized(size);
	if unlikely(!result)
		goto err;
	dst = DeeBytes_BUFFER_DATA(result);
	src = DeeBytes_DATA(self);
	src += size;
	do {
		*dst++ = *--src;
	} while (--size);
	return result;
empty:
	return_reference_((DREF Bytes *)Dee_EmptyBytes);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF Bytes *DCALL
bytes_reverse(Bytes *self, size_t argc,
              DeeObject *const *argv, DeeObject *kw) {
	size_t size;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("reverse", params: "
	size_t start = 0, size_t end = (size_t)-1
", docStringPrefix: "bytes");]]]*/
#define bytes_reverse_params "start=!0,end=!-1"
	struct {
		size_t start;
		size_t end;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__start_end, "|" UNPuSIZ UNPxSIZ ":reverse", &args))
		goto err;
/*[[[end]]]*/
	if unlikely(!DeeBytes_IsWritable(self)) {
		err_bytes_not_writable(Dee_AsObject(self));
		goto err;
	}
	size = DeeBytes_SIZE(self);
	CLAMP_SUBSTR(&args.start, &args.end, &size, done);
	memrev(DeeBytes_DATA(self) + args.start, size);
done:
	return_reference_(self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF Bytes *DCALL
bytes_makereadonly(Bytes *self, size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("makereadonly", params: "");]]]*/
	DeeArg_Unpack0(err, argc, argv, "makereadonly");
/*[[[end]]]*/
	if (!DeeBytes_IsWritable(self))
		return_reference_(self);
	return (DREF Bytes *)DeeBytes_NewSubViewRo(self,
	                                           DeeBytes_DATA(self),
	                                           DeeBytes_SIZE(self));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF Bytes *DCALL
bytes_makewritable(Bytes *self, size_t argc, DeeObject *const *argv) {
	DREF Bytes *result;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("makewritable", params: "");]]]*/
	DeeArg_Unpack0(err, argc, argv, "makewritable");
/*[[[end]]]*/
	if (DeeBytes_IsWritable(self))
		return_reference_(self);
	/* Return a copy of `self' */
	result = DeeBytes_NewBufferData(DeeBytes_DATA(self),
	                                DeeBytes_SIZE(self));
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_hex(Bytes *self, size_t argc,
          DeeObject *const *argv, DeeObject *kw) {
	char *dst;
	DREF DeeStringObject *result;
	byte_t const *src;
	size_t size;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("hex", params: "
	size_t start = 0, size_t end = (size_t)-1
", docStringPrefix: "bytes");]]]*/
#define bytes_hex_params "start=!0,end=!-1"
	struct {
		size_t start;
		size_t end;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__start_end, "|" UNPuSIZ UNPxSIZ ":hex", &args))
		goto err;
/*[[[end]]]*/
	size = DeeBytes_SIZE(self);
	CLAMP_SUBSTR(&args.start, &args.end, &size, empty);
	result = (DREF DeeStringObject *)DeeString_NewBuffer(size * 2);
	if unlikely(!result)
		goto err;
	dst = DeeString_GetBuffer(result);
	src = DeeBytes_DATA(self) + args.start;
	do {
		byte_t byte = *src++;
#ifndef CONFIG_NO_THREADS
		COMPILER_READ_BARRIER();
#endif /* !CONFIG_NO_THREADS */
		*dst++ = DeeAscii_ItoaLowerDigit(byte >> 4);
		*dst++ = DeeAscii_ItoaLowerDigit(byte & 0xf);
	} while (--size);
	ASSERT(dst == DeeString_END(result));
	return Dee_AsObject(result);
empty:
	return DeeString_NewEmpty();
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_ord(Bytes *self, size_t argc, DeeObject *const *argv) {
	size_t index = 0;
	byte_t result;
	if (argc) {
		DeeArg_Unpack1X(err, argc, argv, "ord", &index, UNPuSIZ, DeeObject_AsSize);
		if (index >= DeeBytes_SIZE(self)) {
			DeeRT_ErrIndexOutOfBounds(Dee_AsObject(self), index,
			                          DeeBytes_SIZE(self));
			goto err;
		}
	} else {
		if unlikely(DeeBytes_SIZE(self) != 1)
			goto err_not_single;
	}
	result = DeeBytes_DATA(self)[index];
	return DeeInt_NEWU(result);
err_not_single:
	err_expected_single_character_string(Dee_AsObject(self));
err:
	return NULL;
}

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeString_Scanf(DeeObject *self, DeeObject *format);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_scanf(Bytes *self, size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("scanf", params: "
	DeeStringObject *format
", docStringPrefix: "bytes");]]]*/
#define bytes_scanf_params "format:?Dstring"
	struct {
		DeeStringObject *format;
	} args;
	DeeArg_Unpack1(err, argc, argv, "scanf", &args.format);
/*[[[end]]]*/
	if (DeeObject_AssertTypeExact(args.format, &DeeString_Type))
		goto err;
	return DeeString_Scanf(Dee_AsObject(self), (DeeObject *)args.format);
err:
	return NULL;
}


#define DeeBytes_IsCntrl(self, start, end)   DeeBytes_TestTrait(self, start, end, Dee_UNICODE_ISCNTRL)
#define DeeBytes_IsTab(self, start, end)     DeeBytes_TestTrait(self, start, end, Dee_UNICODE_ISTAB)
#define DeeBytes_IsCempty(self, start, end)  DeeBytes_TestTrait(self, start, end, Dee_UNICODE_ISEMPTY)
#define DeeBytes_IsWhite(self, start, end)   DeeBytes_TestTrait(self, start, end, Dee_UNICODE_ISWHITE)
#define DeeBytes_IsLF(self, start, end)      DeeBytes_TestTrait(self, start, end, Dee_UNICODE_ISLF)
#define DeeBytes_IsSpace(self, start, end)   DeeBytes_TestTrait(self, start, end, Dee_UNICODE_ISSPACE)
#define DeeBytes_IsLower(self, start, end)   DeeBytes_TestTrait(self, start, end, Dee_UNICODE_ISLOWER)
#define DeeBytes_IsUpper(self, start, end)   DeeBytes_TestTrait(self, start, end, Dee_UNICODE_ISUPPER)
#define DeeBytes_IsAlpha(self, start, end)   DeeBytes_TestTrait(self, start, end, Dee_UNICODE_ISALPHA)
#define DeeBytes_IsDigit(self, start, end)   DeeBytes_TestTrait(self, start, end, Dee_UNICODE_ISDIGIT)
#define DeeBytes_IsHex(self, start, end)     DeeBytes_TestTrait(self, start, end, Dee_UNICODE_ISHEX)
#define DeeBytes_IsXdigit(self, start, end)  DeeBytes_TestTrait(self, start, end, Dee_UNICODE_ISXDIGIT)
#define DeeBytes_IsAlnum(self, start, end)   DeeBytes_TestTrait(self, start, end, Dee_UNICODE_ISALNUM)
#define DeeBytes_IsPunct(self, start, end)   DeeBytes_TestTrait(self, start, end, Dee_UNICODE_ISPUNCT)
#define DeeBytes_IsGraph(self, start, end)   DeeBytes_TestTrait(self, start, end, Dee_UNICODE_ISGRAPH)
#define DeeBytes_IsPrint(self, start, end)   DeeBytes_TestTrait(self, start, end, Dee_UNICODE_ISPRINT)
#define DeeBytes_IsBlank(self, start, end)   DeeBytes_TestTrait(self, start, end, Dee_UNICODE_ISBLANK)
#define DeeBytes_IsNumeric(self, start, end) DeeBytes_TestTrait(self, start, end, Dee_UNICODE_ISNUMERIC)
#define DeeBytes_IsSymStrt(self, start, end) DeeBytes_TestTrait(self, start, end, Dee_UNICODE_ISSYMSTRT)
#define DeeBytes_IsSymCont(self, start, end) DeeBytes_TestTrait(self, start, end, Dee_UNICODE_ISSYMCONT)

#define DeeBytes_IsAnyCntrl(self, start, end)   DeeBytes_TestAnyTrait(self, start, end, Dee_UNICODE_ISCNTRL)
#define DeeBytes_IsAnyTab(self, start, end)     DeeBytes_TestAnyTrait(self, start, end, Dee_UNICODE_ISTAB)
#define DeeBytes_IsAnyCempty(self, start, end)  DeeBytes_TestAnyTrait(self, start, end, Dee_UNICODE_ISEMPTY)
#define DeeBytes_IsAnyWhite(self, start, end)   DeeBytes_TestAnyTrait(self, start, end, Dee_UNICODE_ISWHITE)
#define DeeBytes_IsAnyLF(self, start, end)      DeeBytes_TestAnyTrait(self, start, end, Dee_UNICODE_ISLF)
#define DeeBytes_IsAnySpace(self, start, end)   DeeBytes_TestAnyTrait(self, start, end, Dee_UNICODE_ISSPACE)
#define DeeBytes_IsAnyLower(self, start, end)   DeeBytes_TestAnyTrait(self, start, end, Dee_UNICODE_ISLOWER)
#define DeeBytes_IsAnyUpper(self, start, end)   DeeBytes_TestAnyTrait(self, start, end, Dee_UNICODE_ISUPPER)
#define DeeBytes_IsAnyAlpha(self, start, end)   DeeBytes_TestAnyTrait(self, start, end, Dee_UNICODE_ISALPHA)
#define DeeBytes_IsAnyDigit(self, start, end)   DeeBytes_TestAnyTrait(self, start, end, Dee_UNICODE_ISDIGIT)
#define DeeBytes_IsAnyHex(self, start, end)     DeeBytes_TestAnyTrait(self, start, end, Dee_UNICODE_ISHEX)
#define DeeBytes_IsAnyXdigit(self, start, end)  DeeBytes_TestAnyTrait(self, start, end, Dee_UNICODE_ISXDIGIT)
#define DeeBytes_IsAnyAlnum(self, start, end)   DeeBytes_TestAnyTrait(self, start, end, Dee_UNICODE_ISALNUM)
#define DeeBytes_IsAnyPunct(self, start, end)   DeeBytes_TestAnyTrait(self, start, end, Dee_UNICODE_ISPUNCT)
#define DeeBytes_IsAnyGraph(self, start, end)   DeeBytes_TestAnyTrait(self, start, end, Dee_UNICODE_ISGRAPH)
#define DeeBytes_IsAnyPrint(self, start, end)   DeeBytes_TestAnyTrait(self, start, end, Dee_UNICODE_ISPRINT)
#define DeeBytes_IsAnyBlank(self, start, end)   DeeBytes_TestAnyTrait(self, start, end, Dee_UNICODE_ISBLANK)
#define DeeBytes_IsAnyTitle(self, start, end)   DeeBytes_TestAnyTrait(self, start, end, Dee_UNICODE_ISTITLE)
#define DeeBytes_IsAnyNumeric(self, start, end) DeeBytes_TestAnyTrait(self, start, end, Dee_UNICODE_ISNUMERIC)
#define DeeBytes_IsAnySymStrt(self, start, end) DeeBytes_TestAnyTrait(self, start, end, Dee_UNICODE_ISSYMSTRT)
#define DeeBytes_IsAnySymCont(self, start, end) DeeBytes_TestAnyTrait(self, start, end, Dee_UNICODE_ISSYMCONT)

PRIVATE WUNUSED NONNULL((1)) bool DCALL
DeeBytes_TestTrait(Bytes *__restrict self,
                   size_t start, size_t end,
                   Dee_uniflag_t flags) {
	byte_t *data;
	size_t size;
	bool result = true;
	size = DeeBytes_SIZE(self);
	CLAMP_SUBSTR_NONEMPTY(&start, &end, &size, empty);
	data = DeeBytes_DATA(self) + start;
	do {
		byte_t byte = *data++;
		if (!(DeeUni_Flags(byte) & flags)) {
			result = false;
			break;
		}
	} while (--size);
empty:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) bool DCALL
DeeBytes_IsAscii(Bytes *__restrict self,
                 size_t start, size_t end) {
	byte_t *data;
	size_t size;
	bool result = true;
	size = DeeBytes_SIZE(self);
	CLAMP_SUBSTR_NONEMPTY(&start, &end, &size, empty);
	data = DeeBytes_DATA(self) + start;
	do {
		byte_t byte = *data++;
		if (byte > 0x7f) {
			result = false;
			break;
		}
	} while (--size);
empty:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) bool DCALL
DeeBytes_TestAnyTrait(Bytes *__restrict self,
                      size_t start, size_t end,
                      Dee_uniflag_t flags) {
	byte_t *data;
	size_t size;
	bool result = false;
	size = DeeBytes_SIZE(self);
	CLAMP_SUBSTR_NONEMPTY(&start, &end, &size, empty);
	data = DeeBytes_DATA(self) + start;
	do {
		byte_t byte = *data++;
		if (DeeUni_Flags(byte) & flags) {
			result = true;
			break;
		}
	} while (--size);
empty:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) bool DCALL
DeeBytes_IsAnyAscii(Bytes *__restrict self,
                    size_t start, size_t end) {
	byte_t *data;
	size_t size;
	bool result = false;
	size = DeeBytes_SIZE(self);
	CLAMP_SUBSTR_NONEMPTY(&start, &end, &size, empty);
	data = DeeBytes_DATA(self) + start;
	do {
		byte_t byte = *data++;
		if (byte > 0x7f) {
			result = true;
			break;
		}
	} while (--size);
empty:
	return result;
}


PRIVATE WUNUSED NONNULL((1)) bool DCALL
DeeBytes_IsTitle(Bytes *__restrict self,
                 size_t start, size_t end) {
	byte_t *data;
	size_t size;
	bool result = true;
	Dee_uniflag_t flags = (Dee_UNICODE_ISTITLE | Dee_UNICODE_ISUPPER | Dee_UNICODE_ISSPACE);
	size = DeeBytes_SIZE(self);
	CLAMP_SUBSTR_NONEMPTY(&start, &end, &size, empty);
	data = DeeBytes_DATA(self) + start;
	do {
		byte_t byte = *data++;
		Dee_uniflag_t f = DeeUni_Flags(byte);
		if (!(f & flags)) {
			result = false;
			break;
		}
		flags = (f & Dee_UNICODE_ISSPACE) ? (Dee_UNICODE_ISTITLE | Dee_UNICODE_ISUPPER | Dee_UNICODE_ISSPACE)
		                              : (Dee_UNICODE_ISLOWER | Dee_UNICODE_ISSPACE);
	} while (--size);
empty:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) bool DCALL
DeeBytes_IsSymbol(Bytes *__restrict self,
                  size_t start, size_t end) {
	byte_t *data;
	size_t size;
	bool result = true;
	Dee_uniflag_t flags = Dee_UNICODE_ISSYMSTRT;
	size = DeeBytes_SIZE(self);
	CLAMP_SUBSTR_NONEMPTY(&start, &end, &size, empty);
	data = DeeBytes_DATA(self) + start;
	do {
		byte_t byte = *data++;
		if (!(DeeUni_Flags(byte) & flags)) {
			result = false;
			break;
		}
		flags = Dee_UNICODE_ISSYMCONT;
	} while (--size);
empty:
	return result;
}

#define DEFINE_BYTES_TRAIT(name, function, test_ch)                    \
	PRIVATE WUNUSED DREF DeeObject *DCALL                              \
	bytes_##name(Bytes *self, size_t argc, DeeObject *const *argv) {   \
		size_t start = 0, end = (size_t)-1;                            \
		if (argc == 1) {                                               \
			byte_t ch;                                                 \
			size_t size;                                               \
			if unlikely(DeeObject_AsSize(argv[0], &start))             \
				goto err_maybe_overflow;                               \
			size = DeeBytes_SIZE(self);                                \
			if unlikely(start >= size) {                               \
				DeeRT_ErrIndexOutOfBounds(Dee_AsObject(self),          \
				                          start, size);                \
				goto err;                                              \
			}                                                          \
			ch = DeeBytes_DATA(self)[start];                           \
			return_bool(test_ch);                                      \
		} else {                                                       \
			DeeArg_Unpack0Or1XOr2X(err, argc, argv, #name,             \
		                           &start, UNPuSIZ, DeeObject_AsSize,  \
		                           &end, UNPxSIZ, DeeObject_AsSizeM1); \
			return_bool(function(self, start, end));                   \
		}                                                              \
	err:                                                               \
		return NULL;                                                   \
	err_maybe_overflow:                                                \
		DeeRT_ErrIndexOverflow(self);                                  \
		goto err;                                                      \
	}
#define DEFINE_ANY_BYTES_TRAIT(name, function)                                      \
	PRIVATE WUNUSED DREF DeeObject *DCALL                                           \
	bytes_##name(Bytes *self, size_t argc, DeeObject *const *argv, DeeObject *kw) { \
		size_t start = 0, end = (size_t)-1;                                         \
		if (DeeArg_UnpackKw(argc, argv, kw, kwlist__start_end,                      \
		                    "|" UNPuSIZ UNPxSIZ ":" #name, &start, &end))           \
			goto err;                                                               \
		return_bool(function(self, start, end));                                    \
	err:                                                                            \
		return NULL;                                                                \
	}
DEFINE_BYTES_TRAIT(iscntrl, DeeBytes_IsCntrl, DeeUni_IsCntrl(ch))
DEFINE_BYTES_TRAIT(istab, DeeBytes_IsTab, DeeUni_IsTab(ch))
DEFINE_BYTES_TRAIT(iscempty, DeeBytes_IsCempty, DeeUni_IsEmpty(ch))
DEFINE_BYTES_TRAIT(iswhite, DeeBytes_IsWhite, DeeUni_IsWhite(ch))
DEFINE_BYTES_TRAIT(islf, DeeBytes_IsLF, DeeUni_IsLF(ch))
DEFINE_BYTES_TRAIT(isspace, DeeBytes_IsSpace, DeeUni_IsSpace(ch))
DEFINE_BYTES_TRAIT(islower, DeeBytes_IsLower, DeeUni_IsLower(ch))
DEFINE_BYTES_TRAIT(isupper, DeeBytes_IsUpper, DeeUni_IsUpper(ch))
DEFINE_BYTES_TRAIT(isalpha, DeeBytes_IsAlpha, DeeUni_IsAlpha(ch))
DEFINE_BYTES_TRAIT(isdigit, DeeBytes_IsDigit, DeeUni_IsDigit(ch))
DEFINE_BYTES_TRAIT(ishex, DeeBytes_IsHex, DeeUni_IsHex(ch))
DEFINE_BYTES_TRAIT(isxdigit, DeeBytes_IsXdigit, DeeUni_IsXDigit(ch))
DEFINE_BYTES_TRAIT(isalnum, DeeBytes_IsAlnum, DeeUni_IsAlnum(ch))
DEFINE_BYTES_TRAIT(ispunct, DeeBytes_IsPunct, DeeUni_IsPunct(ch))
DEFINE_BYTES_TRAIT(isgraph, DeeBytes_IsGraph, DeeUni_IsGraph(ch))
DEFINE_BYTES_TRAIT(isprint, DeeBytes_IsPrint, DeeUni_IsPrint(ch))
DEFINE_BYTES_TRAIT(isblank, DeeBytes_IsBlank, DeeUni_IsBlank(ch))
DEFINE_BYTES_TRAIT(istitle, DeeBytes_IsTitle, DeeUni_IsTitle(ch))
DEFINE_BYTES_TRAIT(isnumeric, DeeBytes_IsNumeric, DeeUni_IsNumeric(ch))
DEFINE_BYTES_TRAIT(issymstrt, DeeBytes_IsSymStrt, DeeUni_IsSymStrt(ch))
DEFINE_BYTES_TRAIT(issymcont, DeeBytes_IsSymCont, DeeUni_IsSymCont(ch))
DEFINE_BYTES_TRAIT(issymbol, DeeBytes_IsSymbol, DeeUni_IsSymStrt(ch))
DEFINE_BYTES_TRAIT(isascii, DeeBytes_IsAscii, ch <= 0x7f)
DEFINE_ANY_BYTES_TRAIT(isanycntrl, DeeBytes_IsAnyCntrl)
DEFINE_ANY_BYTES_TRAIT(isanytab, DeeBytes_IsAnyTab)
DEFINE_ANY_BYTES_TRAIT(isanycempty, DeeBytes_IsAnyCempty)
DEFINE_ANY_BYTES_TRAIT(isanywhite, DeeBytes_IsAnyWhite)
DEFINE_ANY_BYTES_TRAIT(isanylf, DeeBytes_IsAnyLF)
DEFINE_ANY_BYTES_TRAIT(isanyspace, DeeBytes_IsAnySpace)
DEFINE_ANY_BYTES_TRAIT(isanylower, DeeBytes_IsAnyLower)
DEFINE_ANY_BYTES_TRAIT(isanyupper, DeeBytes_IsAnyUpper)
DEFINE_ANY_BYTES_TRAIT(isanyalpha, DeeBytes_IsAnyAlpha)
DEFINE_ANY_BYTES_TRAIT(isanydigit, DeeBytes_IsAnyDigit)
DEFINE_ANY_BYTES_TRAIT(isanyhex, DeeBytes_IsAnyHex)
DEFINE_ANY_BYTES_TRAIT(isanyxdigit, DeeBytes_IsAnyXdigit)
DEFINE_ANY_BYTES_TRAIT(isanyalnum, DeeBytes_IsAnyAlnum)
DEFINE_ANY_BYTES_TRAIT(isanypunct, DeeBytes_IsAnyPunct)
DEFINE_ANY_BYTES_TRAIT(isanygraph, DeeBytes_IsAnyGraph)
DEFINE_ANY_BYTES_TRAIT(isanyprint, DeeBytes_IsAnyPrint)
DEFINE_ANY_BYTES_TRAIT(isanyblank, DeeBytes_IsAnyBlank)
DEFINE_ANY_BYTES_TRAIT(isanytitle, DeeBytes_IsAnyTitle)
DEFINE_ANY_BYTES_TRAIT(isanynumeric, DeeBytes_IsAnyNumeric)
DEFINE_ANY_BYTES_TRAIT(isanysymstrt, DeeBytes_IsAnySymStrt)
DEFINE_ANY_BYTES_TRAIT(isanysymcont, DeeBytes_IsAnySymCont)
DEFINE_ANY_BYTES_TRAIT(isanyascii, DeeBytes_IsAnyAscii)
#undef DEFINE_ANY_BYTES_TRAIT
#undef DEFINE_BYTES_TRAIT

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_asdigit(Bytes *self, size_t argc, DeeObject *const *argv) {
	byte_t ch, digit;
	DeeObject *defl = NULL;
	if (argc == 0) {
		if unlikely(DeeBytes_SIZE(self) != 1)
			goto err_not_single_char;
		ch = DeeBytes_DATA(self)[0];
	} else {
		size_t index, size;
		DeeArg_Unpack1XOr2X(err, argc, argv, "asdigit",
		                    &index, UNPuSIZ, DeeObject_AsSize,
		                    &defl, "o", _DeeArg_AsObject);
		size = DeeBytes_SIZE(self);
		if unlikely(index >= size) {
			DeeRT_ErrIndexOutOfBounds(Dee_AsObject(self), index, size);
			goto err;
		}
		ch = DeeBytes_DATA(self)[index];
	}
	digit = DeeUni_AsDigitVal(ch);
	if likely(digit < 10)
		return DeeInt_NEWU(digit);
	if (defl)
		return_reference_(defl);
	DeeError_Throwf(&DeeError_ValueError,
	                "Unicode character %I8C isn't a digit",
	                ch);
	goto err;
err_not_single_char:
	err_expected_single_character_string(Dee_AsObject(self));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_asxdigit(Bytes *self, size_t argc, DeeObject *const *argv) {
	byte_t ch, digit;
	DeeObject *defl = NULL;
	if (argc == 0) {
		if unlikely(DeeBytes_SIZE(self) != 1)
			goto err_not_single_char;
		ch = DeeBytes_DATA(self)[0];
	} else {
		size_t index, size;
		DeeArg_Unpack1XOr2X(err, argc, argv, "asxdigit",
		                    &index, UNPuSIZ, DeeObject_AsSize,
		                    &defl, "o", _DeeArg_AsObject);
		size = DeeBytes_SIZE(self);
		if unlikely(index >= size) {
			DeeRT_ErrIndexOutOfBounds(Dee_AsObject(self), index, size);
			goto err;
		}
		ch = DeeBytes_DATA(self)[index];
	}
	digit = DeeUni_AsDigitVal(ch);
	if likely(digit != 0xff)
		return DeeInt_NEWU(digit);
	if (defl)
		return_reference_(defl);
	DeeError_Throwf(&DeeError_ValueError,
	                "Unicode character %I8C isn't a hex-digit",
	                ch);
	goto err;
err_not_single_char:
	err_expected_single_character_string(Dee_AsObject(self));
err:
	return NULL;
}

PRIVATE WUNUSED DREF Bytes *DCALL
bytes_lower(Bytes *self, size_t argc,
            DeeObject *const *argv, DeeObject *kw) {
	size_t size;
	DREF Bytes *result;
	byte_t *dst;
	byte_t const *src;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("lower", params: "
	size_t start = 0, size_t end = (size_t)-1
", docStringPrefix: "bytes");]]]*/
#define bytes_lower_params "start=!0,end=!-1"
	struct {
		size_t start;
		size_t end;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__start_end, "|" UNPuSIZ UNPxSIZ ":lower", &args))
		goto err;
/*[[[end]]]*/
	size = DeeBytes_SIZE(self);
	CLAMP_SUBSTR_NONEMPTY(&args.start, &args.end, &size, empty);
	result = DeeBytes_NewBufferUninitialized(size);
	if unlikely(!result)
		goto err;
	dst = DeeBytes_BUFFER_DATA(result);
	src = DeeBytes_DATA(self);
	do {
		byte_t byte = *src++;
		*dst++ = (byte_t)DeeUni_ToLower(byte);
	} while (--size);
	return result;
empty:
	return_reference_((DREF Bytes *)Dee_EmptyBytes);
err:
	return NULL;
}

PRIVATE WUNUSED DREF Bytes *DCALL
bytes_upper(Bytes *self, size_t argc,
            DeeObject *const *argv, DeeObject *kw) {
	size_t size;
	DREF Bytes *result;
	byte_t *dst;
	byte_t const *src;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("upper", params: "
	size_t start = 0, size_t end = (size_t)-1
", docStringPrefix: "bytes");]]]*/
#define bytes_upper_params "start=!0,end=!-1"
	struct {
		size_t start;
		size_t end;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__start_end, "|" UNPuSIZ UNPxSIZ ":upper", &args))
		goto err;
/*[[[end]]]*/
	size = DeeBytes_SIZE(self);
	CLAMP_SUBSTR_NONEMPTY(&args.start, &args.end, &size, empty);
	result = DeeBytes_NewBufferUninitialized(size);
	if unlikely(!result)
		goto err;
	dst = DeeBytes_BUFFER_DATA(result);
	src = DeeBytes_DATA(self);
	do {
		byte_t byte = *src++;
		*dst++ = (byte_t)DeeUni_ToUpper(byte);
	} while (--size);
	return result;
empty:
	return_reference_((DREF Bytes *)Dee_EmptyBytes);
err:
	return NULL;
}

PRIVATE WUNUSED DREF Bytes *DCALL
bytes_title(Bytes *self, size_t argc,
            DeeObject *const *argv, DeeObject *kw) {
	size_t size;
	DREF Bytes *result;
	byte_t *dst;
	byte_t const *src;
	uintptr_t kind = Dee_UNICODE_CONVERT_TITLE;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("title", params: "
	size_t start = 0, size_t end = (size_t)-1
", docStringPrefix: "bytes");]]]*/
#define bytes_title_params "start=!0,end=!-1"
	struct {
		size_t start;
		size_t end;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__start_end, "|" UNPuSIZ UNPxSIZ ":title", &args))
		goto err;
/*[[[end]]]*/
	size = DeeBytes_SIZE(self);
	CLAMP_SUBSTR_NONEMPTY(&args.start, &args.end, &size, empty);
	result = DeeBytes_NewBufferUninitialized(size);
	if unlikely(!result)
		goto err;
	dst = DeeBytes_BUFFER_DATA(result);
	src = DeeBytes_DATA(self);
	do {
		byte_t byte = *src++;
		*dst++ = kind == Dee_UNICODE_CONVERT_TITLE
		         ? (byte_t)DeeUni_ToTitle(byte)
		         : (byte_t)DeeUni_ToLower(byte);
		kind = DeeUni_IsSpace(byte) ? Dee_UNICODE_CONVERT_TITLE : Dee_UNICODE_CONVERT_LOWER;
	} while (--size);
	return result;
empty:
	return_reference_((DREF Bytes *)Dee_EmptyBytes);
err:
	return NULL;
}

PRIVATE WUNUSED DREF Bytes *DCALL
bytes_capitalize(Bytes *self, size_t argc,
                 DeeObject *const *argv, DeeObject *kw) {
	size_t size;
	DREF Bytes *result;
	byte_t *dst, byte;
	byte_t const *src;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("capitalize", params: "
	size_t start = 0, size_t end = (size_t)-1
", docStringPrefix: "bytes");]]]*/
#define bytes_capitalize_params "start=!0,end=!-1"
	struct {
		size_t start;
		size_t end;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__start_end, "|" UNPuSIZ UNPxSIZ ":capitalize", &args))
		goto err;
/*[[[end]]]*/
	size = DeeBytes_SIZE(self);
	CLAMP_SUBSTR_NONEMPTY(&args.start, &args.end, &size, empty);
	result = DeeBytes_NewBufferUninitialized(size);
	if unlikely(!result)
		goto err;
	dst = DeeBytes_BUFFER_DATA(result);
	src = DeeBytes_DATA(self);
	byte = *src++;
	*dst++ = (byte_t)DeeUni_ToUpper(byte);
	while (--size) {
		byte = *src++;
		*dst++ = (byte_t)DeeUni_ToLower(byte);
	}
	return result;
empty:
	return_reference_((DREF Bytes *)Dee_EmptyBytes);
err:
	return NULL;
}

PRIVATE WUNUSED DREF Bytes *DCALL
bytes_swapcase(Bytes *self, size_t argc,
               DeeObject *const *argv, DeeObject *kw) {
	size_t size;
	DREF Bytes *result;
	byte_t *dst;
	byte_t const *src;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("swapcase", params: "
	size_t start = 0, size_t end = (size_t)-1
", docStringPrefix: "bytes");]]]*/
#define bytes_swapcase_params "start=!0,end=!-1"
	struct {
		size_t start;
		size_t end;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__start_end, "|" UNPuSIZ UNPxSIZ ":swapcase", &args))
		goto err;
/*[[[end]]]*/
	size = DeeBytes_SIZE(self);
	CLAMP_SUBSTR_NONEMPTY(&args.start, &args.end, &size, empty);
	result = DeeBytes_NewBufferUninitialized(size);
	if unlikely(!result)
		goto err;
	dst = DeeBytes_BUFFER_DATA(result);
	src = DeeBytes_DATA(self);
	do {
		byte_t byte = *src++;
		*dst++ = (byte_t)DeeUni_SwapCase(byte);
	} while (--size);
	return result;
empty:
	return_reference_((DREF Bytes *)Dee_EmptyBytes);
err:
	return NULL;
}



PRIVATE WUNUSED DREF Bytes *DCALL
bytes_tolower(Bytes *self, size_t argc,
              DeeObject *const *argv, DeeObject *kw) {
	size_t size;
	byte_t *iter;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("tolower", params: "
	size_t start = 0, size_t end = (size_t)-1
", docStringPrefix: "bytes");]]]*/
#define bytes_tolower_params "start=!0,end=!-1"
	struct {
		size_t start;
		size_t end;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__start_end, "|" UNPuSIZ UNPxSIZ ":tolower", &args))
		goto err;
/*[[[end]]]*/
	size = DeeBytes_SIZE(self);
	CLAMP_SUBSTR_NONEMPTY(&args.start, &args.end, &size, empty);
	if unlikely(!DeeBytes_IsWritable(self)) {
		err_bytes_not_writable(Dee_AsObject(self));
		goto err;
	}
	iter = DeeBytes_DATA(self) + args.start;
	do {
		byte_t byte = *iter;
		*iter = (byte_t)DeeUni_ToLower(byte);
		++iter;
	} while (--size);
empty:
	return_reference_(self);
err:
	return NULL;
}

PRIVATE WUNUSED DREF Bytes *DCALL
bytes_toupper(Bytes *self, size_t argc,
              DeeObject *const *argv, DeeObject *kw) {
	size_t size;
	byte_t *iter;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("toupper", params: "
	size_t start = 0, size_t end = (size_t)-1
", docStringPrefix: "bytes");]]]*/
#define bytes_toupper_params "start=!0,end=!-1"
	struct {
		size_t start;
		size_t end;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__start_end, "|" UNPuSIZ UNPxSIZ ":toupper", &args))
		goto err;
/*[[[end]]]*/
	size = DeeBytes_SIZE(self);
	CLAMP_SUBSTR_NONEMPTY(&args.start, &args.end, &size, empty);
	if unlikely(!DeeBytes_IsWritable(self)) {
		err_bytes_not_writable(Dee_AsObject(self));
		goto err;
	}
	iter = DeeBytes_DATA(self) + args.start;
	do {
		byte_t byte = *iter;
		*iter = (byte_t)DeeUni_ToUpper(byte);
		++iter;
	} while (--size);
empty:
	return_reference_(self);
err:
	return NULL;
}

PRIVATE WUNUSED DREF Bytes *DCALL
bytes_totitle(Bytes *self, size_t argc,
              DeeObject *const *argv, DeeObject *kw) {
	size_t size;
	byte_t *iter;
	uintptr_t kind = Dee_UNICODE_CONVERT_TITLE;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("totitle", params: "
	size_t start = 0, size_t end = (size_t)-1
", docStringPrefix: "bytes");]]]*/
#define bytes_totitle_params "start=!0,end=!-1"
	struct {
		size_t start;
		size_t end;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__start_end, "|" UNPuSIZ UNPxSIZ ":totitle", &args))
		goto err;
/*[[[end]]]*/
	size = DeeBytes_SIZE(self);
	CLAMP_SUBSTR_NONEMPTY(&args.start, &args.end, &size, empty);
	if unlikely(!DeeBytes_IsWritable(self)) {
		err_bytes_not_writable(Dee_AsObject(self));
		goto err;
	}
	iter = DeeBytes_DATA(self) + args.start;
	do {
		byte_t byte = *iter;
		*iter = kind == Dee_UNICODE_CONVERT_TITLE
		        ? (byte_t)DeeUni_ToTitle(byte)
		        : (byte_t)DeeUni_ToLower(byte);
		kind = DeeUni_IsSpace(byte) ? Dee_UNICODE_CONVERT_TITLE : Dee_UNICODE_CONVERT_LOWER;
		++iter;
	} while (--size);
empty:
	return_reference_(self);
err:
	return NULL;
}

PRIVATE WUNUSED DREF Bytes *DCALL
bytes_tocapitalize(Bytes *self, size_t argc,
                   DeeObject *const *argv, DeeObject *kw) {
	size_t size;
	byte_t *iter, byte;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("tocapitalize", params: "
	size_t start = 0, size_t end = (size_t)-1
", docStringPrefix: "bytes");]]]*/
#define bytes_tocapitalize_params "start=!0,end=!-1"
	struct {
		size_t start;
		size_t end;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__start_end, "|" UNPuSIZ UNPxSIZ ":tocapitalize", &args))
		goto err;
/*[[[end]]]*/
	size = DeeBytes_SIZE(self);
	CLAMP_SUBSTR_NONEMPTY(&args.start, &args.end, &size, empty);
	if unlikely(!DeeBytes_IsWritable(self)) {
		err_bytes_not_writable(Dee_AsObject(self));
		goto err;
	}
	iter = DeeBytes_DATA(self) + args.start;
	byte = *iter;
	*iter = (byte_t)DeeUni_ToUpper(byte);
	++iter;
	while (--size) {
		byte = *iter;
		*iter = (byte_t)DeeUni_ToLower(byte);
		++iter;
	}
empty:
	return_reference_(self);
err:
	return NULL;
}

PRIVATE WUNUSED DREF Bytes *DCALL
bytes_toswapcase(Bytes *self, size_t argc,
                 DeeObject *const *argv, DeeObject *kw) {
	size_t size;
	byte_t *iter;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("toswapcase", params: "
	size_t start = 0, size_t end = (size_t)-1
", docStringPrefix: "bytes");]]]*/
#define bytes_toswapcase_params "start=!0,end=!-1"
	struct {
		size_t start;
		size_t end;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__start_end, "|" UNPuSIZ UNPxSIZ ":toswapcase", &args))
		goto err;
/*[[[end]]]*/
	size = DeeBytes_SIZE(self);
	CLAMP_SUBSTR_NONEMPTY(&args.start, &args.end, &size, empty);
	if unlikely(!DeeBytes_IsWritable(self)) {
		err_bytes_not_writable(Dee_AsObject(self));
		goto err;
	}
	iter = DeeBytes_DATA(self) + args.start;
	do {
		byte_t byte = *iter;
		*iter = (byte_t)DeeUni_SwapCase(byte);
		++iter;
	} while (--size);
empty:
	return_reference_(self);
err:
	return NULL;
}

PRIVATE WUNUSED DREF Bytes *DCALL
bytes_replace(Bytes *self, size_t argc,
              DeeObject *const *argv, DeeObject *kw) {
	byte_t *begin, *end, *block_begin;
	Needle find_needle, replace_needle;
	struct Dee_bytes_printer printer;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("replace", params: "
	find: ?X3?DBytes?Dstring?Dint;
	replace: ?X3?DBytes?Dstring?Dint;
	size_t max = (size_t)-1;
", docStringPrefix: "bytes");]]]*/
#define bytes_replace_params "find:?X3?.?Dstring?Dint,replace:?X3?.?Dstring?Dint,max=!-1"
	struct {
		DeeObject *find;
		DeeObject *replace;
		size_t max_;
	} args;
	args.max_ = (size_t)-1;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__find_replace_max, "oo|" UNPxSIZ ":replace", &args))
		goto err;
/*[[[end]]]*/
	if (acquire_needle(&find_needle, args.find))
		goto err;
	if (acquire_needle(&replace_needle, args.replace))
		goto err;
	/* Handle special cases. */
	if unlikely(find_needle.n_size > DeeBytes_SIZE(self))
		goto return_self;
	if unlikely(!find_needle.n_size) {
		DREF Bytes *result;
		if (DeeBytes_SIZE(self))
			goto return_self;
		result = DeeBytes_NewBufferData(replace_needle.n_data, replace_needle.n_size);
		return result;
	}
	Dee_bytes_printer_init(&printer);
	begin       = DeeBytes_DATA(self);
	end         = begin + (DeeBytes_SIZE(self) - (find_needle.n_size - 1));
	block_begin = begin;
	if likely(args.max_) {
		while (begin < end) {
			if (MEMEQB(begin, find_needle.n_data, find_needle.n_size)) {
				/* Found one */
				if unlikely(Dee_bytes_printer_append(&printer, block_begin, (size_t)(begin - block_begin)) < 0)
					goto err_printer;
				if unlikely(Dee_bytes_printer_append(&printer, replace_needle.n_data, replace_needle.n_size) < 0)
					goto err_printer;
				begin += find_needle.n_size;
				block_begin = begin;
				if (begin >= end)
					break;
				if unlikely(!--args.max_)
					break;
				continue;
			}
			++begin;
		}
	}
	end += (find_needle.n_size - 1);
	ASSERT(block_begin <= end);
#ifndef __OPTIMIZE_SIZE__
	if unlikely(block_begin == DeeBytes_DATA(self)) {
		Dee_bytes_printer_fini(&printer);
		goto return_self;
	}
#endif /* !__OPTIMIZE_SIZE__ */
	if unlikely(Dee_bytes_printer_append(&printer, block_begin,
	                                 (size_t)(end - block_begin)) < 0)
		goto err_printer;
	/* Pack together a Bytes object. */
	return (DREF Bytes *)Dee_bytes_printer_pack(&printer);
err_printer:
	Dee_bytes_printer_fini(&printer);
err:
	return NULL;
return_self:
	return_reference_(self);
}

PRIVATE WUNUSED DREF Bytes *DCALL
bytes_casereplace(Bytes *self, size_t argc,
                  DeeObject *const *argv, DeeObject *kw) {
	byte_t *begin, *end, *block_begin;
	Needle find_needle, replace_needle;
	struct Dee_bytes_printer printer;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("casereplace", params: "
	find: ?X3?DBytes?Dstring?Dint;
	replace: ?X3?DBytes?Dstring?Dint;
	size_t max = (size_t)-1;
", docStringPrefix: "bytes");]]]*/
#define bytes_casereplace_params "find:?X3?.?Dstring?Dint,replace:?X3?.?Dstring?Dint,max=!-1"
	struct {
		DeeObject *find;
		DeeObject *replace;
		size_t max_;
	} args;
	args.max_ = (size_t)-1;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__find_replace_max, "oo|" UNPxSIZ ":casereplace", &args))
		goto err;
/*[[[end]]]*/
	if (acquire_needle(&find_needle, args.find))
		goto err;
	if (acquire_needle(&replace_needle, args.replace))
		goto err;
	/* Handle special cases. */
	if unlikely(find_needle.n_size > DeeBytes_SIZE(self))
		goto return_self;
	if unlikely(!find_needle.n_size) {
		DREF Bytes *result;
		if (DeeBytes_SIZE(self))
			goto return_self;
		result = DeeBytes_NewBufferData(replace_needle.n_data, replace_needle.n_size);
		return result;
	}
	Dee_bytes_printer_init(&printer);
	begin       = DeeBytes_DATA(self);
	end         = begin + (DeeBytes_SIZE(self) - (find_needle.n_size - 1));
	block_begin = begin;
	if likely(args.max_) {
		while (begin < end) {
			if (memcasecmp(begin, find_needle.n_data, find_needle.n_size) == 0) {
				/* Found one */
				if unlikely(Dee_bytes_printer_append(&printer, block_begin, (size_t)(begin - block_begin)) < 0)
					goto err_printer;
				if unlikely(Dee_bytes_printer_append(&printer, replace_needle.n_data, replace_needle.n_size) < 0)
					goto err_printer;
				begin += find_needle.n_size;
				block_begin = begin;
				if (begin >= end)
					break;
				if unlikely(!--args.max_)
					break;
				continue;
			}
			++begin;
		}
	}
	end += (find_needle.n_size - 1);
	ASSERT(block_begin <= end);
#ifndef __OPTIMIZE_SIZE__
	if unlikely(block_begin == DeeBytes_DATA(self)) {
		Dee_bytes_printer_fini(&printer);
		goto return_self;
	}
#endif /* !__OPTIMIZE_SIZE__ */
	if unlikely(Dee_bytes_printer_append(&printer, block_begin,
	                                 (size_t)(end - block_begin)) < 0)
		goto err_printer;
	/* Pack together a Bytes object. */
	return (DREF Bytes *)Dee_bytes_printer_pack(&printer);
err_printer:
	Dee_bytes_printer_fini(&printer);
err:
	return NULL;
return_self:
	return_reference_(self);
}


PRIVATE WUNUSED DREF Bytes *DCALL
bytes_toreplace(Bytes *self, size_t argc,
                DeeObject *const *argv, DeeObject *kw) {
	Needle find_needle, replace_needle;
	byte_t *begin, *end;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("toreplace", params: "
	find: ?X3?DBytes?Dstring?Dint;
	replace: ?X3?DBytes?Dstring?Dint;
	size_t max = (size_t)-1;
", docStringPrefix: "bytes");]]]*/
#define bytes_toreplace_params "find:?X3?.?Dstring?Dint,replace:?X3?.?Dstring?Dint,max=!-1"
	struct {
		DeeObject *find;
		DeeObject *replace;
		size_t max_;
	} args;
	args.max_ = (size_t)-1;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__find_replace_max, "oo|" UNPxSIZ ":toreplace", &args))
		goto err;
/*[[[end]]]*/
	if (acquire_needle(&find_needle, args.find))
		goto err;
	if (acquire_needle(&replace_needle, args.replace))
		goto err;
	if unlikely(find_needle.n_size != replace_needle.n_size) {
		DeeError_Throwf(&DeeError_ValueError,
		                "Find(%" PRFuSIZ ") and replace(%" PRFuSIZ ") needles have different sizes",
		                find_needle.n_size, replace_needle.n_size);
		goto err;
	}
	if unlikely(!DeeBytes_IsWritable(self)) {
		err_bytes_not_writable(Dee_AsObject(self));
		goto err;
	}

	/* Handle special cases. */
	if unlikely(find_needle.n_size > DeeBytes_SIZE(self))
		goto done;
	if unlikely(!find_needle.n_size)
		goto done;

	end = (begin = DeeBytes_DATA(self)) + (DeeBytes_SIZE(self) - (find_needle.n_size - 1));
	if likely(args.max_) {
		while (begin < end) {
			if (MEMEQB(begin, find_needle.n_data, find_needle.n_size)) {
				/* Found one */
				memcpy(begin, replace_needle.n_data, replace_needle.n_size);
				begin += find_needle.n_size;
				if (begin >= end)
					break;
				if unlikely(!--args.max_)
					break;
				continue;
			}
			++begin;
		}
	}
done:
	return_reference_(self);
err:
	return NULL;
}

PRIVATE WUNUSED DREF Bytes *DCALL
bytes_tocasereplace(Bytes *self, size_t argc,
                    DeeObject *const *argv, DeeObject *kw) {
	Needle find_needle, replace_needle;
	byte_t *begin, *end;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("tocasereplace", params: "
	find: ?X3?DBytes?Dstring?Dint;
	replace: ?X3?DBytes?Dstring?Dint;
	size_t max = (size_t)-1;
", docStringPrefix: "bytes");]]]*/
#define bytes_tocasereplace_params "find:?X3?.?Dstring?Dint,replace:?X3?.?Dstring?Dint,max=!-1"
	struct {
		DeeObject *find;
		DeeObject *replace;
		size_t max_;
	} args;
	args.max_ = (size_t)-1;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__find_replace_max, "oo|" UNPxSIZ ":tocasereplace", &args))
		goto err;
/*[[[end]]]*/
	if (acquire_needle(&find_needle, args.find))
		goto err;
	if (acquire_needle(&replace_needle, args.replace))
		goto err;
	if unlikely(find_needle.n_size != replace_needle.n_size) {
		DeeError_Throwf(&DeeError_ValueError,
		                "Find(%" PRFuSIZ ") and replace(%" PRFuSIZ ") needles have different sizes",
		                find_needle.n_size, replace_needle.n_size);
		goto err;
	}
	if unlikely(!DeeBytes_IsWritable(self)) {
		err_bytes_not_writable(Dee_AsObject(self));
		goto err;
	}

	/* Handle special cases. */
	if unlikely(find_needle.n_size > DeeBytes_SIZE(self))
		goto done;
	if unlikely(!find_needle.n_size)
		goto done;

	end = (begin = DeeBytes_DATA(self)) + (DeeBytes_SIZE(self) - (find_needle.n_size - 1));
	if likely(args.max_) {
		while (begin < end) {
			if (memcasecmp(begin, find_needle.n_data, find_needle.n_size) == 0) {
				/* Found one */
				memcpy(begin, replace_needle.n_data, replace_needle.n_size);
				begin += find_needle.n_size;
				if (begin >= end)
					break;
				if unlikely(!--args.max_)
					break;
				continue;
			}
			++begin;
		}
	}
done:
	return_reference_(self);
err:
	return NULL;
}


/* The string decode() and encode() member functions also function for `Bytes' objects.
 * As a matter of fact: they'd work for any kind of object, however built-in
 *                      codecs only function for bytes and string objects! */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_decode(DeeObject *self, size_t argc,
              DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_encode(DeeObject *self, size_t argc,
              DeeObject *const *argv, DeeObject *kw);


INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeBytes_SplitByte(Bytes *__restrict self, byte_t sep);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeBytes_Split(Bytes *self, DeeObject *sep);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeBytes_CaseSplitByte(Bytes *__restrict self, byte_t sep);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeBytes_CaseSplit(Bytes *self, DeeObject *sep);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeBytes_SplitLines(Bytes *__restrict self, bool keepends);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeBytes_FindAll(Bytes *self, DeeObject *needle,
                 size_t start, size_t end,
                 bool overlapping);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeBytes_CaseFindAll(Bytes *self, DeeObject *needle,
                     size_t start, size_t end,
                     bool overlapping);

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_findall(Bytes *self, size_t argc,
              DeeObject *const *argv, DeeObject *kw) {
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("findall", params: "
	DeeObject *needle: ?X3?DBytes?Dstring?Dint, size_t start = 0, size_t end = (size_t)-1, bool overlapping = false
", docStringPrefix: "bytes");]]]*/
#define bytes_findall_params "needle:?X3?.?Dstring?Dint,start=!0,end=!-1,overlapping=!f"
	struct {
		DeeObject *needle;
		size_t start;
		size_t end;
		bool overlapping;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	args.overlapping = false;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__needle_start_end_overlapping, "o|" UNPuSIZ UNPxSIZ "b:findall", &args))
		goto err;
/*[[[end]]]*/
	return DeeBytes_FindAll(self, args.needle, args.start, args.end, args.overlapping);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_casefindall(Bytes *self, size_t argc,
                  DeeObject *const *argv, DeeObject *kw) {
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("casefindall", params: "
	DeeObject *needle: ?X3?DBytes?Dstring?Dint, size_t start = 0, size_t end = (size_t)-1, bool overlapping = false
", docStringPrefix: "bytes");]]]*/
#define bytes_casefindall_params "needle:?X3?.?Dstring?Dint,start=!0,end=!-1,overlapping=!f"
	struct {
		DeeObject *needle;
		size_t start;
		size_t end;
		bool overlapping;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	args.overlapping = false;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__needle_start_end_overlapping, "o|" UNPuSIZ UNPxSIZ "b:casefindall", &args))
		goto err;
/*[[[end]]]*/
	return DeeBytes_CaseFindAll(self, args.needle, args.start, args.end, args.overlapping);
err:
	return NULL;
}

struct bytes_join_data {
	struct Dee_bytes_printer bjd_out;   /* Output printer */
	Bytes                   *bjd_sep;   /* [1..1] Separator */
	bool                     bjd_first; /* True if this is the first element. */
};

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
bytes_join_cb(void *arg, DeeObject *elem) {
	struct bytes_join_data *data = (struct bytes_join_data *)arg;
	/* Print `self' prior to every object, starting with the 2nd one. */
	if (!data->bjd_first) {
		if unlikely(Dee_bytes_printer_append(&data->bjd_out,
		                                 DeeBytes_DATA(data->bjd_sep),
		                                 DeeBytes_SIZE(data->bjd_sep)) < 0)
			goto err;
	}
	if unlikely(Dee_bytes_printer_printobject(&data->bjd_out, elem) < 0)
		goto err;
	data->bjd_first = false;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_join(Bytes *self, size_t argc, DeeObject *const *argv) {
	struct bytes_join_data data;
	Dee_ssize_t status;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("join", params: "
	seq: ?S?O
", docStringPrefix: "bytes");]]]*/
#define bytes_join_params "seq:?S?O"
	struct {
		DeeObject *seq;
	} args;
	DeeArg_Unpack1(err, argc, argv, "join", &args.seq);
/*[[[end]]]*/
	Dee_bytes_printer_init(&data.bjd_out);
	data.bjd_sep   = self;
	data.bjd_first = true;
	status = DeeObject_Foreach(args.seq, &bytes_join_cb, &data);
	if unlikely(status < 0)
		goto err_printer;
	return Dee_bytes_printer_pack(&data.bjd_out);
err_printer:
	Dee_bytes_printer_fini(&data.bjd_out);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_split(Bytes *self, size_t argc, DeeObject *const *argv) {
	byte_t bsep;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("split", params: "
	DeeObject *sep: ?X3?DBytes?Dstring?Dint
", docStringPrefix: "bytes");]]]*/
#define bytes_split_params "sep:?X3?.?Dstring?Dint"
	struct {
		DeeObject *sep;
	} args;
	DeeArg_Unpack1(err, argc, argv, "split", &args.sep);
/*[[[end]]]*/
	if (DeeString_Check(args.sep) || DeeBytes_Check(args.sep))
		return DeeBytes_Split(self, args.sep);
	if (DeeObject_AsByte(args.sep, &bsep))
		goto err;
	return DeeBytes_SplitByte(self, bsep);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_casesplit(Bytes *self, size_t argc, DeeObject *const *argv) {
	byte_t bsep;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("casesplit", params: "
	DeeObject *sep: ?X3?DBytes?Dstring?Dint
", docStringPrefix: "bytes");]]]*/
#define bytes_casesplit_params "sep:?X3?.?Dstring?Dint"
	struct {
		DeeObject *sep;
	} args;
	DeeArg_Unpack1(err, argc, argv, "casesplit", &args.sep);
/*[[[end]]]*/
	if (DeeString_Check(args.sep) || DeeBytes_Check(args.sep))
		return DeeBytes_CaseSplit(self, args.sep);
	if (DeeObject_AsByte(args.sep, &bsep))
		goto err;
	return DeeBytes_CaseSplitByte(self, bsep);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_splitlines(Bytes *self, size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("splitlines", params: "
	bool keepends = false;
", docStringPrefix: "bytes");]]]*/
#define bytes_splitlines_params "keepends=!f"
	struct {
		bool keepends;
	} args;
	args.keepends = false;
	DeeArg_Unpack0Or1X(err, argc, argv, "splitlines", &args.keepends, "b", DeeObject_AsBool);
/*[[[end]]]*/
	return DeeBytes_SplitLines(self, args.keepends);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_startswith(Bytes *self, size_t argc,
                 DeeObject *const *argv, DeeObject *kw) {
	bool result;
	Needle needle;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("startswith", params: "
	DeeObject *needle: ?X3?DBytes?Dstring?Dint, size_t start = 0, size_t end = (size_t)-1
", docStringPrefix: "bytes");]]]*/
#define bytes_startswith_params "needle:?X3?.?Dstring?Dint,start=!0,end=!-1"
	struct {
		DeeObject *needle;
		size_t start;
		size_t end;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__needle_start_end, "o|" UNPuSIZ UNPxSIZ ":startswith", &args))
		goto err;
/*[[[end]]]*/
	if (acquire_needle(&needle, args.needle))
		goto err;
	if (args.end > DeeBytes_SIZE(self))
		args.end = DeeBytes_SIZE(self);
	if (args.start > args.end || (args.end - args.start) < needle.n_size) {
		result = false;
	} else {
		result = MEMEQB(DeeBytes_DATA(self) + args.start,
		                needle.n_data, needle.n_size);
	}
	return_bool(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_casestartswith(Bytes *self, size_t argc,
                     DeeObject *const *argv, DeeObject *kw) {
	bool result;
	Needle needle;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("casestartswith", params: "
	DeeObject *needle: ?X3?DBytes?Dstring?Dint, size_t start = 0, size_t end = (size_t)-1
", docStringPrefix: "bytes");]]]*/
#define bytes_casestartswith_params "needle:?X3?.?Dstring?Dint,start=!0,end=!-1"
	struct {
		DeeObject *needle;
		size_t start;
		size_t end;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__needle_start_end, "o|" UNPuSIZ UNPxSIZ ":casestartswith", &args))
		goto err;
/*[[[end]]]*/
	if (acquire_needle(&needle, args.needle))
		goto err;
	if (args.end > DeeBytes_SIZE(self))
		args.end = DeeBytes_SIZE(self);
	if (args.start > args.end || (args.end - args.start) < needle.n_size) {
		result = false;
	} else {
		result = memasciicaseeq(DeeBytes_DATA(self) + args.start,
		                        needle.n_data, needle.n_size);
	}
	return_bool(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_endswith(Bytes *self, size_t argc,
               DeeObject *const *argv, DeeObject *kw) {
	bool result;
	Needle needle;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("endswith", params: "
	DeeObject *needle: ?X3?DBytes?Dstring?Dint, size_t start = 0, size_t end = (size_t)-1
", docStringPrefix: "bytes");]]]*/
#define bytes_endswith_params "needle:?X3?.?Dstring?Dint,start=!0,end=!-1"
	struct {
		DeeObject *needle;
		size_t start;
		size_t end;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__needle_start_end, "o|" UNPuSIZ UNPxSIZ ":endswith", &args))
		goto err;
/*[[[end]]]*/
	if (acquire_needle(&needle, args.needle))
		goto err;
	if (args.end > DeeBytes_SIZE(self))
		args.end = DeeBytes_SIZE(self);
	if (args.start > args.end || (args.end - args.start) < needle.n_size) {
		result = false;
	} else {
		result = MEMEQB(DeeBytes_DATA(self) +
		                (args.end - needle.n_size),
		                needle.n_data, needle.n_size);
	}
	return_bool(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_caseendswith(Bytes *self, size_t argc,
                   DeeObject *const *argv, DeeObject *kw) {
	bool result;
	Needle needle;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("caseendswith", params: "
	DeeObject *needle: ?X3?DBytes?Dstring?Dint, size_t start = 0, size_t end = (size_t)-1
", docStringPrefix: "bytes");]]]*/
#define bytes_caseendswith_params "needle:?X3?.?Dstring?Dint,start=!0,end=!-1"
	struct {
		DeeObject *needle;
		size_t start;
		size_t end;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__needle_start_end, "o|" UNPuSIZ UNPxSIZ ":caseendswith", &args))
		goto err;
/*[[[end]]]*/
	if (acquire_needle(&needle, args.needle))
		goto err;
	if (args.end > DeeBytes_SIZE(self))
		args.end = DeeBytes_SIZE(self);
	if (args.start > args.end || (args.end - args.start) < needle.n_size) {
		result = false;
	} else {
		result = memasciicaseeq(DeeBytes_DATA(self) +
		                        (args.end - needle.n_size),
		                        needle.n_data, needle.n_size);
	}
	return_bool(result);
err:
	return NULL;
}

LOCAL WUNUSED NONNULL((1, 3)) DREF DeeTupleObject *DCALL
bytes_pack_partition(Bytes *self, byte_t *find_ptr,
                     byte_t *start_ptr, size_t search_size,
                     size_t needle_len, bool is_rpartition) {
	DREF DeeTupleObject *result;
	result = DeeTuple_NewUninitialized(3);
	if unlikely(!result)
		goto done;
	if (!find_ptr) {
		Dee_Incref(self);
		Dee_Incref_n(Dee_EmptyBytes, 2);
		DeeTuple_SET(result, 1, Dee_EmptyBytes); /* Inherit reference. */
		if (is_rpartition) {
			DeeTuple_SET(result, 0, Dee_EmptyBytes); /* Inherit reference. */
			DeeTuple_SET(result, 2, self);           /* Inherit reference. */
		} else {
			DeeTuple_SET(result, 0, self);           /* Inherit reference. */
			DeeTuple_SET(result, 2, Dee_EmptyBytes); /* Inherit reference. */
		}
	} else {
		DREF DeeObject *temp;
		temp = DeeBytes_NewSubView(self, start_ptr,
		                           (size_t)(find_ptr - start_ptr));
		if unlikely(!temp)
			goto err_r_0;
		DeeTuple_SET(result, 0, temp); /* Inherit reference. */
		temp = DeeBytes_NewSubView(self, find_ptr, needle_len);
		if unlikely(!temp)
			goto err_r_1;
		DeeTuple_SET(result, 1, temp); /* Inherit reference. */
		find_ptr += needle_len;
		temp = DeeBytes_NewSubView(self, find_ptr,
		                           (start_ptr + search_size) - find_ptr);
		if unlikely(!temp)
			goto err_r_2;
		DeeTuple_SET(result, 2, temp); /* Inherit reference. */
	}
done:
	return result;
err_r_2:
	Dee_Decref(DeeTuple_GET(result, 1));
err_r_1:
	Dee_Decref(DeeTuple_GET(result, 0));
err_r_0:
	DeeTuple_FreeUninitialized(result);
	return NULL;
}

#define empty_bytes_partition__newref() \
	(Dee_Incref(&empty_bytes_partition), (DeeTupleObject *)&empty_bytes_partition)
PRIVATE DEFINE_TUPLE(empty_bytes_partition, 3, {
	Dee_EmptyBytes,
	Dee_EmptyBytes,
	Dee_EmptyBytes
});

PRIVATE WUNUSED NONNULL((1)) DREF DeeTupleObject *DCALL
bytes_partition(Bytes *self, size_t argc,
                DeeObject *const *argv, DeeObject *kw) {
	DREF DeeTupleObject *result;
	Needle needle;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("partition", params: "
	DeeObject *needle: ?X3?DBytes?Dstring?Dint, size_t start = 0, size_t end = (size_t)-1
", docStringPrefix: "bytes");]]]*/
#define bytes_partition_params "needle:?X3?.?Dstring?Dint,start=!0,end=!-1"
	struct {
		DeeObject *needle;
		size_t start;
		size_t end;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__needle_start_end, "o|" UNPuSIZ UNPxSIZ ":partition", &args))
		goto err;
/*[[[end]]]*/
	if (acquire_needle(&needle, args.needle))
		goto err;
	if (args.end > DeeBytes_SIZE(self))
		args.end = DeeBytes_SIZE(self);
	if (args.start >= args.end) {
		result = empty_bytes_partition__newref();
	} else {
		args.end -= args.start;
		result = bytes_pack_partition(self,
		                              memmemb(DeeBytes_DATA(self) + args.start,
		                                      args.end,
		                                      needle.n_data,
		                                      needle.n_size),
		                              DeeBytes_DATA(self) + args.start,
		                              args.end,
		                              needle.n_size,
		                              false);
	}
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTupleObject *DCALL
bytes_casepartition(Bytes *self, size_t argc,
                    DeeObject *const *argv, DeeObject *kw) {
	DREF DeeTupleObject *result;
	Needle needle;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("casepartition", params: "
	DeeObject *needle: ?X3?DBytes?Dstring?Dint, size_t start = 0, size_t end = (size_t)-1
", docStringPrefix: "bytes");]]]*/
#define bytes_casepartition_params "needle:?X3?.?Dstring?Dint,start=!0,end=!-1"
	struct {
		DeeObject *needle;
		size_t start;
		size_t end;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__needle_start_end, "o|" UNPuSIZ UNPxSIZ ":casepartition", &args))
		goto err;
/*[[[end]]]*/
	if (acquire_needle(&needle, args.needle))
		goto err;
	if (args.end > DeeBytes_SIZE(self))
		args.end = DeeBytes_SIZE(self);
	if (args.start >= args.end) {
		result = empty_bytes_partition__newref();
	} else {
		args.end -= args.start;
		result = bytes_pack_partition(self,
		                              memasciicasemem(DeeBytes_DATA(self) + args.start,
		                                              args.end,
		                                              needle.n_data,
		                                              needle.n_size),
		                              DeeBytes_DATA(self) + args.start,
		                              args.end,
		                              needle.n_size,
		                              false);
	}
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTupleObject *DCALL
bytes_rpartition(Bytes *self, size_t argc,
                 DeeObject *const *argv, DeeObject *kw) {
	DREF DeeTupleObject *result;
	Needle needle;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("rpartition", params: "
	DeeObject *needle: ?X3?DBytes?Dstring?Dint, size_t start = 0, size_t end = (size_t)-1
", docStringPrefix: "bytes");]]]*/
#define bytes_rpartition_params "needle:?X3?.?Dstring?Dint,start=!0,end=!-1"
	struct {
		DeeObject *needle;
		size_t start;
		size_t end;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__needle_start_end, "o|" UNPuSIZ UNPxSIZ ":rpartition", &args))
		goto err;
/*[[[end]]]*/
	if (acquire_needle(&needle, args.needle))
		goto err;
	if (args.end > DeeBytes_SIZE(self))
		args.end = DeeBytes_SIZE(self);
	if (args.start >= args.end) {
		result = empty_bytes_partition__newref();
	} else {
		args.end -= args.start;
		result = bytes_pack_partition(self,
		                              memrmemb(DeeBytes_DATA(self) + args.start,
		                                       args.end,
		                                       needle.n_data,
		                                       needle.n_size),
		                              DeeBytes_DATA(self) + args.start,
		                              args.end,
		                              needle.n_size,
		                              true);
	}
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTupleObject *DCALL
bytes_caserpartition(Bytes *self, size_t argc,
                     DeeObject *const *argv, DeeObject *kw) {
	DREF DeeTupleObject *result;
	Needle needle;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("caserpartition", params: "
	DeeObject *needle: ?X3?DBytes?Dstring?Dint, size_t start = 0, size_t end = (size_t)-1
", docStringPrefix: "bytes");]]]*/
#define bytes_caserpartition_params "needle:?X3?.?Dstring?Dint,start=!0,end=!-1"
	struct {
		DeeObject *needle;
		size_t start;
		size_t end;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__needle_start_end, "o|" UNPuSIZ UNPxSIZ ":caserpartition", &args))
		goto err;
/*[[[end]]]*/
	if (acquire_needle(&needle, args.needle))
		goto err;
	if (args.end > DeeBytes_SIZE(self))
		args.end = DeeBytes_SIZE(self);
	if (args.start >= args.end) {
		result = empty_bytes_partition__newref();
	} else {
		args.end -= args.start;
		result = bytes_pack_partition(self,
		                              memasciicasermem(DeeBytes_DATA(self) + args.start,
		                                               args.end,
		                                               needle.n_data,
		                                               needle.n_size),
		                              DeeBytes_DATA(self) + args.start,
		                              args.end,
		                              needle.n_size,
		                              true);
	}
	return result;
err:
	return NULL;
}

#ifdef __INTELLISENSE__
PRIVATE WUNUSED NONNULL((1)) DREF Bytes *DCALL bytes_strip(Bytes *self, size_t argc, DeeObject *const *argv);
PRIVATE WUNUSED NONNULL((1)) DREF Bytes *DCALL bytes_lstrip(Bytes *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE WUNUSED NONNULL((1)) DREF Bytes *DCALL bytes_rstrip(Bytes *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE WUNUSED NONNULL((1)) DREF Bytes *DCALL bytes_sstrip(Bytes *self, size_t argc, DeeObject *const *argv);
PRIVATE WUNUSED NONNULL((1)) DREF Bytes *DCALL bytes_lsstrip(Bytes *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE WUNUSED NONNULL((1)) DREF Bytes *DCALL bytes_rsstrip(Bytes *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE WUNUSED NONNULL((1)) DREF Bytes *DCALL bytes_casestrip(Bytes *self, size_t argc, DeeObject *const *argv);
PRIVATE WUNUSED NONNULL((1)) DREF Bytes *DCALL bytes_caselstrip(Bytes *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE WUNUSED NONNULL((1)) DREF Bytes *DCALL bytes_caserstrip(Bytes *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE WUNUSED NONNULL((1)) DREF Bytes *DCALL bytes_casesstrip(Bytes *self, size_t argc, DeeObject *const *argv);
PRIVATE WUNUSED NONNULL((1)) DREF Bytes *DCALL bytes_caselsstrip(Bytes *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE WUNUSED NONNULL((1)) DREF Bytes *DCALL bytes_casersstrip(Bytes *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE WUNUSED NONNULL((1)) DREF Bytes *DCALL bytes_striplines(Bytes *self, size_t argc, DeeObject *const *argv);
PRIVATE WUNUSED NONNULL((1)) DREF Bytes *DCALL bytes_lstriplines(Bytes *self, size_t argc, DeeObject *const *argv);
PRIVATE WUNUSED NONNULL((1)) DREF Bytes *DCALL bytes_rstriplines(Bytes *self, size_t argc, DeeObject *const *argv);
PRIVATE WUNUSED NONNULL((1)) DREF Bytes *DCALL bytes_sstriplines(Bytes *self, size_t argc, DeeObject *const *argv);
PRIVATE WUNUSED NONNULL((1)) DREF Bytes *DCALL bytes_lsstriplines(Bytes *self, size_t argc, DeeObject *const *argv);
PRIVATE WUNUSED NONNULL((1)) DREF Bytes *DCALL bytes_rsstriplines(Bytes *self, size_t argc, DeeObject *const *argv);
PRIVATE WUNUSED NONNULL((1)) DREF Bytes *DCALL bytes_casestriplines(Bytes *self, size_t argc, DeeObject *const *argv);
PRIVATE WUNUSED NONNULL((1)) DREF Bytes *DCALL bytes_caselstriplines(Bytes *self, size_t argc, DeeObject *const *argv);
PRIVATE WUNUSED NONNULL((1)) DREF Bytes *DCALL bytes_caserstriplines(Bytes *self, size_t argc, DeeObject *const *argv);
PRIVATE WUNUSED NONNULL((1)) DREF Bytes *DCALL bytes_casesstriplines(Bytes *self, size_t argc, DeeObject *const *argv);
PRIVATE WUNUSED NONNULL((1)) DREF Bytes *DCALL bytes_caselsstriplines(Bytes *self, size_t argc, DeeObject *const *argv);
PRIVATE WUNUSED NONNULL((1)) DREF Bytes *DCALL bytes_casersstriplines(Bytes *self, size_t argc, DeeObject *const *argv);
#else /* __INTELLISENSE__ */
DECL_END
#define DEFINE_bytes_strip
#include "bytes_functions-strip.c.inl"
#define DEFINE_bytes_lstrip
#include "bytes_functions-strip.c.inl"
#define DEFINE_bytes_rstrip
#include "bytes_functions-strip.c.inl"
#define DEFINE_bytes_sstrip
#include "bytes_functions-strip.c.inl"
#define DEFINE_bytes_lsstrip
#include "bytes_functions-strip.c.inl"
#define DEFINE_bytes_rsstrip
#include "bytes_functions-strip.c.inl"
#define DEFINE_bytes_casestrip
#include "bytes_functions-strip.c.inl"
#define DEFINE_bytes_caselstrip
#include "bytes_functions-strip.c.inl"
#define DEFINE_bytes_caserstrip
#include "bytes_functions-strip.c.inl"
#define DEFINE_bytes_casesstrip
#include "bytes_functions-strip.c.inl"
#define DEFINE_bytes_caselsstrip
#include "bytes_functions-strip.c.inl"
#define DEFINE_bytes_casersstrip
#include "bytes_functions-strip.c.inl"
#define DEFINE_bytes_striplines
#include "bytes_functions-strip.c.inl"
#define DEFINE_bytes_lstriplines
#include "bytes_functions-strip.c.inl"
#define DEFINE_bytes_rstriplines
#include "bytes_functions-strip.c.inl"
#define DEFINE_bytes_sstriplines
#include "bytes_functions-strip.c.inl"
#define DEFINE_bytes_lsstriplines
#include "bytes_functions-strip.c.inl"
#define DEFINE_bytes_rsstriplines
#include "bytes_functions-strip.c.inl"
#define DEFINE_bytes_casestriplines
#include "bytes_functions-strip.c.inl"
#define DEFINE_bytes_caselstriplines
#include "bytes_functions-strip.c.inl"
#define DEFINE_bytes_caserstriplines
#include "bytes_functions-strip.c.inl"
#define DEFINE_bytes_casesstriplines
#include "bytes_functions-strip.c.inl"
#define DEFINE_bytes_caselsstriplines
#include "bytes_functions-strip.c.inl"
#define DEFINE_bytes_casersstriplines
#include "bytes_functions-strip.c.inl"
DECL_BEGIN
#endif /* !__INTELLISENSE__ */



struct bcompare_args {
	DeeObject    *other;   /* [1..1] String or Bytes object. */
	byte_t const *lhs_ptr; /* [0..my_len] Starting pointer of lhs. */
	size_t        lhs_len; /* # of bytes in lhs. */
	byte_t const *rhs_ptr; /* [0..my_len] Starting pointer of rhs. */
	size_t        rhs_len; /* # of bytes in rhs. */
};
#define release_bcompare_args(self, args) (void)0

#define get_bcompare_decl(rhs_name, return_decl)                                                              \
	"(" rhs_name ":?X2?.?Dstring," rhs_name "_start=!0," rhs_name "_end=!-1)" return_decl "\n"                \
	"(my_start:?Dint," rhs_name ":?X2?.?Dstring," rhs_name "_start=!0," rhs_name "_end=!-1)" return_decl "\n" \
	"(my_start:?Dint,my_end:?Dint," rhs_name ":?X2?.?Dstring," rhs_name "_start=!0," rhs_name "_end=!-1)" return_decl
PRIVATE WUNUSED NONNULL((1, 2, 5)) int DCALL
acquire_bcompare_args(Bytes *__restrict self,
                      struct bcompare_args *__restrict args,
                      size_t argc, DeeObject *const *argv,
                      char const *__restrict funname) {
	DeeObject *other;
	size_t temp, temp2;
	args->lhs_ptr = DeeBytes_DATA(self);
	args->lhs_len = DeeBytes_SIZE(self);
	switch (argc) {

	case 1:
		args->other = other = argv[0];
		if (DeeBytes_Check(other)) {
			args->rhs_ptr = DeeBytes_DATA(other);
			args->rhs_len = DeeBytes_SIZE(other);
		} else {
			if unlikely(!DeeString_Check(other))
				goto err_type_other;
			args->rhs_ptr = DeeString_AsBytes(other, false);
			if unlikely(!args->rhs_ptr)
				goto err;
			args->rhs_len = WSTR_LENGTH(args->rhs_ptr);
		}
		break;

	case 2:
		if (DeeBytes_Check(argv[0])) {
			args->other = other = argv[0];
			if (DeeObject_AsSize(argv[1], &temp))
				goto err;
			args->rhs_ptr = DeeBytes_DATA(other);
			args->rhs_len = DeeBytes_SIZE(other);
			if unlikely(temp >= args->rhs_len) {
				args->rhs_len = 0;
			} else {
				args->rhs_ptr += temp;
				args->rhs_len -= temp;
			}
		} else if (DeeString_Check(argv[0])) {
			args->other = other = argv[0];
			if (DeeObject_AsSize(argv[1], &temp))
				goto err;
			args->rhs_ptr = DeeString_AsBytes(other, false);
			if unlikely(!args->rhs_ptr)
				goto err;
			args->rhs_len = WSTR_LENGTH(args->rhs_ptr);
			if unlikely(temp >= args->rhs_len) {
				args->rhs_len = 0;
			} else {
				args->rhs_ptr += temp;
				args->rhs_len -= temp;
			}
		} else {
			if (DeeObject_AsSize(argv[0], &temp))
				goto err;
			if unlikely(temp >= args->lhs_len) {
				args->lhs_len = 0;
			} else {
				args->lhs_ptr += temp;
				args->lhs_len -= temp;
			}
			args->other = other = argv[1];
			if (DeeBytes_Check(other)) {
				args->rhs_ptr = DeeBytes_DATA(other);
				args->rhs_len = DeeBytes_SIZE(other);
			} else {
				if unlikely(!DeeString_Check(other))
					goto err_type_other;
				args->rhs_ptr = DeeString_AsBytes(other, false);
				if unlikely(!args->rhs_ptr)
					goto err;
				args->rhs_len = WSTR_LENGTH(args->rhs_ptr);
			}
		}
		break;

	case 3:
		if (DeeBytes_Check(argv[0])) {
			args->other = other = argv[0];
			args->rhs_ptr = DeeBytes_DATA(other);
			args->rhs_len = DeeBytes_SIZE(other);
			if (DeeObject_AsSize(argv[1], &temp))
				goto err;
			if (DeeObject_AsSizeM1(argv[2], &temp2))
				goto err;
			CLAMP_SUBSTR_IMPLICIT(&temp, &temp2, &args->rhs_len);
			args->rhs_ptr += temp;
		} else if (DeeString_Check(argv[0])) {
			args->other = other = argv[0];
			args->rhs_ptr = DeeString_AsBytes(other, true);
			if unlikely(!args->rhs_ptr)
				goto err;
			args->rhs_len = WSTR_LENGTH(args->rhs_ptr);
			if (DeeObject_AsSize(argv[1], &temp))
				goto err;
			if (DeeObject_AsSizeM1(argv[2], &temp2))
				goto err;
			CLAMP_SUBSTR_IMPLICIT(&temp, &temp2, &args->rhs_len);
			args->rhs_ptr += temp;
		} else if (DeeBytes_Check(argv[1])) {
			if (DeeObject_AsSize(argv[0], &temp))
				goto err;
			if unlikely(temp >= args->lhs_len) {
				args->lhs_len = 0;
			} else {
				args->lhs_ptr += temp;
				args->lhs_len -= temp;
			}
			args->other = other = argv[1];
			args->rhs_ptr = DeeBytes_DATA(other);
			args->rhs_len = DeeBytes_SIZE(other);
			if (DeeObject_AsSize(argv[2], &temp))
				goto err;
			if unlikely(temp >= args->rhs_len) {
				args->rhs_len = 0;
			} else {
				args->rhs_ptr += temp;
				args->rhs_len -= temp;
			}
		} else if (DeeString_Check(argv[1])) {
			if (DeeObject_AsSize(argv[0], &temp))
				goto err;
			if unlikely(temp >= args->lhs_len) {
				args->lhs_len = 0;
			} else {
				args->lhs_ptr += temp;
				args->lhs_len -= temp;
			}
			args->other = other = argv[1];
			args->rhs_ptr = DeeString_AsBytes(other, false);
			if unlikely(!args->rhs_ptr)
				goto err;
			args->rhs_len = WSTR_LENGTH(args->rhs_ptr);
			if (DeeObject_AsSize(argv[2], &temp))
				goto err;
			if unlikely(temp >= args->rhs_len) {
				args->rhs_len = 0;
			} else {
				args->rhs_ptr += temp;
				args->rhs_len -= temp;
			}
		} else {
			if (DeeObject_AsSize(argv[0], &temp))
				goto err;
			if (DeeObject_AsSizeM1(argv[1], &temp2))
				goto err;
			CLAMP_SUBSTR_IMPLICIT(&temp, &temp2, &args->lhs_len);
			args->lhs_ptr += temp;
			args->other = other = argv[2];
			if (DeeBytes_Check(other)) {
				args->rhs_ptr = DeeBytes_DATA(other);
				args->rhs_len = DeeBytes_SIZE(other);
			} else {
				if unlikely(!DeeString_Check(other))
					goto err_type_other;
				args->rhs_ptr = DeeString_AsBytes(other, false);
				if unlikely(!args->rhs_ptr)
					goto err;
				args->rhs_len = WSTR_LENGTH(args->rhs_ptr);
			}
		}
		break;

	case 4:
		if (DeeObject_AsSize(argv[0], &temp))
			goto err;
		if (DeeBytes_Check(argv[1])) {
			if unlikely(temp >= args->lhs_len) {
				args->lhs_len = 0;
			} else {
				args->lhs_ptr += temp;
				args->lhs_len -= temp;
			}
			args->other = other = argv[1];
			if (DeeObject_AsSize(argv[2], &temp))
				goto err;
			if (DeeObject_AsSizeM1(argv[3], &temp2))
				goto err;
			args->rhs_ptr = DeeBytes_DATA(other);
			args->rhs_len = DeeBytes_SIZE(other);
			CLAMP_SUBSTR_IMPLICIT(&temp, &temp2, &args->rhs_len);
			args->rhs_ptr += temp;
		} else if (DeeString_Check(argv[1])) {
			if unlikely(temp >= args->lhs_len) {
				args->lhs_len = 0;
			} else {
				args->lhs_ptr += temp;
				args->lhs_len -= temp;
			}
			args->other = other = argv[1];
			if (DeeObject_AsSize(argv[2], &temp))
				goto err;
			if (DeeObject_AsSizeM1(argv[3], &temp2))
				goto err;
			args->rhs_ptr = DeeString_AsBytes(other, false);
			if unlikely(!args->rhs_ptr)
				goto err;
			args->rhs_len = WSTR_LENGTH(args->rhs_ptr);
			CLAMP_SUBSTR_IMPLICIT(&temp, &temp2, &args->rhs_len);
			args->rhs_ptr += temp;
		} else {
			if (DeeObject_AsSizeM1(argv[1], &temp2))
				goto err;
			CLAMP_SUBSTR_IMPLICIT(&temp, &temp2, &args->lhs_len);
			args->lhs_ptr += temp;
			args->other = other = argv[2];
			if unlikely(!DeeString_Check(other))
				goto err_type_other;
			if (DeeObject_AsSize(argv[3], &temp))
				goto err;
			args->rhs_ptr = DeeString_AsBytes(other, false);
			if unlikely(!args->rhs_ptr)
				goto err;
			args->rhs_len = WSTR_LENGTH(args->rhs_ptr);
			if (temp >= args->rhs_len) {
				args->rhs_len = 0;
			} else {
				args->rhs_ptr += temp;
				args->rhs_len -= temp;
			}
		}
		break;

	case 5:
		if (DeeObject_AsSize(argv[0], &temp))
			goto err;
		if (DeeObject_AsSizeM1(argv[1], &temp2))
			goto err;
		CLAMP_SUBSTR_IMPLICIT(&temp, &temp2, &args->lhs_len);
		args->lhs_ptr += temp;
		args->other = other = argv[2];
		if (DeeBytes_Check(other)) {
			args->rhs_ptr = DeeBytes_DATA(other);
			args->rhs_len = DeeBytes_SIZE(other);
		} else {
			if unlikely(!DeeString_Check(other))
				goto err_type_other;
			args->rhs_ptr = DeeString_AsBytes(other, false);
			if unlikely(!args->rhs_ptr)
				goto err;
			args->rhs_len = WSTR_LENGTH(args->rhs_ptr);
		}
		if (DeeObject_AsSize(argv[3], &temp))
			goto err;
		if (DeeObject_AsSizeM1(argv[4], &temp2))
			goto err;
		CLAMP_SUBSTR_IMPLICIT(&temp, &temp2, &args->rhs_len);
		args->rhs_ptr += temp;
		break;

	default:
		err_invalid_argc(funname, argc, 1, 5);
		goto err;
	}
	return 0;
err_type_other:
	DeeObject_TypeAssertFailed(other, &DeeBytes_Type);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_compare(Bytes *self, size_t argc, DeeObject *const *argv) {
	struct bcompare_args args;
	int result;
	if (acquire_bcompare_args(self, &args, argc, argv, "compare"))
		goto err;
	if (args.lhs_len < args.rhs_len) {
		result = memcmp(args.lhs_ptr, args.rhs_ptr, args.lhs_len);
		if (result == 0)
			result = -1;
	} else if (args.lhs_len > args.rhs_len) {
		result = memcmp(args.lhs_ptr, args.rhs_ptr, args.rhs_len);
		if (result == 0)
			result = 1;
	} else {
		result = memcmp(args.lhs_ptr, args.rhs_ptr, args.lhs_len);
	}
	if (result < -1)
		result = -1;
	if (result > 1)
		result = 1;
	release_bcompare_args(self, &args);
	return_reference(DeeInt_FromSign(result));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_vercompare(Bytes *self, size_t argc, DeeObject *const *argv) {
	struct bcompare_args args;
	int result;
	if (acquire_bcompare_args(self, &args, argc, argv, "vercompare"))
		goto err;
	result = dee_strverscmpb(args.lhs_ptr, args.lhs_len,
	                         args.rhs_ptr, args.lhs_len);
	release_bcompare_args(self, &args);
	return_reference(DeeInt_FromSign(result));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_wildcompare(Bytes *self, size_t argc, DeeObject *const *argv) {
	struct bcompare_args args;
	int result;
	if (acquire_bcompare_args(self, &args, argc, argv, "wildcompare"))
		goto err;
	result = dee_wildcompareb(args.lhs_ptr, args.lhs_len,
	                          args.rhs_ptr, args.lhs_len);
	release_bcompare_args(self, &args);
	return_reference(DeeInt_FromSign(result));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_fuzzycompare(Bytes *self, size_t argc, DeeObject *const *argv) {
	struct bcompare_args args;
	size_t result;
	if (acquire_bcompare_args(self, &args, argc, argv, "fuzzycompare"))
		goto err;
	result = dee_fuzzy_compareb(args.lhs_ptr, args.lhs_len,
	                            args.rhs_ptr, args.lhs_len);
	release_bcompare_args(self, &args);
	if unlikely(result == (size_t)-1)
		goto err;
	return DeeInt_NewSize(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_wmatch(Bytes *self, size_t argc, DeeObject *const *argv) {
	struct bcompare_args args;
	int result;
	if (acquire_bcompare_args(self, &args, argc, argv, "wmatch"))
		goto err;
	result = dee_wildcompareb(args.lhs_ptr, args.lhs_len,
	                          args.rhs_ptr, args.lhs_len);
	release_bcompare_args(self, &args);
	return_bool(result == 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_casecompare(Bytes *self, size_t argc, DeeObject *const *argv) {
	struct bcompare_args args;
	int result;
	if (acquire_bcompare_args(self, &args, argc, argv, "casecompare"))
		goto err;
	if (args.lhs_len < args.rhs_len) {
		result = memcasecmp(args.lhs_ptr, args.rhs_ptr, args.lhs_len);
		if (result == 0)
			result = -1;
	} else if (args.lhs_len > args.rhs_len) {
		result = memcasecmp(args.lhs_ptr, args.rhs_ptr, args.rhs_len);
		if (result == 0)
			result = 1;
	} else {
		result = memcasecmp(args.lhs_ptr, args.rhs_ptr, args.lhs_len);
	}
	if (result < -1)
		result = -1;
	if (result > 1)
		result = 1;
	release_bcompare_args(self, &args);
	return_reference(DeeInt_FromSign(result));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_casevercompare(Bytes *self, size_t argc, DeeObject *const *argv) {
	int result;
	struct bcompare_args args;
	if (acquire_bcompare_args(self, &args, argc, argv, "casevercompare"))
		goto err;
	result = dee_strcaseverscmp_ascii(args.lhs_ptr, args.lhs_len,
	                                  args.rhs_ptr, args.lhs_len);
	release_bcompare_args(self, &args);
	return_reference(DeeInt_FromSign(result));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_casewildcompare(Bytes *self, size_t argc, DeeObject *const *argv) {
	struct bcompare_args args;
	int result;
	if (acquire_bcompare_args(self, &args, argc, argv, "casewildcompare"))
		goto err;
	result = dee_wildcasecompare_ascii(args.lhs_ptr, args.lhs_len,
	                                   args.rhs_ptr, args.lhs_len);
	release_bcompare_args(self, &args);
	return_reference(DeeInt_FromSign(result));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_casefuzzycompare(Bytes *self, size_t argc, DeeObject *const *argv) {
	struct bcompare_args args;
	Dee_ssize_t result;
	if (acquire_bcompare_args(self, &args, argc, argv, "casefuzzycompare"))
		goto err;
	result = dee_fuzzy_casecompare_ascii(args.lhs_ptr, args.lhs_len,
	                                     args.rhs_ptr, args.lhs_len);
	release_bcompare_args(self, &args);
	if unlikely(result == (Dee_ssize_t)-1)
		goto err;
	return DeeInt_NewSize((size_t)result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_casewmatch(Bytes *self, size_t argc, DeeObject *const *argv) {
	struct bcompare_args args;
	int result;
	if (acquire_bcompare_args(self, &args, argc, argv, "casewmatch"))
		goto err;
	result = dee_wildcasecompare_ascii(args.lhs_ptr, args.lhs_len,
	                                   args.rhs_ptr, args.lhs_len);
	release_bcompare_args(self, &args);
	return_bool(result == 0);
err:
	return NULL;
}

#ifndef DEFINED_err_empty_filler
#define DEFINED_err_empty_filler
PRIVATE ATTR_COLD int DCALL err_empty_filler(void) {
	return DeeError_Throwf(&DeeError_ValueError,
	                       "Empty filler");
}
#endif /* !DEFINED_err_empty_filler */

PRIVATE WUNUSED NONNULL((1)) DREF Bytes *DCALL
bytes_center(Bytes *self, size_t argc, DeeObject *const *argv) {
	DREF Bytes *result;
	Needle filler;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("center", params: "
	size_t width;
	DeeObject *filler: ?X3?DBytes?Dstring?Dint = NULL = !P{ };
", docStringPrefix: "bytes");]]]*/
#define bytes_center_params "width:?Dint,filler:?X3?.?Dstring?Dint=!P{ }"
	struct {
		size_t width;
		DeeObject *filler;
	} args;
	args.filler = NULL;
	DeeArg_UnpackStruct1XOr2X(err, argc, argv, "center", &args, &args.width, UNPuSIZ, DeeObject_AsSize, &args.filler, "o", _DeeArg_AsObject);
/*[[[end]]]*/
	if (args.filler) {
		if (acquire_needle(&filler, args.filler))
			goto err;
		if unlikely(!filler.n_size)
			goto empty_filler;
	} else {
		filler.n_data    = filler._n_buf;
		filler.n_size    = 1;
		filler._n_buf[0] = UNICODE_SPACE;
	}
	if (args.width <= DeeBytes_SIZE(self)) {
		result = self;
		Dee_Incref(result);
	} else {
		size_t fill_front, fill_back;
		result = DeeBytes_NewBufferUninitialized(args.width);
		if unlikely(!result)
			goto err;
		fill_front = (args.width - DeeBytes_SIZE(self));
		fill_back  = fill_front / 2;
		fill_front -= fill_back;
		mempfilb(DeeBytes_BUFFER_DATA(result) + 0, fill_front, filler.n_data, filler.n_size);
		memcpyb(DeeBytes_BUFFER_DATA(result) + fill_front,
		        DeeBytes_DATA(self), DeeBytes_SIZE(self));
		mempfilb(DeeBytes_BUFFER_DATA(result) + fill_front + DeeBytes_SIZE(self),
		        fill_back, filler.n_data, filler.n_size);
	}
	return result;
empty_filler:
	err_empty_filler();
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF Bytes *DCALL
bytes_ljust(Bytes *self, size_t argc, DeeObject *const *argv) {
	DREF Bytes *result;
	Needle filler;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("ljust", params: "
	size_t width;
	DeeObject *filler: ?X3?DBytes?Dstring?Dint = NULL = !P{ };
", docStringPrefix: "bytes");]]]*/
#define bytes_ljust_params "width:?Dint,filler:?X3?.?Dstring?Dint=!P{ }"
	struct {
		size_t width;
		DeeObject *filler;
	} args;
	args.filler = NULL;
	DeeArg_UnpackStruct1XOr2X(err, argc, argv, "ljust", &args, &args.width, UNPuSIZ, DeeObject_AsSize, &args.filler, "o", _DeeArg_AsObject);
/*[[[end]]]*/
	if (args.filler) {
		if (acquire_needle(&filler, args.filler))
			goto err;
		if unlikely(!filler.n_size)
			goto empty_filler;
	} else {
		filler.n_data    = filler._n_buf;
		filler.n_size    = 1;
		filler._n_buf[0] = UNICODE_SPACE;
	}
	if (args.width <= DeeBytes_SIZE(self)) {
		result = self;
		Dee_Incref(result);
	} else {
		size_t fill_back;
		result = DeeBytes_NewBufferUninitialized(args.width);
		if unlikely(!result)
			goto err;
		fill_back = (args.width - DeeBytes_SIZE(self));
		memcpyb(DeeBytes_BUFFER_DATA(result) + 0,
		        DeeBytes_DATA(self), DeeBytes_SIZE(self));
		mempfilb(DeeBytes_BUFFER_DATA(result) + DeeBytes_SIZE(self),
		        fill_back, filler.n_data, filler.n_size);
	}
	return result;
empty_filler:
	err_empty_filler();
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF Bytes *DCALL
bytes_rjust(Bytes *self, size_t argc, DeeObject *const *argv) {
	DREF Bytes *result;
	Needle filler;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("rjust", params: "
	size_t width;
	DeeObject *filler: ?X3?DBytes?Dstring?Dint = NULL = !P{ };
", docStringPrefix: "bytes");]]]*/
#define bytes_rjust_params "width:?Dint,filler:?X3?.?Dstring?Dint=!P{ }"
	struct {
		size_t width;
		DeeObject *filler;
	} args;
	args.filler = NULL;
	DeeArg_UnpackStruct1XOr2X(err, argc, argv, "rjust", &args, &args.width, UNPuSIZ, DeeObject_AsSize, &args.filler, "o", _DeeArg_AsObject);
/*[[[end]]]*/
	if (args.filler) {
		if (acquire_needle(&filler, args.filler))
			goto err;
		if unlikely(!filler.n_size)
			goto empty_filler;
	} else {
		filler.n_data    = filler._n_buf;
		filler.n_size    = 1;
		filler._n_buf[0] = UNICODE_SPACE;
	}
	if (args.width <= DeeBytes_SIZE(self)) {
		result = self;
		Dee_Incref(result);
	} else {
		size_t fill_front;
		result = DeeBytes_NewBufferUninitialized(args.width);
		if unlikely(!result)
			goto err;
		fill_front = (args.width - DeeBytes_SIZE(self));
		mempfilb(DeeBytes_BUFFER_DATA(result) + 0, fill_front, filler.n_data, filler.n_size);
		memcpyb(DeeBytes_BUFFER_DATA(result) + fill_front,
		        DeeBytes_DATA(self), DeeBytes_SIZE(self));
	}
	return result;
empty_filler:
	err_empty_filler();
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF Bytes *DCALL
bytes_zfill(Bytes *self, size_t argc, DeeObject *const *argv) {
	DREF Bytes *result;
	Needle filler;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("zfill", params: "
	size_t width;
	DeeObject *filler: ?X3?DBytes?Dstring?Dint = NULL = !P{0};
", docStringPrefix: "bytes");]]]*/
#define bytes_zfill_params "width:?Dint,filler:?X3?.?Dstring?Dint=!P{0}"
	struct {
		size_t width;
		DeeObject *filler;
	} args;
	args.filler = NULL;
	DeeArg_UnpackStruct1XOr2X(err, argc, argv, "zfill", &args, &args.width, UNPuSIZ, DeeObject_AsSize, &args.filler, "o", _DeeArg_AsObject);
/*[[[end]]]*/
	if (args.filler) {
		if (acquire_needle(&filler, args.filler))
			goto err;
		if unlikely(!filler.n_size)
			goto empty_filler;
	} else {
		filler.n_data    = filler._n_buf;
		filler.n_size    = 1;
		filler._n_buf[0] = UNICODE_ZERO;
	}
	if (args.width <= DeeBytes_SIZE(self)) {
		result = self;
		Dee_Incref(result);
	} else {
		size_t fill_front, src_len;
		byte_t *dst, *src;
		result = DeeBytes_NewBufferUninitialized(args.width);
		if unlikely(!result)
			goto err;
		dst = DeeBytes_BUFFER_DATA(result);
		src = DeeBytes_DATA(self);
		src_len    = DeeBytes_SIZE(self);
		fill_front = (args.width - src_len);
		while (src_len && DeeUni_IsSign(src[0])) {
			*dst++ = *src++;
			--src_len;
		}
		mempfilb(dst + 0, fill_front, filler.n_data, filler.n_size);
		memcpyb(dst + fill_front, src, src_len);
	}
	return result;
empty_filler:
	err_empty_filler();
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_expandtabs(Bytes *self, size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("expandtabs", params: "
	size_t tabwidth = 8
", docStringPrefix: "bytes");]]]*/
#define bytes_expandtabs_params "tabwidth=!8"
	struct {
		size_t tabwidth;
	} args;
	args.tabwidth = 8;
	DeeArg_Unpack0Or1X(err, argc, argv, "expandtabs", &args.tabwidth, UNPuSIZ, DeeObject_AsSize);
/*[[[end]]]*/
	{
		struct Dee_bytes_printer printer = Dee_BYTES_PRINTER_INIT;
		byte_t *iter, *end, *flush_start;
		size_t line_inset;
		line_inset  = 0;
		iter        = DeeBytes_DATA(self);
		end         = iter + DeeBytes_SIZE(self);
		flush_start = iter;
		for (; iter < end; ++iter) {
			byte_t ch = *iter;
			if (!DeeUni_IsTab(ch)) {
				++line_inset;
				if (DeeUni_IsLF(ch))
					line_inset = 0; /* Reset insets at line starts. */
				continue;
			}
			if (Dee_bytes_printer_append(&printer, flush_start,
			                             (size_t)(iter - flush_start)) < 0)
				goto err_printer;
			/* Replace with white-space. */
			if likely(args.tabwidth) {
				line_inset = args.tabwidth - (line_inset % args.tabwidth);
				if (Dee_bytes_printer_repeat(&printer, ASCII_SPACE, line_inset) < 0)
					goto err_printer;
				line_inset = 0;
			}
			flush_start = iter + 1;
		}
		if (!Dee_BYTES_PRINTER_SIZE(&printer))
			goto retself;
		if (Dee_bytes_printer_append(&printer, flush_start,
		                         (size_t)(iter - flush_start)) < 0)
			goto err_printer;
		return Dee_bytes_printer_pack(&printer);
retself:
		Dee_bytes_printer_fini(&printer);
		return_reference_(Dee_AsObject(self));
err_printer:
		Dee_bytes_printer_fini(&printer);
	}
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_unifylines(Bytes *self, size_t argc, DeeObject *const *argv) {
	Needle replace;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("unifylines", params: "
	DeeObject *replacement: ?X3?DBytes?Dstring?Dint = NULL = !P{\\n}
", docStringPrefix: "bytes");]]]*/
#define bytes_unifylines_params "replacement:?X3?.?Dstring?Dint=!P{\n}"
	struct {
		DeeObject *replacement;
	} args;
	args.replacement = NULL;
	DeeArg_Unpack0Or1(err, argc, argv, "unifylines", &args.replacement);
/*[[[end]]]*/
	if (args.replacement) {
		if (acquire_needle(&replace, args.replacement))
			goto err;
	} else {
		replace.n_data    = replace._n_buf;
		replace.n_size    = 1;
		replace._n_buf[0] = '\n';
	}
	{
		struct Dee_bytes_printer printer = Dee_BYTES_PRINTER_INIT;
		byte_t *iter, *end, *flush_start;
		iter        = DeeBytes_DATA(self);
		end         = iter + DeeBytes_SIZE(self);
		flush_start = iter;
		for (; iter < end; ++iter) {
			byte_t ch = *iter;
			if (ch != ASCII_CR && ch != ASCII_LF)
				continue; /* Not a line-feed character */
			if (replace.n_size == 1 && ch == replace.n_data[0]) {
				if (ch != ASCII_CR)
					continue; /* No-op replacement. */
				if (iter + 1 >= end)
					continue; /* Cannot be CRLF */
				if (iter[1] != ASCII_LF)
					continue; /* Isn't CRLF */
			}
			if (Dee_bytes_printer_append(&printer, flush_start,
			                             (size_t)(iter - flush_start)) < 0)
				goto err_printer;
			if (Dee_bytes_printer_append(&printer,
			                             replace.n_data,
			                             replace.n_size) < 0)
				goto err_printer;
			if (ch == ASCII_CR && iter + 1 < end && iter[1] == ASCII_LF)
				++iter;
			flush_start = iter + 1;
		}
		if (!Dee_BYTES_PRINTER_SIZE(&printer))
			goto retself;
		if (Dee_bytes_printer_append(&printer, flush_start,
		                             (size_t)(iter - flush_start)) < 0)
			goto err_printer;
		return Dee_bytes_printer_pack(&printer);
retself:
		Dee_bytes_printer_fini(&printer);
		return_reference_(Dee_AsObject(self));
err_printer:
		Dee_bytes_printer_fini(&printer);
	}
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_indent(Bytes *self, size_t argc, DeeObject *const *argv) {
	Needle filler;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("indent", params: "
	DeeObject *filler: ?X3?DBytes?Dstring?Dint = Dee_AsObject(&str_tab) = !P{\t}
", docStringPrefix: "bytes");]]]*/
#define bytes_indent_params "filler:?X3?.?Dstring?Dint=!P{	}"
	struct {
		DeeObject *filler;
	} args;
	args.filler = Dee_AsObject(&str_tab);
	DeeArg_Unpack0Or1(err, argc, argv, "indent", &args.filler);
/*[[[end]]]*/
	if (args.filler) {
		if (acquire_needle(&filler, args.filler))
			goto err;
		if unlikely(!filler.n_size)
			goto retself;
	} else {
		filler.n_data    = filler._n_buf;
		filler.n_size    = 1;
		filler._n_buf[0] = ASCII_TAB;
	}
	if unlikely(DeeBytes_IsEmpty(self))
		goto retself;
	{
		struct Dee_bytes_printer printer = Dee_BYTES_PRINTER_INIT;
		byte_t *flush_start, *iter, *end;
		/* Start by inserting the initial, unconditional indentation at the start. */
		if (Dee_bytes_printer_append(&printer, filler.n_data, filler.n_size) < 0)
			goto err_printer;
		iter        = DeeBytes_DATA(self);
		end         = iter + DeeBytes_SIZE(self);
		flush_start = iter;
		while (iter < end) {
			byte_t ch = *iter;
			if (DeeUni_IsLF(ch)) {
				++iter;
				/* Deal with windows-style linefeeds. */
				if (ch == ASCII_CR && *iter == ASCII_LF)
					++iter;
				/* Flush all unwritten data up to this point. */
				if (Dee_bytes_printer_append(&printer, flush_start,
				                             (size_t)(iter - flush_start)) < 0)
					goto err_printer;
				flush_start = iter;
				/* Insert the filler just before the linefeed. */
				if (Dee_bytes_printer_append(&printer, filler.n_data, filler.n_size) < 0)
					goto err_printer;
				continue;
			}
			++iter;
		}
		if (iter == flush_start) {
			/* Either the string is empty or ends with a line-feed.
			 * In either case, we must remove `filler' from its end,
			 * because we're not supposed to have the resulting
			 * string include it as trailing memory. */
			ASSERT(Dee_BYTES_PRINTER_SIZE(&printer) >= filler.n_size);
			Dee_bytes_printer_release(&printer, filler.n_size);
		} else {
			/* Flush the remainder. */
			if (Dee_bytes_printer_append(&printer, flush_start,
			                             (size_t)(iter - flush_start)) < 0)
				goto err_printer;
		}
		return Dee_bytes_printer_pack(&printer);
err_printer:
		Dee_bytes_printer_fini(&printer);
	}
err:
	return NULL;
retself:
	return_reference_(Dee_AsObject(self));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_dedent(Bytes *self, size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("dedent", params: "
	size_t max = 1;
	DeeObject *mask: ?X3?DBytes?Dstring?Dint = NULL;
", docStringPrefix: "bytes");]]]*/
#define bytes_dedent_params "max=!1,mask?:?X3?.?Dstring?Dint"
	struct {
		size_t max_;
		DeeObject *mask;
	} args;
	args.max_ = 1;
	args.mask = NULL;
	if (DeeArg_UnpackStruct(argc, argv, "|" UNPuSIZ "o:dedent", &args))
		goto err;
/*[[[end]]]*/
	if unlikely(!args.max_)
		goto retself;
	{
		struct Dee_bytes_printer printer = Dee_BYTES_PRINTER_INIT;
		byte_t *flush_start, *iter, *end;
		size_t i;
		iter = DeeBytes_DATA(self);
		end  = iter + DeeBytes_SIZE(self);
		if (args.mask) {
			Needle mask;
			if (acquire_needle(&mask, args.mask))
				goto err_printer;

			/* Remove leading characters. */
			for (i = 0; i < args.max_ && memchr(mask.n_data, *iter, mask.n_size); ++i)
				++iter;
			flush_start = iter;
			while (iter < end) {
				byte_t ch = *iter;
				if (DeeUni_IsLF(ch)) {
					++iter;
					if (ch == UNICODE_CR && *iter == UNICODE_LF)
						++iter;

					/* Flush all unwritten data up to this point. */
					if (Dee_bytes_printer_append(&printer, flush_start,
					                             (size_t)(iter - flush_start)) < 0)
						goto err_printer;

					/* Skip up to `args.max_' characters after a linefeed. */
					for (i = 0; i < args.max_ && memchr(mask.n_data, *iter, mask.n_size); ++i)
						++iter;
					flush_start = iter;
					continue;
				}
				++iter;
			}

			/* Flush the remainder. */
			if (Dee_bytes_printer_append(&printer, flush_start,
			                             (size_t)(iter - flush_start)) < 0)
				goto err_printer;
		} else {
			/* Remove leading characters. */
			for (i = 0; i < args.max_ && DeeUni_IsSpace(*iter); ++i)
				++iter;
			flush_start = iter;
			while (iter < end) {
				byte_t ch = *iter;
				if (DeeUni_IsLF(ch)) {
					++iter;
					if (ch == ASCII_CR && *iter == ASCII_LF)
						++iter;

					/* Flush all unwritten data up to this point. */
					if (Dee_bytes_printer_append(&printer, flush_start,
					                         (size_t)(iter - flush_start)) < 0)
						goto err_printer;

					/* Skip up to `args.max_' characters after a linefeed. */
					for (i = 0; i < args.max_ && DeeUni_IsSpace(*iter); ++i)
						++iter;
					flush_start = iter;
					continue;
				}
				++iter;
			}

			/* Flush the remainder. */
			if (Dee_bytes_printer_append(&printer, flush_start,
			                             (size_t)(iter - flush_start)) < 0)
				goto err_printer;
		}
		return Dee_bytes_printer_pack(&printer);
err_printer:
		Dee_bytes_printer_fini(&printer);
	}
err:
	return NULL;
retself:
	return_reference_(Dee_AsObject(self));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_common(Bytes *self, size_t argc, DeeObject *const *argv) {
	struct bcompare_args args;
	size_t result = 0;
	if (acquire_bcompare_args(self, &args, argc, argv, "common"))
		goto err;
	if (args.lhs_len > args.rhs_len)
		args.lhs_len = args.rhs_len;
	while (result < args.lhs_len) {
		if (args.lhs_ptr[result] != args.rhs_ptr[result])
			break;
		++result;
	}
	release_bcompare_args(self, &args);
	return DeeInt_NewSize(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_rcommon(Bytes *self, size_t argc, DeeObject *const *argv) {
	struct bcompare_args args;
	size_t result = 0;
	if (acquire_bcompare_args(self, &args, argc, argv, "rcommon"))
		goto err;
	args.lhs_ptr += args.lhs_len;
	args.rhs_ptr += args.rhs_len;
	if (args.lhs_len > args.rhs_len)
		args.lhs_len = args.rhs_len;
	while (result < args.lhs_len) {
		if (args.lhs_ptr[-1] != args.rhs_ptr[-1])
			break;
		++result;
		--args.lhs_ptr;
		--args.rhs_ptr;
	}
	release_bcompare_args(self, &args);
	return DeeInt_NewSize(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_casecommon(Bytes *self, size_t argc, DeeObject *const *argv) {
	struct bcompare_args args;
	size_t result = 0;
	if (acquire_bcompare_args(self, &args, argc, argv, "casecommon"))
		goto err;
	if (args.lhs_len > args.rhs_len)
		args.lhs_len = args.rhs_len;
	while (result < args.lhs_len) {
		byte_t a = args.lhs_ptr[result];
		byte_t b = args.rhs_ptr[result];
		if (a != b) {
			a = (byte_t)DeeUni_IsLower(a);
			b = (byte_t)DeeUni_IsLower(b);
			if (a != b)
				break;
		}
		++result;
	}
	release_bcompare_args(self, &args);
	return DeeInt_NewSize(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_casercommon(Bytes *self, size_t argc, DeeObject *const *argv) {
	struct bcompare_args args;
	size_t result = 0;
	if (acquire_bcompare_args(self, &args, argc, argv, "casercommon"))
		goto err;
	args.lhs_ptr += args.lhs_len;
	args.rhs_ptr += args.rhs_len;
	if (args.lhs_len > args.rhs_len)
		args.lhs_len = args.rhs_len;
	while (result < args.lhs_len) {
		byte_t a = args.lhs_ptr[-1];
		byte_t b = args.rhs_ptr[-1];
		if (a != b) {
			a = (byte_t)DeeUni_IsLower(a);
			b = (byte_t)DeeUni_IsLower(b);
			if (a != b)
				break;
		}
		++result;
		--args.lhs_ptr;
		--args.rhs_ptr;
	}
	release_bcompare_args(self, &args);
	return DeeInt_NewSize(result);
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_findmatch(Bytes *self, size_t argc, DeeObject *const *argv) {
	byte_t const *ptr, *scan_str;
	size_t scan_len;
	Needle s_open, s_clos;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("findmatch", params: "
	DeeObject *open: ?X3?.?Dstring?Dint;
	DeeObject *close: ?X3?.?Dstring?Dint;
	size_t start = 0;
	size_t end = (size_t)-1;
", docStringPrefix: "bytes");]]]*/
#define bytes_findmatch_params "open:?X3?.?Dstring?Dint,close:?X3?.?Dstring?Dint,start=!0,end=!-1"
	struct {
		DeeObject *open;
		DeeObject *close;
		size_t start;
		size_t end;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	if (DeeArg_UnpackStruct(argc, argv, "oo|" UNPuSIZ UNPxSIZ ":findmatch", &args))
		goto err;
/*[[[end]]]*/
	if (acquire_needle(&s_open, args.open))
		goto err;
	if (acquire_needle(&s_clos, args.close))
		goto err;
	scan_len = DeeBytes_SIZE(self);
	CLAMP_SUBSTR(&args.start, &args.end, &scan_len, not_found);
	scan_str = DeeBytes_DATA(self);
	ptr = dee_find_matchb(scan_str + args.start, scan_len,
	                      s_open.n_data, s_open.n_size,
	                      s_clos.n_data, s_clos.n_size);
	if unlikely(!ptr)
		goto not_found;
	return DeeInt_NewSize((size_t)(ptr - scan_str));
not_found:
	return DeeInt_NewMinusOne();
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_rfindmatch(Bytes *self, size_t argc, DeeObject *const *argv) {
	byte_t const *ptr, *scan_str;
	size_t scan_len;
	Needle s_open, s_clos;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("rfindmatch", params: "
	DeeObject *open: ?X3?.?Dstring?Dint;
	DeeObject *close: ?X3?.?Dstring?Dint;
	size_t start = 0;
	size_t end = (size_t)-1;
", docStringPrefix: "bytes");]]]*/
#define bytes_rfindmatch_params "open:?X3?.?Dstring?Dint,close:?X3?.?Dstring?Dint,start=!0,end=!-1"
	struct {
		DeeObject *open;
		DeeObject *close;
		size_t start;
		size_t end;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	if (DeeArg_UnpackStruct(argc, argv, "oo|" UNPuSIZ UNPxSIZ ":rfindmatch", &args))
		goto err;
/*[[[end]]]*/
	if (acquire_needle(&s_open, args.open))
		goto err;
	if (acquire_needle(&s_clos, args.close))
		goto err;
	scan_len = DeeBytes_SIZE(self);
	CLAMP_SUBSTR(&args.start, &args.end, &scan_len, not_found);
	scan_str = DeeBytes_DATA(self);
	ptr = dee_rfind_matchb(scan_str + args.start, scan_len,
	                       s_open.n_data, s_open.n_size,
	                       s_clos.n_data, s_clos.n_size);
	if unlikely(!ptr)
		goto not_found;
	return DeeInt_NewSize((size_t)(ptr - scan_str));
not_found:
	return DeeInt_NewMinusOne();
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_indexmatch(Bytes *self, size_t argc, DeeObject *const *argv) {
	byte_t const *ptr, *scan_str;
	size_t scan_len;
	Needle s_open, s_clos;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("indexmatch", params: "
	DeeObject *open: ?X3?.?Dstring?Dint;
	DeeObject *close: ?X3?.?Dstring?Dint;
	size_t start = 0;
	size_t end = (size_t)-1;
", docStringPrefix: "bytes");]]]*/
#define bytes_indexmatch_params "open:?X3?.?Dstring?Dint,close:?X3?.?Dstring?Dint,start=!0,end=!-1"
	struct {
		DeeObject *open;
		DeeObject *close;
		size_t start;
		size_t end;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	if (DeeArg_UnpackStruct(argc, argv, "oo|" UNPuSIZ UNPxSIZ ":indexmatch", &args))
		goto err;
/*[[[end]]]*/
	if (acquire_needle(&s_open, args.open))
		goto err;
	if (acquire_needle(&s_clos, args.close))
		goto err;
	scan_len = DeeBytes_SIZE(self);
	CLAMP_SUBSTR(&args.start, &args.end, &scan_len, not_found);
	scan_str = DeeBytes_DATA(self);
	ptr = dee_find_matchb(scan_str + args.start, scan_len,
	                      s_open.n_data, s_open.n_size,
	                      s_clos.n_data, s_clos.n_size);
	if unlikely(!ptr)
		goto not_found;
	return DeeInt_NewSize((size_t)(ptr - scan_str));
not_found:
	DeeRT_ErrSubstringNotFound(Dee_AsObject(self), args.close, args.start, args.end);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_rindexmatch(Bytes *self, size_t argc, DeeObject *const *argv) {
	byte_t const *ptr, *scan_str;
	size_t scan_len;
	Needle s_open, s_clos;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("rindexmatch", params: "
	DeeObject *open: ?X3?.?Dstring?Dint;
	DeeObject *close: ?X3?.?Dstring?Dint;
	size_t start = 0;
	size_t end = (size_t)-1;
", docStringPrefix: "bytes");]]]*/
#define bytes_rindexmatch_params "open:?X3?.?Dstring?Dint,close:?X3?.?Dstring?Dint,start=!0,end=!-1"
	struct {
		DeeObject *open;
		DeeObject *close;
		size_t start;
		size_t end;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	if (DeeArg_UnpackStruct(argc, argv, "oo|" UNPuSIZ UNPxSIZ ":rindexmatch", &args))
		goto err;
/*[[[end]]]*/
	if (acquire_needle(&s_open, args.open))
		goto err;
	if (acquire_needle(&s_clos, args.close))
		goto err;
	scan_len = DeeBytes_SIZE(self);
	CLAMP_SUBSTR(&args.start, &args.end, &scan_len, not_found);
	scan_str = DeeBytes_DATA(self);
	ptr = dee_rfind_matchb(scan_str + args.start, scan_len,
	                       s_open.n_data, s_open.n_size,
	                       s_clos.n_data, s_clos.n_size);
	if unlikely(!ptr)
		goto not_found;
	return DeeInt_NewSize((size_t)(ptr - scan_str));
not_found:
	DeeRT_ErrSubstringNotFound(Dee_AsObject(self), args.open, args.start, args.end);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_casefindmatch(Bytes *self, size_t argc, DeeObject *const *argv) {
	byte_t const *ptr, *scan_str;
	size_t scan_len;
	Needle s_open, s_clos;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("casefindmatch", params: "
	DeeObject *open: ?X3?.?Dstring?Dint;
	DeeObject *close: ?X3?.?Dstring?Dint;
	size_t start = 0;
	size_t end = (size_t)-1;
", docStringPrefix: "bytes");]]]*/
#define bytes_casefindmatch_params "open:?X3?.?Dstring?Dint,close:?X3?.?Dstring?Dint,start=!0,end=!-1"
	struct {
		DeeObject *open;
		DeeObject *close;
		size_t start;
		size_t end;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	if (DeeArg_UnpackStruct(argc, argv, "oo|" UNPuSIZ UNPxSIZ ":casefindmatch", &args))
		goto err;
/*[[[end]]]*/
	if (acquire_needle(&s_open, args.open))
		goto err;
	if (acquire_needle(&s_clos, args.close))
		goto err;
	scan_len = DeeBytes_SIZE(self);
	CLAMP_SUBSTR(&args.start, &args.end, &scan_len, not_found);
	scan_str = DeeBytes_DATA(self);
	ptr = dee_find_casematch_ascii(scan_str + args.start, scan_len,
	                               s_open.n_data, s_open.n_size,
	                               s_clos.n_data, s_clos.n_size);
	if unlikely(!ptr)
		goto not_found;
	return DeeTuple_NewII((size_t)(ptr - scan_str), s_clos.n_size);
not_found:
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_caserfindmatch(Bytes *self, size_t argc, DeeObject *const *argv) {
	byte_t const *ptr, *scan_str;
	size_t scan_len, result;
	Needle s_open, s_clos;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("caserfindmatch", params: "
	DeeObject *open: ?X3?.?Dstring?Dint;
	DeeObject *close: ?X3?.?Dstring?Dint;
	size_t start = 0;
	size_t end = (size_t)-1;
", docStringPrefix: "bytes");]]]*/
#define bytes_caserfindmatch_params "open:?X3?.?Dstring?Dint,close:?X3?.?Dstring?Dint,start=!0,end=!-1"
	struct {
		DeeObject *open;
		DeeObject *close;
		size_t start;
		size_t end;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	if (DeeArg_UnpackStruct(argc, argv, "oo|" UNPuSIZ UNPxSIZ ":caserfindmatch", &args))
		goto err;
/*[[[end]]]*/
	if (acquire_needle(&s_open, args.open))
		goto err;
	if (acquire_needle(&s_clos, args.close))
		goto err;
	scan_len = DeeBytes_SIZE(self);
	CLAMP_SUBSTR(&args.start, &args.end, &scan_len, not_found);
	scan_str = DeeBytes_DATA(self);
	ptr = dee_rfind_casematch_ascii(scan_str + args.start, scan_len,
	                                s_open.n_data, s_open.n_size,
	                                s_clos.n_data, s_clos.n_size);
	if unlikely(!ptr)
		goto not_found;
	result = (size_t)(ptr - scan_str);
	return DeeTuple_NewII(result, result + s_open.n_size);
not_found:
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_caseindexmatch(Bytes *self, size_t argc, DeeObject *const *argv) {
	byte_t const *ptr, *scan_str;
	size_t scan_len, result;
	Needle s_open, s_clos;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("caseindexmatch", params: "
	DeeObject *open: ?X3?.?Dstring?Dint;
	DeeObject *close: ?X3?.?Dstring?Dint;
	size_t start = 0;
	size_t end = (size_t)-1;
", docStringPrefix: "bytes");]]]*/
#define bytes_caseindexmatch_params "open:?X3?.?Dstring?Dint,close:?X3?.?Dstring?Dint,start=!0,end=!-1"
	struct {
		DeeObject *open;
		DeeObject *close;
		size_t start;
		size_t end;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	if (DeeArg_UnpackStruct(argc, argv, "oo|" UNPuSIZ UNPxSIZ ":caseindexmatch", &args))
		goto err;
/*[[[end]]]*/
	if (acquire_needle(&s_open, args.open))
		goto err;
	if (acquire_needle(&s_clos, args.close))
		goto err;
	scan_len = DeeBytes_SIZE(self);
	CLAMP_SUBSTR(&args.start, &args.end, &scan_len, not_found);
	scan_str = DeeBytes_DATA(self);
	ptr = dee_find_casematch_ascii(scan_str + args.start, scan_len,
	                               s_open.n_data, s_open.n_size,
	                               s_clos.n_data, s_clos.n_size);
	if unlikely(!ptr)
		goto not_found;
	result = (size_t)(ptr - scan_str);
	return DeeTuple_NewII(result, result + s_clos.n_size);
not_found:
	DeeRT_ErrSubstringNotFound(Dee_AsObject(self), args.close, args.start, args.end);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_caserindexmatch(Bytes *self, size_t argc, DeeObject *const *argv) {
	byte_t const *ptr, *scan_str;
	size_t scan_len, result;
	Needle s_open, s_clos;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("caserindexmatch", params: "
	DeeObject *open: ?X3?.?Dstring?Dint;
	DeeObject *close: ?X3?.?Dstring?Dint;
	size_t start = 0;
	size_t end = (size_t)-1;
", docStringPrefix: "bytes");]]]*/
#define bytes_caserindexmatch_params "open:?X3?.?Dstring?Dint,close:?X3?.?Dstring?Dint,start=!0,end=!-1"
	struct {
		DeeObject *open;
		DeeObject *close;
		size_t start;
		size_t end;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	if (DeeArg_UnpackStruct(argc, argv, "oo|" UNPuSIZ UNPxSIZ ":caserindexmatch", &args))
		goto err;
/*[[[end]]]*/
	if (acquire_needle(&s_open, args.open))
		goto err;
	if (acquire_needle(&s_clos, args.close))
		goto err;
	scan_len = DeeBytes_SIZE(self);
	CLAMP_SUBSTR(&args.start, &args.end, &scan_len, not_found);
	scan_str = DeeBytes_DATA(self);
	ptr = dee_rfind_casematch_ascii(scan_str + args.start, scan_len,
	                                s_open.n_data, s_open.n_size,
	                                s_clos.n_data, s_clos.n_size);
	if unlikely(!ptr)
		goto not_found;
	result = (size_t)(ptr - scan_str);
	return DeeTuple_NewII(result, result + s_open.n_size);
not_found:
	DeeRT_ErrSubstringNotFound(Dee_AsObject(self), args.open, args.start, args.end);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_partitionmatch(Bytes *self, size_t argc, DeeObject *const *argv) {
	DREF DeeTupleObject *result;
	byte_t *scan_str, *match_start, *match_end;
	size_t scan_len;
	Needle s_open, s_clos;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("partitionmatch", params: "
	DeeObject *open: ?X3?.?Dstring?Dint;
	DeeObject *close: ?X3?.?Dstring?Dint;
	size_t start = 0;
	size_t end = (size_t)-1;
", docStringPrefix: "bytes");]]]*/
#define bytes_partitionmatch_params "open:?X3?.?Dstring?Dint,close:?X3?.?Dstring?Dint,start=!0,end=!-1"
	struct {
		DeeObject *open;
		DeeObject *close;
		size_t start;
		size_t end;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	if (DeeArg_UnpackStruct(argc, argv, "oo|" UNPuSIZ UNPxSIZ ":partitionmatch", &args))
		goto err;
/*[[[end]]]*/
	if (acquire_needle(&s_open, args.open))
		goto err;
	if (acquire_needle(&s_clos, args.close))
		goto err;
#define SET_BYTES(a, b, c)                                 \
	do {                                                   \
		if ((result->t_elem[0] = Dee_AsObject(a)) == NULL) \
			goto err_r_0;                                  \
		if ((result->t_elem[1] = Dee_AsObject(b)) == NULL) \
			goto err_r_1;                                  \
		if ((result->t_elem[2] = Dee_AsObject(c)) == NULL) \
			goto err_r_2;                                  \
	}	__WHILE0
	result = DeeTuple_NewUninitialized(3);
	if unlikely(!result)
		goto err;
	scan_len = DeeBytes_SIZE(self);
	CLAMP_SUBSTR(&args.start, &args.end, &scan_len, not_found);
	scan_str    = DeeBytes_DATA(self);
	match_start = memmemb(scan_str + args.start, scan_len,
	                      s_open.n_data, s_open.n_size);
	if unlikely(!match_start)
		goto not_found;
	match_end = (byte_t *)dee_find_matchb(match_start + s_open.n_size,
	                                      scan_len - (match_start - (scan_str + args.start)),
	                                      s_open.n_data, s_open.n_size,
	                                      s_clos.n_data, s_clos.n_size);
	if unlikely(!match_end)
		goto not_found;
	SET_BYTES(DeeBytes_NewSubView(self, scan_str,
	                              (size_t)(match_start - scan_str)),
	          DeeBytes_NewSubView(self, match_start,
	                              (match_end + s_clos.n_size) -
	                              match_start),
	          DeeBytes_NewSubView(self, match_end + s_clos.n_size,
	                              (size_t)(scan_str + args.end) -
	                              (size_t)(match_end + s_clos.n_size)));
#undef SET_BYTES
done:
	return Dee_AsObject(result);
not_found:
	result->t_elem[0] = Dee_AsObject(bytes_getsubstr_locked(self, args.start, args.end));
	if unlikely(!result->t_elem[0])
		goto err_r_0;
	result->t_elem[1] = Dee_EmptyBytes;
	result->t_elem[2] = Dee_EmptyBytes;
	Dee_Incref_n(Dee_EmptyBytes, 2);
	goto done;
err_r_2:
	Dee_Decref_likely(result->t_elem[1]);
err_r_1:
	Dee_Decref_likely(result->t_elem[0]);
err_r_0:
	DeeTuple_FreeUninitialized(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_rpartitionmatch(Bytes *self, size_t argc, DeeObject *const *argv) {
	DREF DeeTupleObject *result;
	byte_t *scan_str, *match_start, *match_end;
	size_t scan_len;
	Needle s_open, s_clos;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("rpartitionmatch", params: "
	DeeObject *open: ?X3?.?Dstring?Dint;
	DeeObject *close: ?X3?.?Dstring?Dint;
	size_t start = 0;
	size_t end = (size_t)-1;
", docStringPrefix: "bytes");]]]*/
#define bytes_rpartitionmatch_params "open:?X3?.?Dstring?Dint,close:?X3?.?Dstring?Dint,start=!0,end=!-1"
	struct {
		DeeObject *open;
		DeeObject *close;
		size_t start;
		size_t end;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	if (DeeArg_UnpackStruct(argc, argv, "oo|" UNPuSIZ UNPxSIZ ":rpartitionmatch", &args))
		goto err;
/*[[[end]]]*/
	if (acquire_needle(&s_open, args.open))
		goto err;
	if (acquire_needle(&s_clos, args.close))
		goto err;
#define SET_BYTES(a, b, c)                                 \
	do {                                                   \
		if ((result->t_elem[0] = Dee_AsObject(a)) == NULL) \
			goto err_r_0;                                  \
		if ((result->t_elem[1] = Dee_AsObject(b)) == NULL) \
			goto err_r_1;                                  \
		if ((result->t_elem[2] = Dee_AsObject(c)) == NULL) \
			goto err_r_2;                                  \
	}	__WHILE0
	result = DeeTuple_NewUninitialized(3);
	if unlikely(!result)
		goto err;
	scan_len = DeeBytes_SIZE(self);
	CLAMP_SUBSTR(&args.start, &args.end, &scan_len, not_found);
	scan_str  = DeeBytes_DATA(self);
	match_end = memrmemb(scan_str + args.start, scan_len,
	                     s_clos.n_data, s_clos.n_size);
	if unlikely(!match_end)
		goto not_found;
	match_start = (byte_t *)dee_rfind_matchb(scan_str + args.start,
	                                         (size_t)(match_end - (scan_str + args.start)),
	                                         s_open.n_data, s_open.n_size,
	                                         s_clos.n_data, s_clos.n_size);
	if unlikely(!match_start)
		goto not_found;
	SET_BYTES(DeeBytes_NewSubView(self, scan_str,
	                              (size_t)(match_start - scan_str)),
	          DeeBytes_NewSubView(self, match_start,
	                              (size_t)((match_end + s_clos.n_size) -
	                                       match_start)),
	          DeeBytes_NewSubView(self, match_end + s_clos.n_size,
	                              (size_t)(scan_str + args.end) -
	                              (size_t)(match_end + s_clos.n_size)));
#undef SET_BYTES
done:
	return Dee_AsObject(result);
not_found:
	result->t_elem[2] = Dee_AsObject(bytes_getsubstr_locked(self, args.start, args.end));
	if unlikely(!result->t_elem[2])
		goto err_r_0;
	Dee_Incref_n(Dee_EmptyString, 2);
	result->t_elem[0] = Dee_EmptyString;
	result->t_elem[2] = Dee_EmptyString;
	goto done;
err_r_2:
	Dee_Decref_likely(result->t_elem[1]);
err_r_1:
	Dee_Decref_likely(result->t_elem[0]);
err_r_0:
	DeeTuple_FreeUninitialized(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_casepartitionmatch(Bytes *self, size_t argc, DeeObject *const *argv) {
	DREF DeeTupleObject *result;
	byte_t *scan_str, *match_start, *match_end;
	size_t scan_len;
	Needle s_open, s_clos;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("casepartitionmatch", params: "
	DeeObject *open: ?X3?.?Dstring?Dint;
	DeeObject *close: ?X3?.?Dstring?Dint;
	size_t start = 0;
	size_t end = (size_t)-1;
", docStringPrefix: "bytes");]]]*/
#define bytes_casepartitionmatch_params "open:?X3?.?Dstring?Dint,close:?X3?.?Dstring?Dint,start=!0,end=!-1"
	struct {
		DeeObject *open;
		DeeObject *close;
		size_t start;
		size_t end;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	if (DeeArg_UnpackStruct(argc, argv, "oo|" UNPuSIZ UNPxSIZ ":casepartitionmatch", &args))
		goto err;
/*[[[end]]]*/
	if (acquire_needle(&s_open, args.open))
		goto err;
	if (acquire_needle(&s_clos, args.close))
		goto err;
#define SET_BYTES(a, b, c)                                 \
	do {                                                   \
		if ((result->t_elem[0] = Dee_AsObject(a)) == NULL) \
			goto err_r_0;                                  \
		if ((result->t_elem[1] = Dee_AsObject(b)) == NULL) \
			goto err_r_1;                                  \
		if ((result->t_elem[2] = Dee_AsObject(c)) == NULL) \
			goto err_r_2;                                  \
	}	__WHILE0
	result = DeeTuple_NewUninitialized(3);
	if unlikely(!result)
		goto err;
	scan_len = DeeBytes_SIZE(self);
	CLAMP_SUBSTR(&args.start, &args.end, &scan_len, not_found);
	scan_str = DeeBytes_DATA(self);
	match_start = memasciicasemem(scan_str + args.start, scan_len,
	                              s_open.n_data, s_open.n_size);
	if unlikely(!match_start)
		goto not_found;
	match_end = (byte_t *)dee_find_casematch_ascii(match_start + s_open.n_size,
	                                               scan_len - (match_start - (scan_str + args.start)),
	                                               s_open.n_data, s_open.n_size,
	                                               s_clos.n_data, s_clos.n_size);
	if unlikely(!match_end)
		goto not_found;
	SET_BYTES(DeeBytes_NewSubView(self, scan_str,
	                              (size_t)(match_start - scan_str)),
	          DeeBytes_NewSubView(self, match_start,
	                              (match_end + s_clos.n_size) -
	                              match_start),
	          DeeBytes_NewSubView(self, match_end + s_clos.n_size,
	                              (size_t)(scan_str + args.end) -
	                              (size_t)(match_end + s_clos.n_size)));
#undef SET_BYTES
done:
	return Dee_AsObject(result);
not_found:
	result->t_elem[0] = Dee_AsObject(bytes_getsubstr_locked(self, args.start, args.end));
	if unlikely(!result->t_elem[0])
		goto err_r_0;
	Dee_Incref_n(Dee_EmptyBytes, 2);
	result->t_elem[1] = Dee_EmptyBytes;
	result->t_elem[2] = Dee_EmptyBytes;
	goto done;
err_r_2:
	Dee_Decref_likely(result->t_elem[1]);
err_r_1:
	Dee_Decref_likely(result->t_elem[0]);
err_r_0:
	DeeTuple_FreeUninitialized(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_caserpartitionmatch(Bytes *self, size_t argc, DeeObject *const *argv) {
	DREF DeeTupleObject *result;
	byte_t *scan_str, *match_start, *match_end;
	size_t scan_len;
	Needle s_open, s_clos;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("caserpartitionmatch", params: "
	DeeObject *open: ?X3?.?Dstring?Dint;
	DeeObject *close: ?X3?.?Dstring?Dint;
	size_t start = 0;
	size_t end = (size_t)-1;
", docStringPrefix: "bytes");]]]*/
#define bytes_caserpartitionmatch_params "open:?X3?.?Dstring?Dint,close:?X3?.?Dstring?Dint,start=!0,end=!-1"
	struct {
		DeeObject *open;
		DeeObject *close;
		size_t start;
		size_t end;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	if (DeeArg_UnpackStruct(argc, argv, "oo|" UNPuSIZ UNPxSIZ ":caserpartitionmatch", &args))
		goto err;
/*[[[end]]]*/
	if (acquire_needle(&s_open, args.open))
		goto err;
	if (acquire_needle(&s_clos, args.close))
		goto err;
#define SET_BYTES(a, b, c)                                 \
	do {                                                   \
		if ((result->t_elem[0] = Dee_AsObject(a)) == NULL) \
			goto err_r_0;                                  \
		if ((result->t_elem[1] = Dee_AsObject(b)) == NULL) \
			goto err_r_1;                                  \
		if ((result->t_elem[2] = Dee_AsObject(c)) == NULL) \
			goto err_r_2;                                  \
	}	__WHILE0
	result = DeeTuple_NewUninitialized(3);
	if unlikely(!result)
		goto err;
	scan_len = DeeBytes_SIZE(self);
	CLAMP_SUBSTR(&args.start, &args.end, &scan_len, not_found);
	scan_str  = DeeBytes_DATA(self);
	match_end = memasciicasermem(scan_str + args.start, scan_len,
	                             s_clos.n_data, s_clos.n_size);
	if unlikely(!match_end)
		goto not_found;
	match_start = (byte_t *)dee_rfind_casematch_ascii(scan_str + args.start,
	                                                  (size_t)(match_end - (scan_str + args.start)),
	                                                  s_open.n_data, s_open.n_size,
	                                                  s_clos.n_data, s_clos.n_size);
	if unlikely(!match_start)
		goto not_found;
	SET_BYTES(DeeBytes_NewSubView(self, scan_str,
	                              (size_t)(match_start - scan_str)),
	          DeeBytes_NewSubView(self, match_start,
	                              (size_t)((match_end + s_clos.n_size) -
	                                       match_start)),
	          DeeBytes_NewSubView(self, match_end + s_clos.n_size,
	                              (size_t)(scan_str + args.end) -
	                              (size_t)(match_end + s_clos.n_size)));
#undef SET_BYTES
done:
	return Dee_AsObject(result);
not_found:
	result->t_elem[2] = Dee_AsObject(bytes_getsubstr_locked(self, args.start, args.end));
	if unlikely(!result->t_elem[2])
		goto err_r_0;
	Dee_Incref_n(Dee_EmptyString, 2);
	result->t_elem[0] = Dee_EmptyString;
	result->t_elem[1] = Dee_EmptyString;
	goto done;
err_r_2:
	Dee_Decref_likely(result->t_elem[1]);
err_r_1:
	Dee_Decref_likely(result->t_elem[0]);
err_r_0:
	DeeTuple_FreeUninitialized(result);
err:
	return NULL;
}

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeBytes_Segments(DeeBytesObject *__restrict self, size_t substring_length);

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_segments(Bytes *self, size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("segments", params: "
	size_t substring_length
", docStringPrefix: "bytes");]]]*/
#define bytes_segments_params "substring_length:?Dint"
	struct {
		size_t substring_length;
	} args;
	DeeArg_Unpack1X(err, argc, argv, "segments", &args.substring_length, UNPuSIZ, DeeObject_AsSize);
/*[[[end]]]*/
	if unlikely(!args.substring_length) {
		err_invalid_segment_size(args.substring_length);
		goto err;
	}
	return DeeBytes_Segments(self, args.substring_length);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_distribute(Bytes *self, size_t argc, DeeObject *const *argv) {
	size_t substring_length;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("distribute", params: "
	size_t substring_count
", docStringPrefix: "bytes");]]]*/
#define bytes_distribute_params "substring_count:?Dint"
	struct {
		size_t substring_count;
	} args;
	DeeArg_Unpack1X(err, argc, argv, "distribute", &args.substring_count, UNPuSIZ, DeeObject_AsSize);
/*[[[end]]]*/
	if unlikely(!args.substring_count) {
		err_invalid_distribution_count(args.substring_count);
		goto err;
	}
	substring_length = DeeBytes_SIZE(self);
	substring_length += args.substring_count - 1;
	substring_length /= args.substring_count;
	if unlikely(!substring_length)
		return DeeSeq_NewEmpty();
	return DeeBytes_Segments(self, substring_length);
err:
	return NULL;
}


/************************************************************************/
/* Regex functions                                                      */
/************************************************************************/
#ifdef __INTELLISENSE__ /* Stuff we share with "./string_functions.c" */
struct DeeRegexExecWithRange {
	struct DeeRegexExec rewr_exec;    /* Normal exec args */
	size_t              rewr_range;   /* Max # of search attempts to perform (in bytes) */
	DeeStringObject    *rewr_pattern; /* [1..1] Pattern string that is being used */
	DeeStringObject    *rewr_rules;   /* [0..1] Pattern rules */
};

struct DeeRegexBaseExec {
	DREF String               *rx_pattern;  /* [1..1] Pattern string (only a reference within objects in "./reproxy.c.inl") */
	struct DeeRegexCode const *rx_code;     /* [1..1] Regex code */
	void const                *rx_inbase;   /* [0..rx_insize][valid_if(rx_startoff < rx_endoff)] Input data to scan
	                                         * When `rx_code' was compiled with `Dee_RE_COMPILE_NOUTF8', this data
	                                         * is treated as raw bytes; otherwise, it is treated as a utf-8 string.
	                                         * In either case, `rx_insize' is the # of bytes within this buffer. */
	size_t                     rx_insize;   /* Total # of bytes starting at `rx_inbase' */
	size_t                     rx_startoff; /* Starting byte offset into `rx_inbase' of data to match. */
	size_t                     rx_endoff;   /* Ending byte offset into `rx_inbase' of data to match. */
	unsigned int               rx_eflags;   /* Execution-flags (set of `Dee_RE_EXEC_*') */
};

/* Functions from "./reproxy.c.inl" */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bytes_re_findall(DeeBytesObject *__restrict self,
                 struct DeeRegexBaseExec const *__restrict exec);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bytes_reg_findall(DeeBytesObject *__restrict self,
                 struct DeeRegexBaseExec const *__restrict exec);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bytes_re_locateall(DeeBytesObject *__restrict self,
                   struct DeeRegexBaseExec const *__restrict exec);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bytes_reg_locateall(DeeBytesObject *__restrict self,
                    struct DeeRegexBaseExec const *__restrict exec);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bytes_re_split(DeeBytesObject *__restrict self,
               struct DeeRegexBaseExec const *__restrict exec);
#endif /* __INTELLISENSE__ */
#define bytes_rereplace_kwlist     kwlist__pattern_replace_max_rules
#define bytes_generic_regex_kwlist kwlist__pattern_start_end_rules
#define bytes_search_regex_kwlist  kwlist__pattern_start_end_range_rules

#define BYTES_GENERIC_REGEX_GETARGS_FMT(name) "o|" UNPuSIZ UNPxSIZ "o:" name
PRIVATE WUNUSED NONNULL((1, 5, 6)) int DCALL
bytes_generic_regex_getargs(Bytes *self, size_t argc, DeeObject *const *argv,
                            DeeObject *kw, char const *__restrict fmt,
                            struct DeeRegexExec *__restrict result) {
	DeeObject *pattern, *rules = NULL;
	result->rx_startoff = 0;
	result->rx_endoff   = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, bytes_generic_regex_kwlist, fmt,
	                    &pattern, &result->rx_startoff, &result->rx_endoff,
	                    &rules))
		goto err;
	if (DeeObject_AssertTypeExact(pattern, &DeeString_Type))
		goto err;
	result->rx_code = DeeString_GetRegex(pattern, Dee_RE_COMPILE_NOUTF8, rules);
	if unlikely(!result->rx_code)
		goto err;
	result->rx_nmatch = 0;
	result->rx_pmatch = NULL;
	result->rx_eflags = 0; /* TODO: NOTBOL/NOTEOL */
	result->rx_inbase = DeeBytes_DATA(self);
	result->rx_insize = DeeBytes_SIZE(self);
	if (result->rx_endoff > result->rx_insize)
		result->rx_endoff = result->rx_insize;
	return 0;
err:
	return -1;
}

#define BYTES_SEARCH_REGEX_GETARGS_FMT(name) "o|" UNPuSIZ UNPxSIZ UNPxSIZ "o:" name
PRIVATE WUNUSED NONNULL((1, 5, 6)) int DCALL
bytes_search_regex_getargs(Bytes *self, size_t argc, DeeObject *const *argv,
                           DeeObject *kw, char const *__restrict fmt,
                           struct DeeRegexExecWithRange *__restrict result) {
	result->rewr_exec.rx_startoff = 0;
	result->rewr_exec.rx_endoff   = (size_t)-1;
	result->rewr_range = (size_t)-1;
	result->rewr_rules = NULL;
	if (DeeArg_UnpackKw(argc, argv, kw, bytes_search_regex_kwlist, fmt,
	                    &result->rewr_pattern, &result->rewr_exec.rx_startoff,
	                    &result->rewr_exec.rx_endoff,
	                    &result->rewr_range, &result->rewr_rules))
		goto err;
	if (DeeObject_AssertTypeExact(result->rewr_pattern, &DeeString_Type))
		goto err;
	result->rewr_exec.rx_code = DeeString_GetRegex((DeeObject *)result->rewr_pattern,
	                                               Dee_RE_COMPILE_NOUTF8,
	                                               (DeeObject *)result->rewr_rules);
	if unlikely(!result->rewr_exec.rx_code)
		goto err;
	result->rewr_exec.rx_nmatch = 0;
	result->rewr_exec.rx_pmatch = NULL;
	result->rewr_exec.rx_eflags = 0; /* TODO: NOTBOL/NOTEOL */
	result->rewr_exec.rx_inbase = DeeBytes_DATA(self);
	result->rewr_exec.rx_insize = DeeBytes_SIZE(self);
	if (result->rewr_exec.rx_endoff > result->rewr_exec.rx_insize)
		result->rewr_exec.rx_endoff = result->rewr_exec.rx_insize;
	return 0;
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_rematch(Bytes *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	Dee_ssize_t result;
	struct DeeRegexExec exec;
	if unlikely(bytes_generic_regex_getargs(self, argc, argv, kw,
	                                        BYTES_GENERIC_REGEX_GETARGS_FMT("rematch"),
	                                        &exec))
		goto err;
	result = DeeRegex_Match(&exec);
	if unlikely(result == Dee_RE_STATUS_ERROR)
		goto err;
	if (result == Dee_RE_STATUS_NOMATCH)
		return_none;
	return DeeInt_NewSize((size_t)result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_regmatch(Bytes *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DREF ReGroups *result;
	Dee_ssize_t status;
	struct DeeRegexExec exec;
	if unlikely(bytes_generic_regex_getargs(self, argc, argv, kw,
	                                        BYTES_GENERIC_REGEX_GETARGS_FMT("regmatch"),
	                                        &exec))
		goto err;
	result = ReGroups_Malloc(1 + exec.rx_code->rc_ngrps);
	if unlikely(!result) {
		goto err;
	}
	exec.rx_nmatch = exec.rx_code->rc_ngrps;
	exec.rx_pmatch = result->rg_groups + 1;
	status = DeeRegex_Match(&exec);
	if unlikely(status == Dee_RE_STATUS_ERROR)
		goto err_r;
	if (status == Dee_RE_STATUS_NOMATCH) {
		ReGroups_Free(result);
		return DeeSeq_NewEmpty();
	}
	result->rg_groups[0].rm_so = 0;
	result->rg_groups[0].rm_eo = (size_t)status;
	ReGroups_Init(result, 1 + exec.rx_code->rc_ngrps);
	return Dee_AsObject(result);
err_r:
	ReGroups_Free(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_rematches(Bytes *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	Dee_ssize_t status;
	struct DeeRegexExec exec;
	if unlikely(bytes_generic_regex_getargs(self, argc, argv, kw,
	                                        BYTES_GENERIC_REGEX_GETARGS_FMT("rematches"),
	                                        &exec))
		goto err;
	status = DeeRegex_Match(&exec);
	if unlikely(status == Dee_RE_STATUS_ERROR)
		goto err;
	if (status == Dee_RE_STATUS_NOMATCH)
		return_false;
	ASSERT((size_t)status <= exec.rx_endoff);
	return_bool((size_t)status >= exec.rx_endoff);
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_refind(Bytes *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	Dee_ssize_t status;
	size_t match_size;
	struct DeeRegexExecWithRange exec;
	if unlikely(bytes_search_regex_getargs(self, argc, argv, kw,
	                                       BYTES_SEARCH_REGEX_GETARGS_FMT("refind"),
	                                       &exec))
		goto err;
	status = DeeRegex_Search(&exec.rewr_exec, exec.rewr_range, &match_size);
	if unlikely(status == Dee_RE_STATUS_ERROR)
		goto err;
	if (status == Dee_RE_STATUS_NOMATCH)
		return_none;
	match_size += (size_t)status;
	return DeeTuple_NewII((size_t)status, (size_t)match_size);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_rerfind(Bytes *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	Dee_ssize_t status;
	size_t match_size;
	struct DeeRegexExecWithRange exec;
	if unlikely(bytes_search_regex_getargs(self, argc, argv, kw,
	                                       BYTES_SEARCH_REGEX_GETARGS_FMT("rerfind"),
	                                       &exec))
		goto err;
	status = DeeRegex_RSearch(&exec.rewr_exec, exec.rewr_range, &match_size);
	if unlikely(status == Dee_RE_STATUS_ERROR)
		goto err;
	if (status == Dee_RE_STATUS_NOMATCH)
		return_none;
	match_size += (size_t)status;
	return DeeTuple_NewII((size_t)status, (size_t)match_size);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_rescanf(Bytes *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DREF ReSubBytes *result;
	Dee_ssize_t status;
	struct DeeRegexExec exec;
	if unlikely(bytes_generic_regex_getargs(self, argc, argv, kw,
	                                        BYTES_GENERIC_REGEX_GETARGS_FMT("rescanf"),
	                                        &exec))
		goto err;
	result = ReSubBytes_Malloc(exec.rx_code->rc_ngrps);
	if unlikely(!result) {
		goto err;
	}
	exec.rx_nmatch = exec.rx_code->rc_ngrps;
	exec.rx_pmatch = result->rss_groups;
	status = DeeRegex_Match(&exec);
	if unlikely(status == Dee_RE_STATUS_ERROR)
		goto err_r;
	if (status == Dee_RE_STATUS_NOMATCH) {
		ReSubBytes_Free(result);
		return DeeSeq_NewEmpty();
	}
	ReSubBytes_InitBytes(result, Dee_AsObject(self),
	                     exec.rx_inbase,
	                     exec.rx_code->rc_ngrps);
	return Dee_AsObject(result);
err_r:
	ReSubBytes_Free(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_regfind(Bytes *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DREF ReGroups *result;
	Dee_ssize_t status;
	size_t match_size;
	struct DeeRegexExecWithRange exec;
	if unlikely(bytes_search_regex_getargs(self, argc, argv, kw,
	                                       BYTES_SEARCH_REGEX_GETARGS_FMT("regfind"),
	                                       &exec))
		goto err;
	result = ReGroups_Malloc(1 + exec.rewr_exec.rx_code->rc_ngrps);
	if unlikely(!result)
		goto err;
	exec.rewr_exec.rx_nmatch = exec.rewr_exec.rx_code->rc_ngrps;
	exec.rewr_exec.rx_pmatch = result->rg_groups + 1;
	status = DeeRegex_Search(&exec.rewr_exec, exec.rewr_range, &match_size);
	if unlikely(status == Dee_RE_STATUS_ERROR)
		goto err_r;
	if (status == Dee_RE_STATUS_NOMATCH) {
		ReGroups_Free(result);
		return DeeSeq_NewEmpty();
	}
	match_size += (size_t)status;
	result->rg_groups[0].rm_so = (size_t)status;
	result->rg_groups[0].rm_eo = match_size;
	ReGroups_Init(result, 1 + exec.rewr_exec.rx_code->rc_ngrps);
	return Dee_AsObject(result);
err_r:
	ReGroups_Free(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_regrfind(Bytes *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DREF ReGroups *result;
	Dee_ssize_t status;
	size_t match_size;
	struct DeeRegexExecWithRange exec;
	if unlikely(bytes_search_regex_getargs(self, argc, argv, kw,
	                                       BYTES_SEARCH_REGEX_GETARGS_FMT("regrfind"),
	                                       &exec))
		goto err;
	result = ReGroups_Malloc(1 + exec.rewr_exec.rx_code->rc_ngrps);
	if unlikely(!result)
		goto err;
	exec.rewr_exec.rx_nmatch = exec.rewr_exec.rx_code->rc_ngrps;
	exec.rewr_exec.rx_pmatch = result->rg_groups + 1;
	status = DeeRegex_RSearch(&exec.rewr_exec, exec.rewr_range, &match_size);
	if unlikely(status == Dee_RE_STATUS_ERROR)
		goto err_r;
	if (status == Dee_RE_STATUS_NOMATCH) {
		ReGroups_Free(result);
		return DeeSeq_NewEmpty();
	}
	match_size += (size_t)status;
	result->rg_groups[0].rm_so = (size_t)status;
	result->rg_groups[0].rm_eo = match_size;
	ReGroups_Init(result, 1 + exec.rewr_exec.rx_code->rc_ngrps);
	return Dee_AsObject(result);
err_r:
	ReGroups_Free(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_reglocate(Bytes *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DREF ReSubBytes *result;
	Dee_ssize_t status;
	size_t match_size;
	struct DeeRegexExecWithRange exec;
	if unlikely(bytes_search_regex_getargs(self, argc, argv, kw,
	                                       BYTES_SEARCH_REGEX_GETARGS_FMT("reglocate"),
	                                       &exec))
		goto err;
	result = ReSubBytes_Malloc(1 + exec.rewr_exec.rx_code->rc_ngrps);
	if unlikely(!result)
		goto err;
	exec.rewr_exec.rx_nmatch = exec.rewr_exec.rx_code->rc_ngrps;
	exec.rewr_exec.rx_pmatch = result->rss_groups + 1;
	status = DeeRegex_Search(&exec.rewr_exec, exec.rewr_range, &match_size);
	if unlikely(status == Dee_RE_STATUS_ERROR)
		goto err_r;
	if (status == Dee_RE_STATUS_NOMATCH) {
		ReGroups_Free(result);
		return DeeSeq_NewEmpty();
	}
	match_size += (size_t)status;
	result->rss_groups[0].rm_so = (size_t)status;
	result->rss_groups[0].rm_eo = match_size;
	ReSubBytes_InitBytes(result, Dee_AsObject(self),
	                     exec.rewr_exec.rx_inbase,
	                     1 + exec.rewr_exec.rx_code->rc_ngrps);
	return Dee_AsObject(result);
err_r:
	ReGroups_Free(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_regrlocate(Bytes *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DREF ReSubBytes *result;
	Dee_ssize_t status;
	size_t match_size;
	struct DeeRegexExecWithRange exec;
	if unlikely(bytes_search_regex_getargs(self, argc, argv, kw,
	                                       BYTES_SEARCH_REGEX_GETARGS_FMT("regrlocate"),
	                                       &exec))
		goto err;
	result = ReSubBytes_Malloc(1 + exec.rewr_exec.rx_code->rc_ngrps);
	if unlikely(!result)
		goto err;
	exec.rewr_exec.rx_nmatch = exec.rewr_exec.rx_code->rc_ngrps;
	exec.rewr_exec.rx_pmatch = result->rss_groups + 1;
	status = DeeRegex_RSearch(&exec.rewr_exec, exec.rewr_range, &match_size);
	if unlikely(status == Dee_RE_STATUS_ERROR)
		goto err_r;
	if (status == Dee_RE_STATUS_NOMATCH) {
		ReGroups_Free(result);
		return DeeSeq_NewEmpty();
	}
	match_size += (size_t)status;
	result->rss_groups[0].rm_so = (size_t)status;
	result->rss_groups[0].rm_eo = match_size;
	ReSubBytes_InitBytes(result, Dee_AsObject(self),
	                     exec.rewr_exec.rx_inbase,
	                     1 + exec.rewr_exec.rx_code->rc_ngrps);
	return Dee_AsObject(result);
err_r:
	ReGroups_Free(result);
err:
	return NULL;
}

PRIVATE ATTR_COLD NONNULL((1, 2)) int DCALL
err_regex_not_found_in_bytes(Bytes *self, struct DeeRegexExecWithRange const *__restrict exec) {
	return DeeRT_ErrRegexNotFound(Dee_AsObject(self),
	                              (DeeObject *)exec->rewr_pattern,
	                              exec->rewr_exec.rx_startoff,
	                              exec->rewr_exec.rx_endoff,
	                              exec->rewr_range,
	                              (DeeObject *)exec->rewr_rules,
	                              exec->rewr_exec.rx_eflags);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_reindex(Bytes *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	Dee_ssize_t status;
	size_t match_size;
	struct DeeRegexExecWithRange exec;
	if unlikely(bytes_search_regex_getargs(self, argc, argv, kw,
	                                       BYTES_SEARCH_REGEX_GETARGS_FMT("reindex"),
	                                       &exec))
		goto err;
	status = DeeRegex_Search(&exec.rewr_exec, exec.rewr_range, &match_size);
	if unlikely(status == Dee_RE_STATUS_ERROR)
		goto err;
	if (status == Dee_RE_STATUS_NOMATCH)
		goto not_found;
	match_size += (size_t)status;
	return DeeTuple_NewII((size_t)status, (size_t)match_size);
not_found:
	err_regex_not_found_in_bytes(self, &exec);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_rerindex(Bytes *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	Dee_ssize_t status;
	size_t match_size;
	struct DeeRegexExecWithRange exec;
	if unlikely(bytes_search_regex_getargs(self, argc, argv, kw,
	                                       BYTES_SEARCH_REGEX_GETARGS_FMT("rerindex"),
	                                       &exec))
		goto err;
	status = DeeRegex_RSearch(&exec.rewr_exec, exec.rewr_range, &match_size);
	if unlikely(status == Dee_RE_STATUS_ERROR)
		goto err;
	if (status == Dee_RE_STATUS_NOMATCH)
		goto not_found;
	match_size += (size_t)status;
	return DeeTuple_NewII((size_t)status, (size_t)match_size);
not_found:
	err_regex_not_found_in_bytes(self, &exec);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_regindex(Bytes *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DREF ReGroups *result;
	Dee_ssize_t status;
	size_t match_size;
	struct DeeRegexExecWithRange exec;
	if unlikely(bytes_search_regex_getargs(self, argc, argv, kw,
	                                       BYTES_SEARCH_REGEX_GETARGS_FMT("regindex"),
	                                       &exec))
		goto err;
	result = ReGroups_Malloc(1 + exec.rewr_exec.rx_code->rc_ngrps);
	if unlikely(!result)
		goto err;
	exec.rewr_exec.rx_nmatch = exec.rewr_exec.rx_code->rc_ngrps;
	exec.rewr_exec.rx_pmatch = result->rg_groups + 1;
	status = DeeRegex_Search(&exec.rewr_exec, exec.rewr_range, &match_size);
	if unlikely(status == Dee_RE_STATUS_ERROR)
		goto err_r;
	if (status == Dee_RE_STATUS_NOMATCH)
		goto not_found;
	match_size += (size_t)status;
	result->rg_groups[0].rm_so = (size_t)status;
	result->rg_groups[0].rm_eo = match_size;
	ReGroups_Init(result, 1 + exec.rewr_exec.rx_code->rc_ngrps);
	return Dee_AsObject(result);
not_found:
	err_regex_not_found_in_bytes(self, &exec);
err_r:
	ReGroups_Free(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_regrindex(Bytes *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DREF ReGroups *result;
	Dee_ssize_t status;
	size_t match_size;
	struct DeeRegexExecWithRange exec;
	if unlikely(bytes_search_regex_getargs(self, argc, argv, kw,
	                                       BYTES_SEARCH_REGEX_GETARGS_FMT("regrindex"),
	                                       &exec))
		goto err;
	result = ReGroups_Malloc(1 + exec.rewr_exec.rx_code->rc_ngrps);
	if unlikely(!result)
		goto err;
	exec.rewr_exec.rx_nmatch = exec.rewr_exec.rx_code->rc_ngrps;
	exec.rewr_exec.rx_pmatch = result->rg_groups + 1;
	status = DeeRegex_RSearch(&exec.rewr_exec, exec.rewr_range, &match_size);
	if unlikely(status == Dee_RE_STATUS_ERROR)
		goto err_r;
	if (status == Dee_RE_STATUS_NOMATCH)
		goto not_found;
	match_size += (size_t)status;
	result->rg_groups[0].rm_so = (size_t)status;
	result->rg_groups[0].rm_eo = match_size;
	ReGroups_Init(result, 1 + exec.rewr_exec.rx_code->rc_ngrps);
	return Dee_AsObject(result);
not_found:
	err_regex_not_found_in_bytes(self, &exec);
err_r:
	ReGroups_Free(result);
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
bytes_relocate(Bytes *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *result;
	Dee_ssize_t status;
	size_t match_size;
	struct DeeRegexExecWithRange exec;
	if unlikely(bytes_search_regex_getargs(self, argc, argv, kw,
	                                       BYTES_SEARCH_REGEX_GETARGS_FMT("relocate"),
	                                       &exec))
		goto err;
	status = DeeRegex_Search(&exec.rewr_exec, exec.rewr_range, &match_size);
	if unlikely(status == Dee_RE_STATUS_ERROR)
		goto err;
	if (status == Dee_RE_STATUS_NOMATCH) {
		result = DeeNone_NewRef();
	} else {
		result = DeeBytes_NewSubView(self,
		                             (void *)((char const *)exec.rewr_exec.rx_inbase +
		                                      (size_t)status),
		                             match_size);
	}
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
bytes_rerlocate(Bytes *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *result;
	Dee_ssize_t status;
	size_t match_size;
	struct DeeRegexExecWithRange exec;
	if unlikely(bytes_search_regex_getargs(self, argc, argv, kw,
	                                       BYTES_SEARCH_REGEX_GETARGS_FMT("rerlocate"),
	                                       &exec))
		goto err;
	status = DeeRegex_RSearch(&exec.rewr_exec, exec.rewr_range, &match_size);
	if unlikely(status == Dee_RE_STATUS_ERROR)
		goto err;
	if (status == Dee_RE_STATUS_NOMATCH) {
		result = DeeNone_NewRef();
	} else {
		result = DeeBytes_NewSubView(self,
		                             (void *)((char const *)exec.rewr_exec.rx_inbase + (size_t)status),
		                             match_size);
	}
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeTupleObject *DCALL
bytes_pack_partition_not_found(Bytes *__restrict self,
                               char const *__restrict bytes_base,
                               size_t startoff, size_t endoff,
                               bool is_rpartition) {
	DREF DeeTupleObject *result;
	DREF DeeObject *str0;
	result = DeeTuple_NewUninitialized(3);
	if unlikely(!result)
		goto err;
	if (startoff < endoff) {
		str0 = DeeBytes_NewSubView(self,
		                           (void *)(bytes_base + startoff),
		                           endoff - startoff);
		if unlikely(!str0)
			goto err_r;
	} else {
		str0 = DeeString_NewEmpty();
	}
	result->t_elem[1] = Dee_AsObject(&DeeString_Empty);
	Dee_Incref_n(&DeeString_Empty, 2);
	if (is_rpartition) {
		result->t_elem[0] = Dee_AsObject(&DeeString_Empty);
		result->t_elem[2] = str0;
	} else {
		result->t_elem[0] = str0;
		result->t_elem[2] = Dee_AsObject(&DeeString_Empty);
	}
	return result;
err_r:
	DeeTuple_FreeUninitialized(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeTupleObject *DCALL
bytes_pack_partition_found(Bytes *__restrict self,
                           char const *__restrict bytes_base,
                           size_t match_startoff,
                           size_t match_endoff,
                           size_t str_endoff) {
	DREF DeeTupleObject *result;
	DREF DeeObject *str;
	result = DeeTuple_NewUninitialized(3);
	if unlikely(!result)
		goto err;
	str = DeeBytes_NewSubView(self, (void *)bytes_base, match_startoff);
	if unlikely(!str)
		goto err_r_0;
	DeeTuple_SET(result, 0, str);
	str = DeeBytes_NewSubView(self, (void *)(bytes_base + match_startoff),
	                          match_endoff - match_startoff);
	if unlikely(!str)
		goto err_r_1;
	DeeTuple_SET(result, 1, str);
	str = DeeBytes_NewSubView(self, (void *)(bytes_base + match_endoff),
	                          str_endoff - match_endoff);
	if unlikely(!str)
		goto err_r_2;
	DeeTuple_SET(result, 2, str);
	return result;
err_r_2:
	Dee_Decref_likely(DeeTuple_GET(result, 1));
err_r_1:
	Dee_Decref_likely(DeeTuple_GET(result, 0));
err_r_0:
	DeeTuple_FreeUninitialized(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTupleObject *DCALL
bytes_repartition(Bytes *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	Dee_ssize_t status;
	size_t match_size;
	DREF DeeTupleObject *result;
	struct DeeRegexExecWithRange exec;
	if unlikely(bytes_search_regex_getargs(self, argc, argv, kw,
	                                       BYTES_SEARCH_REGEX_GETARGS_FMT("repartition"),
	                                       &exec))
		goto err;
	status = DeeRegex_Search(&exec.rewr_exec, exec.rewr_range, &match_size);
	if unlikely(status == Dee_RE_STATUS_ERROR)
		goto err;
	if (status == Dee_RE_STATUS_NOMATCH) {
		result = bytes_pack_partition_not_found(self,
		                                        (char const *)exec.rewr_exec.rx_inbase,
		                                        exec.rewr_exec.rx_startoff,
		                                        exec.rewr_exec.rx_endoff,
		                                        false);
	} else {
		status -= exec.rewr_exec.rx_startoff;
		exec.rewr_exec.rx_endoff -= exec.rewr_exec.rx_startoff;
		exec.rewr_exec.rx_inbase = (char const *)exec.rewr_exec.rx_inbase + exec.rewr_exec.rx_startoff;
		result = bytes_pack_partition_found(self,
		                                    (char const *)exec.rewr_exec.rx_inbase,
		                                    (size_t)status,
		                                    (size_t)status + match_size,
		                                    exec.rewr_exec.rx_endoff);
	}
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTupleObject *DCALL
bytes_rerpartition(Bytes *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	Dee_ssize_t status;
	size_t match_size;
	DREF DeeTupleObject *result;
	struct DeeRegexExecWithRange exec;
	if unlikely(bytes_search_regex_getargs(self, argc, argv, kw,
	                                       BYTES_SEARCH_REGEX_GETARGS_FMT("rerpartition"),
	                                       &exec))
		goto err;
	status = DeeRegex_RSearch(&exec.rewr_exec, exec.rewr_range, &match_size);
	if unlikely(status == Dee_RE_STATUS_ERROR)
		goto err;
	if (status == Dee_RE_STATUS_NOMATCH) {
		result = bytes_pack_partition_not_found(self,
		                                        (char const *)exec.rewr_exec.rx_inbase,
		                                        exec.rewr_exec.rx_startoff,
		                                        exec.rewr_exec.rx_endoff,
		                                        true);
	} else {
		status -= exec.rewr_exec.rx_startoff;
		exec.rewr_exec.rx_endoff -= exec.rewr_exec.rx_startoff;
		exec.rewr_exec.rx_inbase = (char const *)exec.rewr_exec.rx_inbase + exec.rewr_exec.rx_startoff;
		result = bytes_pack_partition_found(self,
		                                    (char const *)exec.rewr_exec.rx_inbase,
		                                    (size_t)status,
		                                    (size_t)status + match_size,
		                                    exec.rewr_exec.rx_endoff);
	}
	return result;
err:
	return NULL;
}


#ifndef CONFIG_HAVE_memsetp
#define CONFIG_HAVE_memsetp
#define memsetp(dst, pointer, num_pointers) \
	dee_memsetp(dst, (__UINTPTR_TYPE__)(pointer), num_pointers)
DeeSystem_DEFINE_memsetp(dee_memsetp)
#endif /* !CONFIG_HAVE_memsetp */

PRIVATE WUNUSED NONNULL((1)) DREF Bytes *DCALL
bytes_rereplace(Bytes *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct Dee_bytes_printer printer = Dee_BYTES_PRINTER_INIT;
	DeeObject *pattern, *replace, *rules = NULL;
	struct DeeRegexMatch groups[9];
	size_t maxreplace = (size_t)-1;
	char const *replace_start, *replace_end;
	struct DeeRegexExec exec;
	if (DeeArg_UnpackKw(argc, argv, kw, bytes_rereplace_kwlist,
	                    "oo|" UNPuSIZ "o:rereplace",
	                    &pattern, &replace, &maxreplace, &rules))
		goto err_printer;
	if (DeeObject_AssertTypeExact(pattern, &DeeString_Type))
		goto err_printer;
	if (DeeObject_AssertTypeExact(replace, &DeeString_Type))
		goto err_printer;
	replace_start = DeeString_AsUtf8(replace);
	if unlikely(!replace_start)
		goto err_printer;
	replace_end = replace_start + WSTR_LENGTH(replace_start);
	exec.rx_eflags = 0; /* TODO: NOTBOL/NOTEOL */
	exec.rx_code = DeeString_GetRegex(pattern, Dee_RE_COMPILE_NOUTF8, rules);
	if unlikely(!exec.rx_code)
		goto err_printer;
	exec.rx_nmatch   = COMPILER_LENOF(groups);
	exec.rx_pmatch   = groups;
	exec.rx_inbase   = DeeBytes_DATA(self);
	exec.rx_insize   = DeeBytes_SIZE(self);
	exec.rx_startoff = 0;
	exec.rx_endoff   = exec.rx_insize;
	while (exec.rx_startoff < exec.rx_endoff && maxreplace) {
		char const *replace_iter, *replace_flush;
		Dee_ssize_t match_offset;
		size_t match_size;
		memsetp(groups, (void *)(uintptr_t)(size_t)-1, 2 * 9);
		match_offset = DeeRegex_SearchNoEpsilon(&exec, (size_t)-1, &match_size);
		if unlikely(match_offset == Dee_RE_STATUS_ERROR)
			goto err_printer;
		if (match_offset == Dee_RE_STATUS_NOMATCH)
			break;

		/* Flush until start-of-match. */
		if unlikely(Dee_bytes_printer_append(&printer,
		                                     (byte_t const *)exec.rx_inbase + exec.rx_startoff,
		                                     (size_t)match_offset - exec.rx_startoff) < 0)
			goto err_printer;

		/* Parse and print the replacement bytes. */
		for (replace_iter = replace_flush = replace_start;
		     replace_iter < replace_end;) {
			struct DeeRegexMatch match;
			char ch = *replace_iter;
			switch (ch) {

			case '&':
				/* Insert full match. */
				match.rm_so = (size_t)match_offset;
				match.rm_eo = (size_t)match_offset + match_size;
do_insert_match:
				if unlikely(Dee_bytes_printer_print(&printer, replace_flush,
				                                (size_t)(replace_iter - replace_flush)) < 0)
					goto err_printer;
				if unlikely(Dee_bytes_printer_append(&printer,
				                                     (byte_t const *)exec.rx_inbase + match.rm_so,
				                                     (size_t)(match.rm_eo - match.rm_so)) < 0)
					goto err_printer;
				++replace_iter;
				if (ch != '&')
					++replace_iter;
				replace_flush = replace_iter;
				break;

			case '\\':
				ch = replace_iter[1];
				if (ch == '&' || ch == '\\') {
					/* Insert literal '&' or '\' */
					if unlikely(Dee_bytes_printer_print(&printer, replace_flush,
					                                    (size_t)(replace_iter - replace_flush)) < 0)
						goto err_printer;
					++replace_iter;
					replace_flush = replace_iter;
					++replace_iter;
				} else if (ch >= '1' && ch <= '9') {
					/* Insert matched group N.
					 * NOTE: When the group was never matched, both of its offsets will be equal
					 *       here, meaning that the code above will simply print an empty bytes! */
					match = groups[ch - '1'];
					goto do_insert_match;
				} else {
					++replace_iter;
				}
				break;

			default:
				++replace_iter;
				break;
			}
		}

		/* Flush remainder of replacement bytes. */
		if (replace_flush < replace_end) {
			if unlikely(Dee_bytes_printer_print(&printer, replace_flush,
			                                    (size_t)(replace_end - replace_flush)) < 0)
				goto err_printer;
		}

		/* Keep on scanning after the matched area. */
		exec.rx_startoff = (size_t)match_offset + match_size;
		--maxreplace;
	}

	/* Flush remainder */
#ifndef __OPTIMIZE_SIZE__
	if unlikely(exec.rx_startoff == 0) {
		Dee_bytes_printer_fini(&printer);
		return_reference_(self);
	}
#endif /* !__OPTIMIZE_SIZE__ */
	if unlikely(Dee_bytes_printer_append(&printer,
	                                     (byte_t const *)exec.rx_inbase + exec.rx_startoff,
	                                     exec.rx_endoff - exec.rx_startoff) < 0)
		goto err_printer;
	return (DREF Bytes *)Dee_bytes_printer_pack(&printer);
err_printer:
	Dee_bytes_printer_fini(&printer);
/*err:*/
	return NULL;
}

#define BYTES_BASE_REGEX_GETARGS_FMT BYTES_GENERIC_REGEX_GETARGS_FMT
#define bytes_base_regex_kwlist      bytes_generic_regex_kwlist
PRIVATE WUNUSED NONNULL((1, 5, 6)) int DCALL
bytes_base_regex_getargs(Bytes *self, size_t argc, DeeObject *const *argv,
                         DeeObject *kw, char const *__restrict fmt,
                         struct DeeRegexBaseExec *__restrict result) {
	DeeObject *rules = NULL;
	result->rx_startoff = 0;
	result->rx_endoff   = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, bytes_base_regex_kwlist, fmt,
	                    &result->rx_pattern,
	                    &result->rx_startoff,
	                    &result->rx_endoff,
	                    &rules))
		goto err;
	if (DeeObject_AssertTypeExact(result->rx_pattern, &DeeString_Type))
		goto err;
	result->rx_code = DeeString_GetRegex((DeeObject *)result->rx_pattern,
	                                     Dee_RE_COMPILE_NOUTF8, rules);
	if unlikely(!result->rx_code)
		goto err;
	result->rx_eflags = 0; /* TODO: NOTBOL/NOTEOL */
	result->rx_inbase = DeeBytes_DATA(self);
	result->rx_insize = DeeBytes_SIZE(self);
	if (result->rx_endoff > result->rx_insize)
		result->rx_endoff = result->rx_insize;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_refindall(Bytes *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct DeeRegexBaseExec exec;
	if unlikely(bytes_base_regex_getargs(self, argc, argv, kw,
	                                     BYTES_BASE_REGEX_GETARGS_FMT("refindall"),
	                                     &exec))
		goto err;
	return bytes_re_findall(self, &exec);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_regfindall(Bytes *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct DeeRegexBaseExec exec;
	if unlikely(bytes_base_regex_getargs(self, argc, argv, kw,
	                                     BYTES_BASE_REGEX_GETARGS_FMT("regfindall"),
	                                     &exec))
		goto err;
	return bytes_reg_findall(self, &exec);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_reglocateall(Bytes *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct DeeRegexBaseExec exec;
	if unlikely(bytes_base_regex_getargs(self, argc, argv, kw,
	                                     BYTES_BASE_REGEX_GETARGS_FMT("reglocateall"),
	                                     &exec))
		goto err;
	return bytes_reg_locateall(self, &exec);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_relocateall(Bytes *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct DeeRegexBaseExec exec;
	if unlikely(bytes_base_regex_getargs(self, argc, argv, kw,
	                                     BYTES_BASE_REGEX_GETARGS_FMT("relocateall"),
	                                     &exec))
		goto err;
	return bytes_re_locateall(self, &exec);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_resplit(Bytes *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct DeeRegexBaseExec exec;
	if unlikely(bytes_base_regex_getargs(self, argc, argv, kw,
	                                     BYTES_BASE_REGEX_GETARGS_FMT("resplit"),
	                                     &exec))
		goto err;
	return bytes_re_split(self, &exec);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_restartswith(Bytes *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	Dee_ssize_t result;
	struct DeeRegexExec exec;
	if unlikely(bytes_generic_regex_getargs(self, argc, argv, kw,
	                                        BYTES_GENERIC_REGEX_GETARGS_FMT("restartswith"),
	                                        &exec))
		goto err;
	result = DeeRegex_Match(&exec);
	if unlikely(result == Dee_RE_STATUS_ERROR)
		goto err;
	return_bool(result != Dee_RE_STATUS_NOMATCH);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_reendswith(Bytes *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	size_t match_size;
	Dee_ssize_t result;
	struct DeeRegexExec exec;
	if unlikely(bytes_generic_regex_getargs(self, argc, argv, kw,
	                                        BYTES_GENERIC_REGEX_GETARGS_FMT("reendswith"),
	                                        &exec))
		goto err;
	result = DeeRegex_RSearch(&exec, (size_t)-1, &match_size);
	if unlikely(result == Dee_RE_STATUS_ERROR)
		goto err;
	if unlikely(result == Dee_RE_STATUS_NOMATCH)
		return_false;
	ASSERT((size_t)result + match_size <= exec.rx_endoff || !match_size);
	return_bool((size_t)result + match_size >= exec.rx_endoff);
err:
	return NULL;
}

PRIVATE WUNUSED DREF Bytes *DCALL
bytes_relstrip(Bytes *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DREF Bytes *result;
	struct DeeRegexExec exec;
	if unlikely(bytes_generic_regex_getargs(self, argc, argv, kw,
	                                        BYTES_GENERIC_REGEX_GETARGS_FMT("relstrip"),
	                                        &exec))
		goto err;
	for (;;) {
		Dee_ssize_t status = DeeRegex_Match(&exec);
		if unlikely(status == Dee_RE_STATUS_ERROR)
			goto err;
		if (status == Dee_RE_STATUS_NOMATCH)
			break;
		if (status == 0)
			break; /* Prevent infinite loop when epsilon is matched. */
		exec.rx_startoff = (size_t)status;
		if (exec.rx_startoff >= exec.rx_endoff)
			break;
	}

	/* End-of-matching-area */
	if unlikely(exec.rx_startoff >= exec.rx_endoff) {
		result = (Bytes *)Dee_EmptyBytes;
		Dee_Incref(result);
	} else {
		result = (DREF Bytes *)DeeBytes_NewSubView(self,
		                                           (void *)((char const *)exec.rx_inbase + exec.rx_startoff),
		                                           exec.rx_endoff - exec.rx_startoff);
	}
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED DREF Bytes *DCALL
bytes_rerstrip(Bytes *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DREF Bytes *result;
	struct DeeRegexExec exec;
	if unlikely(bytes_generic_regex_getargs(self, argc, argv, kw,
	                                        BYTES_GENERIC_REGEX_GETARGS_FMT("rerstrip"),
	                                        &exec))
		goto err;
	for (;;) {
		size_t match_size;
		Dee_ssize_t status = DeeRegex_RSearch(&exec, (size_t)-1, &match_size);
		if unlikely(status == Dee_RE_STATUS_ERROR)
			goto err;
		if (status == Dee_RE_STATUS_NOMATCH)
			break;
		if (match_size == 0)
			break; /* Prevent infinite loop when epsilon is matched. */
		ASSERT((size_t)status + match_size <= exec.rx_endoff);
		if ((size_t)status + match_size < exec.rx_endoff)
			break;
		exec.rx_endoff = (size_t)status;
		if (exec.rx_startoff >= exec.rx_endoff)
			break;
	}

	/* End-of-matching-area */
	if unlikely(exec.rx_startoff >= exec.rx_endoff) {
		result = (Bytes *)Dee_EmptyBytes;
		Dee_Incref(result);
	} else {
		result = (DREF Bytes *)DeeBytes_NewSubView(self,
		                                           (void *)((char const *)exec.rx_inbase + exec.rx_startoff),
		                                           exec.rx_endoff - exec.rx_startoff);
	}
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED DREF Bytes *DCALL
bytes_restrip(Bytes *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DREF Bytes *result;
	struct DeeRegexExec exec;
	if unlikely(bytes_generic_regex_getargs(self, argc, argv, kw,
	                                        BYTES_GENERIC_REGEX_GETARGS_FMT("restrip"),
	                                        &exec))
		goto err;
	/* lstrip */
	for (;;) {
		Dee_ssize_t status = DeeRegex_Match(&exec);
		if unlikely(status == Dee_RE_STATUS_ERROR)
			goto err;
		if (status == Dee_RE_STATUS_NOMATCH)
			break;
		if (status == 0)
			break; /* Prevent infinite loop when epsilon is matched. */
		exec.rx_startoff = (size_t)status;
		if (exec.rx_startoff >= exec.rx_endoff)
			break;
	}

	/* rstrip */
	for (;;) {
		size_t match_size;
		Dee_ssize_t status = DeeRegex_RSearch(&exec, (size_t)-1, &match_size);
		if unlikely(status == Dee_RE_STATUS_ERROR)
			goto err;
		if (status == Dee_RE_STATUS_NOMATCH)
			break;
		if (match_size == 0)
			break; /* Prevent infinite loop when epsilon is matched. */
		ASSERT((size_t)status + match_size <= exec.rx_endoff);
		if ((size_t)status + match_size < exec.rx_endoff)
			break;
		exec.rx_endoff = (size_t)status;
		if (exec.rx_startoff >= exec.rx_endoff)
			break;
	}

	/* End-of-matching-area */
	if unlikely(exec.rx_startoff >= exec.rx_endoff) {
		result = (Bytes *)Dee_EmptyBytes;
		Dee_Incref(result);
	} else {
		result = (DREF Bytes *)DeeBytes_NewSubView(self,
		                                           (void *)((char const *)exec.rx_inbase + exec.rx_startoff),
		                                           exec.rx_endoff - exec.rx_startoff);
	}
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_recontains(Bytes *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	Dee_ssize_t result;
	struct DeeRegexExec exec;
	if unlikely(bytes_generic_regex_getargs(self, argc, argv, kw,
	                                        BYTES_GENERIC_REGEX_GETARGS_FMT("recontains"),
	                                        &exec))
		goto err;
	result = DeeRegex_Search(&exec, (size_t)-1, NULL);
	if unlikely(result == Dee_RE_STATUS_ERROR)
		goto err;
	return_bool(result != Dee_RE_STATUS_NOMATCH);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_recount(Bytes *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	size_t count;
	struct DeeRegexExec exec;
	if unlikely(bytes_generic_regex_getargs(self, argc, argv, kw,
	                                        BYTES_GENERIC_REGEX_GETARGS_FMT("recount"),
	                                        &exec))
		goto err;
	count = 0;
	for (;;) {
		Dee_ssize_t result;
		size_t match_size;
		result = DeeRegex_SearchNoEpsilon(&exec, (size_t)-1, &match_size);
		if unlikely(result == Dee_RE_STATUS_ERROR)
			goto err;
		if (result == Dee_RE_STATUS_NOMATCH)
			break;
		++count;
		exec.rx_startoff = (size_t)result + match_size;
		if (exec.rx_startoff >= exec.rx_endoff)
			break;
	}
	return DeeInt_NewSize(count);
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
bytes_xchitem_index(Bytes *self, size_t index, DeeObject *value) {
	byte_t val, result;
	if (DeeObject_AsByte(value, &val))
		goto err;
	if unlikely(index >= DeeBytes_SIZE(self)) {
		DeeRT_ErrIndexOutOfBounds(Dee_AsObject(self), index,
		                          DeeBytes_SIZE(self));
		goto err;
	}
	if unlikely(!DeeBytes_IsWritable(self))
		goto err_readonly;
	result = atomic_xch(&DeeBytes_DATA(self)[index], val);
	return DeeInt_NEWU(result);
err_readonly:
	err_bytes_not_writable(Dee_AsObject(self));
err:
	return NULL;
}

INTDEF struct type_method tpconst bytes_methods[];
INTDEF struct type_method_hint tpconst bytes_method_hints[];
INTERN_TPCONST struct type_method tpconst bytes_methods[] = {
	/* TODO: Pretty much everything below is can be a constant expression when "!DeeBytes_IsWritable(thisarg)" */
	TYPE_KWMETHOD("decode", &string_decode,
	              "(codec:?Dstring,errors=!Pstrict)->?X2?Dstring?O\n"
	              "#tValueError{The given @codec or @errors wasn't recognized}"
	              "#tUnicodeDecodeError{@this string could not be decoded as @codec and @errors was set to $\"strict\"}"
	              "#perrors{The way that decode-errors are handled as one of $\"strict\", $\"replace\" or $\"ignore\"}"
	              "Same as ?Adecode?Dstring, but instead use the data of @this ?. object as characters to decode"),
	TYPE_KWMETHOD("encode", &string_encode,
	              "(codec:?Dstring,errors=!Pstrict)->?X3?.?Dstring?O\n"
	              "#tValueError{The given @codec or @errors wasn't recognized}"
	              "#tUnicodeEncodeError{@this string could not be decoded as @codec and @errors was set to $\"strict\"}"
	              "#perrors{The way that decode-errors are handled as one of $\"strict\", $\"replace\" or $\"ignore\"}"
	              "Same as ?Aencode?Dstring, but instead use the data of @this ?. object as characters to decode"),
	TYPE_KWMETHOD_F("bytes", &bytes_substr,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	                "(" bytes_substr_params ")->?.\n"
	                "Same as ?#substr (here for ABI compatibility with ?Abytes?Dstring)"),
	TYPE_METHOD_F("ord", &bytes_ord,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	              "->?Dint\n"
	              "#tValueError{The length of @this ?. object is not equal to $1}"
	              "Same as ${this[0]}\n"

	              "\n"
	              "(index:?Dint)->?Dint\n"
	              "#tIntegerOverflow{The given @index is lower than $0}"
	              "#tIndexError{The given @index is greater than ${##this}}"
	              "Same as ${this[index]}"),

	/* Bytes-specific functions. */
	TYPE_METHOD_F("resized", &bytes_resized,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	              "(newsize:?Dint,filler?:?Dint)->?.\n"
	              "Return a new writable ?. object with a length of @newsize, and its "
	              /**/ "first ${(##this, newsize) < ...} bytes initialized from ${this.substr(0, newsize)}, "
	              /**/ "with the remainder then either left uninitialized, or initialized to @filler\n"
	              "Note that because a ?. object cannot be resized in-line, code using this function "
	              /**/ "must make use of the returned ?. object:\n"
	              "${"
	              "local x = \"foobar\";\n"
	              "local y = x.bytes();\n"
	              "print repr y; /* \"foobar\" */\n"
	              "y = y.resized(16, \"?\".ord());\n"
	              "print repr y; /* \"foobar??" "??" "??" "??" "??\" */"
	              "}"),
	TYPE_KWMETHOD("reverse", &bytes_reverse,
	              "(" bytes_reverse_params ")->?.\n"
	              "#tBufferError{@this ?. object is not writable}"
	              "Same as ?#reversed, but modifications are performed "
	              /**/ "in-line, before @this ?. object is re-returned"),
	TYPE_METHOD_F("makereadonly", &bytes_makereadonly,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES,
	              "->?.\n"
	              "The inverse of ?#makewritable, either re-returning @this ?. object if it "
	              /**/ "already is read-only, or construct a view for the data contained within @this "
	              /**/ "?. object, but making that view read-only"),
	TYPE_METHOD("makewritable", &bytes_makewritable,
	            "->?.\n"
	            "Either re-return @this ?. object is it already ?#iswritable, or create a "
	            /**/ "copy (s.a. ?#{op:copy}) and return it:\n"
	            "${"
	            "function makewritable() {\n"
	            "	if (this.iswritable)\n"
	            "		return this;\n"
	            "	return copy this;\n"
	            "}}"),
	TYPE_KWMETHOD_F("hex", &bytes_hex,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	                "(" bytes_hex_params ")->?Dstring\n"
	                "Returns a hex-encoded string for the bytes contained within "
	                /**/ "${this.substr(start, end)}, that is a string containing 2 characters "
	                /**/ "for each encoded byte, both of which are lower-case hexadecimal digits\n"
	                "The returned string is formatted such that ?#fromhex can be used to decode "
	                /**/ "it into another ?. object"),

	/* Bytes formatting / scanning. */
	TYPE_METHOD_F(STR_format, &bytes_format,
	              /* TODO: CONSTEXPR based on what appears in the template string
	               *       of "thisarg", and how it uses each argument */
	              METHOD_FNOREFESCAPE,
	              "(" bytes_format_params ")->?."),
	TYPE_METHOD_F("scanf", &bytes_scanf,
	              /* CONSTCALL even though "Bytes" are mutable, because this returns a proxy */
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	              "(" bytes_scanf_params ")->?S?O"),

/* String/Character traits */
#define DEFINE_BYTES_TRAIT_EX(name, func, doc)                                      \
	TYPE_METHOD_F(name, &func,                                                      \
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | \
	              METHOD_FNOREFESCAPE,                                              \
	              "->?Dbool\n"                                                      \
	              "(index:?Dint)->?Dbool\n"                                         \
	              "(start:?Dint,end:?Dint)->?Dbool\n"                               \
	              "#tIndexError{The given @index is larger than ${##this}}"         \
	              "#tIntegerOverflow{The given @index is negative or too large}"    \
	              doc)
#define DEFINE_BYTES_TRAIT(name, func, are_xxx) \
	DEFINE_BYTES_TRAIT_EX(name, func, "Returns ?t if $this, ${this[index]}, or all characters in ${this.substr(start, end)} " are_xxx)
	DEFINE_BYTES_TRAIT("iscntrl", bytes_iscntrl, "are control characters"),
	DEFINE_BYTES_TRAIT("istab", bytes_istab, "are tabulator characters (#C{U+0009}, #C{U+000B}, #C{U+000C}, ...)"),
	DEFINE_BYTES_TRAIT("iscempty", bytes_iscempty, "are tabulator (?#istab) or white-space (?#iswhite) characters (alias for ?#isspacexlf)"),
	DEFINE_BYTES_TRAIT("iswhite", bytes_iswhite, "are white-space characters (#C{U+0020}, ...)"),
	DEFINE_BYTES_TRAIT("islf", bytes_islf, "are line-feeds"),
	DEFINE_BYTES_TRAIT("isspace", bytes_isspace, "are space-characters"),
	DEFINE_BYTES_TRAIT("islower", bytes_islower, "are lower-case"),
	DEFINE_BYTES_TRAIT("isupper", bytes_isupper, "are upper-case"),
	DEFINE_BYTES_TRAIT("isalpha", bytes_isalpha, "are alphabetical"),
	DEFINE_BYTES_TRAIT("isdigit", bytes_isdigit, "are digits"),
	DEFINE_BYTES_TRAIT("ishex", bytes_ishex, "are alphabetical hex-characters (#C{U+0041-U+0046}, #C{U+0061-U+0066})"),
	DEFINE_BYTES_TRAIT("isxdigit", bytes_isxdigit, "are digit (?#isdigit) or alphabetical hex-characters (?#ishex)"),
	DEFINE_BYTES_TRAIT("isalnum", bytes_isalnum, "are alpha-numerical"),
	DEFINE_BYTES_TRAIT("ispunct", bytes_ispunct, "are punctuational characters"),
	DEFINE_BYTES_TRAIT("isgraph", bytes_isgraph, "are graphical characters"),
	DEFINE_BYTES_TRAIT("isprint", bytes_isprint, "are printable"),
	DEFINE_BYTES_TRAIT("isblank", bytes_isblank, "are blank"),
	DEFINE_BYTES_TRAIT("isnumeric", bytes_isnumeric, "qualify as digits or otherwise numeric characters"),
	DEFINE_BYTES_TRAIT("issymstrt", bytes_issymstrt, "can be used to start a symbol name"),
	DEFINE_BYTES_TRAIT("issymcont", bytes_issymcont, "can be used to continue a symbol name"),
	DEFINE_BYTES_TRAIT("isspacexlf", bytes_iscempty, "are space-characters, where linefeeds are not considered as spaces (IsSpaceeXcludingLineFeed) (alias for ?#iscempty)"),
	DEFINE_BYTES_TRAIT("isascii", bytes_isascii, "are ascii-characters, that is have an ordinal value ${<= 0x7f}"),
	TYPE_METHOD_F("istitle", &bytes_istitle,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	              "(index:?Dint)->?Dbool\n"
	              "#tIndexError{The given @index is larger than ${?#this}}"
	              "#tIntegerOverflow{The given @index is negative or too large}"
	              "Returns ?t if the character at ${this[index]} has title-casing\n"
	              "\n"

	              "->?Dbool\n"
	              "(start:?Dint,end:?Dint)->?Dbool\n"
	              "Returns ?t if $this, or the sub-bytes ${this.substr(start, end)} "
	              /**/ "follows title-casing, meaning that space is followed by title-case, "
	              /**/ "with all remaining characters not being title-case"),
	TYPE_METHOD_F("issymbol", &bytes_issymbol,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	              "(index:?Dint)->?Dbool\n"
	              "#tIndexError{The given @index is larger than ${?#this}}"
	              "#tIntegerOverflow{The given @index is negative or too large}"
	              "Returns ?t if the character at ${this[index]} can be used "
	              /**/ "to start a symbol name. Same as ${this.issymstrt(index)}\n"
	              "\n"

	              "->?Dbool\n"
	              "(start:?Dint,end:?Dint)->?Dbool\n"
	              "Returns ?t if $this, or the sub-bytes ${this.substr(start, end)} "
	              /**/ "is a valid symbol name"),
#undef DEFINE_BYTES_TRAIT

#define DEFINE_ANY_BYTES_TRAIT_EX(name, func, doc)                                    \
	TYPE_KWMETHOD_F(name, &func,                                                      \
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | \
	                METHOD_FNOREFESCAPE,                                              \
	                "(start=!0,end=!-1)->?Dbool\n" doc)
#define DEFINE_ANY_BYTES_TRAIT(name, func, is_xxx) \
	DEFINE_ANY_BYTES_TRAIT_EX(name, func, "Returns ?t if any character in ${this.substr(start, end)} " is_xxx)
	DEFINE_ANY_BYTES_TRAIT("isanycntrl", bytes_isanycntrl, "is a control character"),
	DEFINE_ANY_BYTES_TRAIT("isanytab", bytes_isanytab, "is a tabulator character (#C{U+0009}, #C{U+000B}, #C{U+000C}, ...)"),
	DEFINE_ANY_BYTES_TRAIT("isanycempty", bytes_isanycempty, "is a tabulator (?#istab) or white-space (?#iswhite) character (alias for ?#isanyspacexlf)"),
	DEFINE_ANY_BYTES_TRAIT("isanywhite", bytes_isanywhite, "is a white-space character (#C{U+0020}, ...)"),
	DEFINE_ANY_BYTES_TRAIT("isanylf", bytes_isanylf, "is a line-feed"),
	DEFINE_ANY_BYTES_TRAIT("isanyspace", bytes_isanyspace, "is a space character"),
	DEFINE_ANY_BYTES_TRAIT("isanylower", bytes_isanylower, "is lower-case"),
	DEFINE_ANY_BYTES_TRAIT("isanyupper", bytes_isanyupper, "is upper-case"),
	DEFINE_ANY_BYTES_TRAIT("isanyalpha", bytes_isanyalpha, "is alphabetical"),
	DEFINE_ANY_BYTES_TRAIT("isanydigit", bytes_isanydigit, "is a digit"),
	DEFINE_ANY_BYTES_TRAIT("isanyhex", bytes_isanyhex, "is an alphabetical hex-character (#C{U+0041-U+0046}, #C{U+0061-U+0066})"),
	DEFINE_ANY_BYTES_TRAIT("isanyxdigit", bytes_isanyxdigit, "is a digit (?#isdigit) or an alphabetical hex-character (?#ishex)"),
	DEFINE_ANY_BYTES_TRAIT("isanyalnum", bytes_isanyalnum, "is alpha-numerical"),
	DEFINE_ANY_BYTES_TRAIT("isanypunct", bytes_isanypunct, "is a punctuational character"),
	DEFINE_ANY_BYTES_TRAIT("isanygraph", bytes_isanygraph, "is a graphical character"),
	DEFINE_ANY_BYTES_TRAIT("isanyprint", bytes_isanyprint, "is printable"),
	DEFINE_ANY_BYTES_TRAIT("isanyblank", bytes_isanyblank, "is blank"),
	DEFINE_ANY_BYTES_TRAIT("isanytitle", bytes_isanytitle, "has title-casing"),
	DEFINE_ANY_BYTES_TRAIT("isanynumeric", bytes_isanynumeric, "qualifies as digit or some other numeric character"),
	DEFINE_ANY_BYTES_TRAIT("isanysymstrt", bytes_isanysymstrt, "can be used to start a symbol name"),
	DEFINE_ANY_BYTES_TRAIT("isanysymcont", bytes_isanysymcont, "can be used to continue a symbol name"),
	DEFINE_ANY_BYTES_TRAIT("isanyspacexlf", bytes_isanycempty, "is a space character, where linefeeds are not considered as spaces (IsSpaceeXcludingLineFeed) (alias for ?#isanycempty)"),
	TYPE_KWMETHOD_F("isanyascii", &bytes_isanyascii,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	                "(start=!0,end=!-1)->?Dbool\n"
	                "Returns ?t if any character in ${this.substr(start, end)} is "
	                /**/ "an ascii character, that is has an ordinal value ${<= 0x7f}"),
#undef DEFINE_ANY_BYTES_TRAIT
#undef DEFINE_ANY_BYTES_TRAIT_EX

	TYPE_METHOD_F("asnumeric", &bytes_asdigit,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	              "->?Dint\n"
	              "#tValueError{The string is longer than a single character}"
	              "(index:?Dint)->?Dint\n"
	              "#tValueError{The character at @index isn't numeric}"
	              "(index:?Dint,defl:?Dint)->?Dint\n"
	              "(index:?Dint,defl)->\n"
	              "#tIntegerOverflow{The given @index is negative or too large}"
	              "#tIndexError{The given @index is out of bounds}"
	              "Alias for ?#asdigit"),
	TYPE_METHOD_F("asdigit", &bytes_asdigit,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	              "->?Dint\n"
	              "#tValueError{The string is longer than a single character}"
	              "(index:?Dint)->?Dint\n"
	              "#tValueError{The character at @index isn't numeric}"
	              "(index:?Dint,defl:?Dint)->?Dint\n"
	              "(index:?Dint,defl)->\n"
	              "#tIntegerOverflow{The given @index is negative or too large}"
	              "#tIndexError{The given @index is out of bounds}"
	              "Returns the digit value of the byte at the specific index"),
	TYPE_METHOD_F("asxdigit", &bytes_asxdigit,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	              "->?Dint\n"
	              "#tValueError{The string is longer than a single character}"
	              "(index:?Dint)->?Dint\n"
	              "#tValueError{The character at @index isn't numeric}"
	              "(index:?Dint,defl:?Dint)->?Dint\n"
	              "(index:?Dint,defl)->\n"
	              "#tIntegerOverflow{The given @index is negative or too large}"
	              "#tIndexError{The given @index is out of bounds}"
	              "Same as ?#asdigit, but also accepts #C{a-f} and #C{A-F}"),

	/* Bytes conversion functions */
	TYPE_KWMETHOD_F("lower", &bytes_lower,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	                "(" bytes_lower_params ")->?.\n"
	                "Returns a writable copy of @this ?. object converted to lower-case (when interpreted as ASCII)"),
	TYPE_KWMETHOD_F("upper", &bytes_upper,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	                "(" bytes_upper_params ")->?.\n"
	                "Returns a writable copy of @this ?. object converted to upper-case (when interpreted as ASCII)"),
	TYPE_KWMETHOD_F("title", &bytes_title,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	                "(" bytes_title_params ")->?.\n"
	                "Returns a writable copy of @this ?. object converted to title-casing (when interpreted as ASCII)"),
	TYPE_KWMETHOD_F("capitalize", &bytes_capitalize,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	                "(" bytes_capitalize_params ")->?.\n"
	                "Returns a writable copy of @this ?. object with each word capitalized (when interpreted as ASCII)"),
	TYPE_KWMETHOD_F("swapcase", &bytes_swapcase,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	                "(" bytes_swapcase_params ")->?.\n"
	                "Returns a writable copy of @this ?. object with the casing of each "
	                /**/ "character that has two different casings swapped (when interpreted as ASCII)"),
	TYPE_KWMETHOD_F("casefold", &bytes_lower,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	                "(" bytes_lower_params ")->?.\n"
	                "Alias for ?#{lower}. This function exists to match ?Acasefold?Dstring in "
	                /**/ "order to improve binary compatibility between ?. and ?Dstring objects"),

	/* Inplace variants of bytes conversion functions */
	TYPE_KWMETHOD("tolower", &bytes_tolower,
	              "(" bytes_tolower_params ")->?.\n"
	              "#tBufferError{@this ?. object is not writable}"
	              "Same as ?#lower, but character modifications are performed in-place, and @this ?. object is re-returned"),
	TYPE_KWMETHOD("toupper", &bytes_toupper,
	              "(" bytes_toupper_params ")->?.\n"
	              "#tBufferError{@this ?. object is not writable}"
	              "Same as ?#upper, but character modifications are performed in-place, and @this ?. object is re-returned"),
	TYPE_KWMETHOD("totitle", &bytes_totitle,
	              "(" bytes_totitle_params ")->?.\n"
	              "#tBufferError{@this ?. object is not writable}"
	              "Same as ?#title, but character modifications are performed in-place, and @this ?. object is re-returned"),
	TYPE_KWMETHOD("tocapitalize", &bytes_tocapitalize,
	              "(" bytes_tocapitalize_params ")->?.\n"
	              "#tBufferError{@this ?. object is not writable}"
	              "Same as ?#capitalize, but character modifications are performed in-place, and @this ?. object is re-returned"),
	TYPE_KWMETHOD("toswapcase", &bytes_toswapcase,
	              "(" bytes_toswapcase_params ")->?.\n"
	              "#tBufferError{@this ?. object is not writable}"
	              "Same as ?#swapcase, but character modifications are performed in-place, and @this ?. object is re-returned"),
	TYPE_KWMETHOD("tocasefold", &bytes_tolower,
	              "(" bytes_tolower_params ")->?.\n"
	              "#tBufferError{@this ?. object is not writable}"
	              "Alias for ?#tolower, here to coincide with ?#casefold existing as an alias for ?#lower"),

	/* Case-sensitive query functions */
	TYPE_KWMETHOD_F(STR_replace, &bytes_replace,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES,
	                "(" bytes_replace_params ")->?.\n"
	                "#tValueError{The given @find or @replace is a string containing characters ${> 0xff}}"
	                "#tIntegerOverflow{The given @find or @replace is an integer lower than $0, or greater than $0xff}"
	                "Find up to @max occurrences of @find and replace each with @replace, then return the resulting data as a writable ?. object"),
	TYPE_KWMETHOD("toreplace", &bytes_toreplace,
	              "(" bytes_toreplace_params ")->?.\n"
	              "#tValueError{The given @find or @replace is a string containing characters ${> 0xff}}"
	              "#tIntegerOverflow{The given @find or @replace is an integer lower than $0, or greater than $0xff}"
	              "#tValueError{The number of bytes specified by @find and @replace are not identical}"
	              "#tBufferError{@this ?. object is not writable}"
	              "Same as ?#replace, but the ?. object is modified in-place, and @this is re-returned"),
	TYPE_KWMETHOD_F("find", &bytes_find,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	                "(" bytes_find_params ")->?Dint\n"
	                "#tValueError{The given @needle is a string containing characters ${> 0xff}}"
	                "#tIntegerOverflow{The given @needle is an integer lower than $0, or greater than $0xff}"
	                "Find the first instance of @needle that exists within ${this.substr(start, end)}, "
	                /**/ "and return its starting index, or ${-1} if no such position exists"),
	TYPE_KWMETHOD_F("rfind", &bytes_rfind,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	                "(" bytes_rfind_params ")->?Dint\n"
	                "#tValueError{The given @needle is a string containing characters ${> 0xff}}"
	                "#tIntegerOverflow{The given @needle is an integer lower than $0, or greater than $0xff}"
	                "Find the first instance of @needle that exists within ${this.substr(start, end)}, "
	                /**/ "and return its starting index, or ${-1} if no such position exists"),
	TYPE_KWMETHOD_F(STR_index, &bytes_index,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	                "(" bytes_index_params ")->?Dint\n"
	                "#tIndexError{No instance of @needle can be found within ${this.substr(start, end)}}"
	                "Find the first instance of @needle that exists within ${this.substr(start, end)}, "
	                /**/ "and return its starting index"),
	TYPE_KWMETHOD_F("rindex", &bytes_rindex,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	                "(" bytes_rindex_params ")->?Dint\n"
	                "#tIndexError{No instance of @needle can be found within ${this.substr(start, end)}}"
	                "Find the last instance of @needle that exists within ${this.substr(start, end)}, "
	                /**/ "and return its starting index"),
	TYPE_KWMETHOD_F("findany", &bytes_findany,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	                "(" bytes_findany_params ")->?X2?Dint?N\n"
	                "#tValueError{One of the given @needles is a string containing characters ${> 0xff}}"
	                "#tIntegerOverflow{The given @needle is an integer lower than $0, or greater than $0xff}"
	                "Find all instances of any of the given @needles within ${this.substr(start, end)}, "
	                /**/ "and return the lowest starting index, or ?N if no such position exists"),
	TYPE_KWMETHOD_F("rfindany", &bytes_rfindany,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	                "(" bytes_rfindany_params ")->?X2?Dint?N\n"
	                "#tValueError{One of the given @needles is a string containing characters ${> 0xff}}"
	                "#tIntegerOverflow{The given @needle is an integer lower than $0, or greater than $0xff}"
	                "Find all instances of any of the given @needles within ${this.substr(start, end)}, "
	                /**/ "and return the greatest starting index, or ?N if no such position exists"),
	TYPE_KWMETHOD_F("indexany", &bytes_indexany,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	                "(" bytes_indexany_params ")->?X2?Dint?N\n"
	                "#tIndexError{No instance of @needle can be found within ${this.substr(start, end)}}"
	                "Find all instances of any of the given @needles within ${this.substr(start, end)}, "
	                /**/ "and return the lowest starting index"),
	TYPE_KWMETHOD_F("rindexany", &bytes_rindexany,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	                "(" bytes_rindexany_params ")->?X2?Dint?N\n"
	                "#tIndexError{No instance of @needle can be found within ${this.substr(start, end)}}"
	                "Find all instances of any of the given @needles within ${this.substr(start, end)}, "
	                /**/ "and return the greatest starting index"),
	TYPE_KWMETHOD_F("findall", &bytes_findall,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES,
	                "(" bytes_findall_params ")->?S?Dint\n"
	                "Find all instances of @needle within ${this.substr(start, end)}, "
	                /**/ "and return their starting indices as a sequence"),
	TYPE_KWMETHOD_F("count", &bytes_count,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	                "(" bytes_count_params ")->?Dint\n"
	                "Count the number of instances of @needle that exists within ${this.substr(start, end)}, "
	                /**/ "and return now many were found"),
	TYPE_KWMETHOD_F("contains", &bytes_contains_f,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	                "(" bytes_contains_params ")->?Dbool\n"
	                "Check if @needle can be found within ${this.substr(start, end)}, and return a boolean indicative of that"),
	TYPE_KWMETHOD_F("substr", &bytes_substr,
	                /* Because modifications made to the returned proxy get mirrored in "thisarg",
	                 * said proxy can already be constructed at compile-time, so this function is
	                 * a constcall, even if the Bytes object is writable. */
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	                "(" bytes_substr_params ")->?.\n"
	                "Similar to ${this[start:end]}, and semantically equialent to ?Asubstr?Dstring\n"
	                "This function can be used to view a sub-set of bytes from @this ?. object\n"
	                "Modifications then made to the returned ?. object will affect the same memory "
	                /**/ "already described by @this ?. object"),
	TYPE_METHOD_F("strip", &bytes_strip,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES,
	              "(mask?:?X3?.?Dstring?Dint)->?.\n"
	              "Strip all leading and trailing whitespace-characters, or "
	              /**/ "characters apart of @mask, and return a sub-view of @this ?. object"),
	TYPE_KWMETHOD_F("lstrip", &bytes_lstrip,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES,
	                "(mask?:?X3?.?Dstring?Dint,max=!-1)->?.\n"
	                "Strip all leading whitespace-characters, or characters "
	                /**/ "apart of @mask, and return a sub-view of @this ?. object"),
	TYPE_KWMETHOD_F("rstrip", &bytes_rstrip,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES,
	                "(mask?:?X3?.?Dstring?Dint,max=!-1)->?.\n"
	                "Strip all trailing whitespace-characters, or characters "
	                /**/ "apart of @mask, and return a sub-view of @this ?. object"),
	TYPE_METHOD_F("sstrip", &bytes_sstrip,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES,
	              "(needle:?X3?.?Dstring?Dint)->?.\n"
	              "Strip all leading and trailing instances of @needle from @this string\n"
	              "${"
	              /**/ "local result = this;\n"
	              /**/ "while (result.startswith(needle))\n"
	              /**/ "	result = result[##needle:];\n"
	              /**/ "while (result.endswith(needle))\n"
	              /**/ "	result = result[:##result - ##needle];"
	              "}"),
	TYPE_KWMETHOD_F("lsstrip", &bytes_lsstrip,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES,
	                "(needle:?X3?.?Dstring?Dint,max=!-1)->?.\n"
	                "Strip all leading instances of @needle from @this string\n"
	                "${"
	                /**/ "local result = this;\n"
	                /**/ "while (result.startswith(needle))\n"
	                /**/ "	result = result[##needle:];"
	                "}"),
	TYPE_KWMETHOD_F("rsstrip", &bytes_rsstrip,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES,
	                "(needle:?X3?.?Dstring?Dint,max=!-1)->?.\n"
	                "Strip all trailing instances of @needle from @this string\n"
	                "${"
	                /**/ "local result = this;\n"
	                /**/ "while (result.endswith(needle))\n"
	                /**/ "	result = result[:##result - ##needle];"
	                "}"),
	TYPE_METHOD_F("striplines", &bytes_striplines,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES,
	              "(mask?:?X3?.?Dstring?Dint)->?.\n"
	              "Strip all whitspace, or @mask characters at the start, end, and before/after linefeeds\n"
	              "Note that for this purpose, linefeed characters don't count as whitespace\n"
	              "aka: strip all leading and trailing whitespace\n"
	              "Similar to ${\"\".bytes().join(this.splitlines(true).each.strip())}"),
	TYPE_METHOD_F("lstriplines", &bytes_lstriplines,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES,
	              "(mask?:?X3?.?Dstring?Dint)->?.\n"
	              "Strip all whitspace, or @mask characters at the start, and after linefeeds\n"
	              "Note that for this purpose, linefeed characters don't count as whitespace\n"
	              "aka: strip all leading whitespace\n"
	              "Similar to ${\"\".bytes().join(this.splitlines(true).each.lstrip())}"),
	TYPE_METHOD_F("rstriplines", &bytes_rstriplines,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES,
	              "(mask?:?X3?.?Dstring?Dint)->?.\n"
	              "Strip all whitspace, or @mask characters at the end, and before linefeeds\n"
	              "Note that for this purpose, linefeed characters don't count as whitespace\n"
	              "aka: strip all trailing whitespace\n"
	              "Similar to ${\"\".bytes().join(this.splitlines(true).each.rstrip())}"),
	TYPE_METHOD_F("sstriplines", &bytes_sstriplines,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES,
	              "(needle:?X3?.?Dstring?Dint)->?.\n"
	              "Same as ?#striplines, but sequence for complete sequences of #needle, rather "
	              /**/ "than bytes apart of its $mask character."),
	TYPE_METHOD_F("lsstriplines", &bytes_lsstriplines,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES,
	              "(needle:?X3?.?Dstring?Dint)->?.\n"
	              "Same as ?#lstriplines, but sequence for complete sequences of #needle, rather "
	              /**/ "than bytes apart of its $mask character."),
	TYPE_METHOD_F("rsstriplines", &bytes_rsstriplines,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES,
	              "(needle:?X3?.?Dstring?Dint)->?.\n"
	              "Same as ?#rstriplines, but sequence for complete sequences of #needle, rather "
	              /**/ "than bytes apart of its $mask character."),
	TYPE_KWMETHOD_F("startswith", &bytes_startswith,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	                "(" bytes_startswith_params ")->?Dbool\n"
	                "Return ?t if the sub-string ${this.substr(start, end)} starts with @other"),
	TYPE_KWMETHOD_F("endswith", &bytes_endswith,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	                "(" bytes_endswith_params ")->?Dbool\n"
	                "Return ?t if the sub-string ${this.substr(start, end)} ends with @other"),
	TYPE_KWMETHOD_F("partition", &bytes_partition,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES,
	                "(" bytes_partition_params ")->?T3?.?.?.\n"
	                "Search for the first instance of @needle within ${this.substr(start, end)} and "
	                /**/ "return a 3-element sequence of byte objects ${(this[:pos], needle, this[pos + ##needle:])}.\n"
	                "If @needle could not be found, ${(this, \"\".bytes(), \"\".bytes())} is returned"),
	TYPE_KWMETHOD_F("rpartition", &bytes_rpartition,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES,
	                "(" bytes_rpartition_params ")->?T3?.?.?.\n"
	                "Search for the last instance of @needle within ${this.substr(start, end)} and "
	                "return a 3-element sequence of strings ${(this[:pos], needle, this[pos + ##needle:])}.\n"
	                "If @needle could not be found, ${(\"\".bytes(), \"\".bytes(), this)} is returned"),
	TYPE_METHOD_F("compare", &bytes_compare,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	              "" get_bcompare_decl("other", "->?Dint") "\n"
	              "Compare the sub-string ${left = this.substr(my_start, my_end)} with ${right = other.substr(other_start, other_end)}, "
	              /**/ "returning ${< 0} if ${left < right}, ${> 0} if ${left > right}, or ${== 0} if they are equal"),
	TYPE_METHOD_F("vercompare", &bytes_vercompare,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	              "" get_bcompare_decl("other", "->?Dint") "\n"
	              "Performs a version-string comparison. This is similar to ?#compare, but rather than "
	              /**/ "performing a strict lexicographical comparison, the numbers found in the strings "
	              /**/ "being compared are compared as a whole, solving the common problem seen in applications "
	              /**/ "such as file navigators showing a file order of $\"foo1.txt\", $\"foo10.txt\", "
	              /**/ "$\"foo11.txt\", $\"foo2.txt\", etc...\n"
	              "This function is a portable implementation of the GNU function "
	              /**/ "#A{strverscmp|https://linux.die.net/man/3/strverscmp}, "
	              /**/ "for which you may follow the link for further details"),
	TYPE_METHOD_F("wildcompare", &bytes_wildcompare,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	              "" get_bcompare_decl("pattern", "->?Dint") "\n"
	              "Perform a wild-character-enabled comparising of the sub-string ${left = this.substr(my_start,my_end)} "
	              /**/ "with ${right = pattern.substr(pattern_start, pattern_end)}, returning ${< 0} if ${left < right}, ${> 0} "
	              /**/ "if ${left > right}, or ${== 0} if they are equal\n"
	              "Wild-compare characters are only parsed from @pattern, allowing $\"?\" to "
	              /**/ "be matched with any single character from @this, and $\"*\" to be matched to "
	              /**/ "any number of characters"),
	TYPE_METHOD_F("fuzzycompare", &bytes_fuzzycompare,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	              "" get_bcompare_decl("other", "->?Dint") "\n"
	              "Perform a fuzzy string comparison between ${this.substr(my_start,my_end)} and ${other.substr(other_start,other_end)}\n"
	              "The return value is a similarty-factor that can be used to score how close the two strings look alike.\n"
	              "How exactly the scoring is done is implementation-specific, however a score of $0 is reserved for two "
	              /**/ "strings that are perfectly identical, any two differing strings always have a score ${> 0}, and the closer "
	              /**/ "the score is to $0, the more alike they are\n"
	              "The intended use of this function is for auto-completion, as well as warning "
	              /**/ "messages and recommendations in the sense of I-dont-know-foo-but-did-you-mean-bar\n"
	              "Note that there is another version ?#casefuzzycompare that also ignores casing"),
	TYPE_METHOD_F("wmatch", &bytes_wmatch,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	              "" get_bcompare_decl("pattern", "->?Dbool") "\n"
	              "Same as ?#wildcompare, returning ?t where ?#wildcompare would return $0, and ?f in all pattern cases"),

	/* Case-insensitive query functions */
	TYPE_KWMETHOD_F("casereplace", &bytes_casereplace,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	                "(" bytes_casereplace_params ")->?.\n"
	                "#tValueError{The given @find or @replace is a string containing characters ${> 0xff}}"
	                "#tIntegerOverflow{The given @find or @replace is an integer lower than $0, or greater than $0xff}"
	                "Same as ?#replace, however ascii-casing is ignored during character comparisons"),
	TYPE_KWMETHOD("tocasereplace", &bytes_tocasereplace,
	              "(" bytes_tocasereplace_params ")->?.\n"
	              "#tValueError{The given @find or @replace is a string containing characters ${> 0xff}}"
	              "#tIntegerOverflow{The given @find or @replace is an integer lower than $0, or greater than $0xff}"
	              "#tValueError{The number of bytes specified by @find and @replace are not identical}"
	              "#tBufferError{@this ?. object is not writable}"
	              "Same as ?#toreplace, however ascii-casing is ignored during character comparisons"),
	TYPE_KWMETHOD_F("casefind", &bytes_casefind,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	                "(" bytes_casefind_params ")->?X2?T2?Dint?Dint?N\n"
	                "#tValueError{The given @needle is a string containing characters ${> 0xff}}"
	                "#tIntegerOverflow{The given @needle is an integer lower than $0, or greater than $0xff}"
	                "Same as ?#find, however ascii-casing is ignored during character comparisons\n"
	                "Upon success, the second returned integer is equal to ${return[0] + ##needle}\n"
	                "If no match if found, ?N is returned"),
	TYPE_KWMETHOD_F("caserfind", &bytes_caserfind,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	                "(" bytes_caserfind_params ")->?X2?T2?Dint?Dint?N\n"
	                "#tValueError{The given @needle is a string containing characters ${> 0xff}}"
	                "#tIntegerOverflow{The given @needle is an integer lower than $0, or greater than $0xff}"
	                "Same as ?#rfind, however ascii-casing is ignored during character comparisons\n"
	                "Upon success, the second returned integer is equal to ${return[0] + ##needle}\n"
	                "If no match if found, ?N is returned"),
	TYPE_KWMETHOD_F("caseindex", &bytes_caseindex,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	                "(" bytes_caseindex_params ")->?T2?Dint?Dint\n"
	                "#tIndexError{No instance of @needle can be found within ${this.substr(start, end)}}"
	                "Same as ?#index, however ascii-casing is ignored during character comparisons\n"
	                "Upon success, the second returned integer is equal to ${return[0] + ##needle}"),
	TYPE_KWMETHOD_F("caserindex", &bytes_caserindex,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	                "(" bytes_caserindex_params ")->?T2?Dint?Dint\n"
	                "#tIndexError{No instance of @needle can be found within ${this.substr(start, end)}}"
	                "Same as ?#rindex, however ascii-casing is ignored during character comparisons\n"
	                "Upon success, the second returned integer is equal to ${return[0] + ##needle}"),
	TYPE_KWMETHOD_F("casefindany", &bytes_casefindany,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	                "(" bytes_casefindany_params ")->?X2?T2?Dint?Dint?N\n"
	                "#tValueError{The given @needle is a string containing characters ${> 0xff}}"
	                "#tIntegerOverflow{The given @needle is an integer lower than $0, or greater than $0xff}"
	                "Same as ?#findany, however ascii-casing is ignored during character comparisons\n"
	                "Upon success, the second returned integer is equal to ${return[0] + ##needle}\n"
	                "If no match if found, ?N is returned"),
	TYPE_KWMETHOD_F("caserfindany", &bytes_caserfindany,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	                "(" bytes_caserfindany_params ")->?X2?T2?Dint?Dint?N\n"
	                "#tValueError{The given @needle is a string containing characters ${> 0xff}}"
	                "#tIntegerOverflow{The given @needle is an integer lower than $0, or greater than $0xff}"
	                "Same as ?#rfindany, however ascii-casing is ignored during character comparisons\n"
	                "Upon success, the second returned integer is equal to ${return[0] + ##needle}\n"
	                "If no match if found, ?N is returned"),
	TYPE_KWMETHOD_F("caseindexany", &bytes_caseindexany,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	                "(" bytes_caseindexany_params ")->?T2?Dint?Dint\n"
	                "#tIndexError{No instance of @needle can be found within ${this.substr(start, end)}}"
	                "Same as ?#indexany, however ascii-casing is ignored during character comparisons\n"
	                "Upon success, the second returned integer is equal to ${return[0] + ##needle}"),
	TYPE_KWMETHOD_F("caserindexany", &bytes_caserindexany,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	                "(" bytes_caserindexany_params ")->?T2?Dint?Dint\n"
	                "#tIndexError{No instance of @needle can be found within ${this.substr(start, end)}}"
	                "Same as ?#rindexany, however ascii-casing is ignored during character comparisons\n"
	                "Upon success, the second returned integer is equal to ${return[0] + ##needle}"),
	TYPE_KWMETHOD_F("casefindall", &bytes_casefindall,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	                "(" bytes_casefindall_params ")->?S?T2?Dint?Dint\n"
	                "Same as ?#findall, however ascii-casing is ignored during character comparisons\n"
	                "Upon success, the second returned integer is equal to ${return[0] + ##needle}"),
	TYPE_KWMETHOD_F("casecount", &bytes_casecount,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	                "(" bytes_casecount_params ")->?Dint\n"
	                "Same as ?#count, however ascii-casing is ignored during character comparisons"),
	TYPE_KWMETHOD_F("casecontains", &bytes_casecontains_f,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	                "(" bytes_casecontains_params ")->?Dbool\n"
	                "Same as ?#contains, however ascii-casing is ignored during character comparisons"),
	TYPE_METHOD_F("casestrip", &bytes_casestrip,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES,
	              "(mask?:?X3?.?Dstring?Dint)->?.\n"
	              "Same as ?#strip, however ascii-casing is ignored during character comparisons"),
	TYPE_KWMETHOD_F("caselstrip", &bytes_caselstrip,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES,
	                "(mask?:?X3?.?Dstring?Dint,max=!-1)->?.\n"
	                "Same as ?#lstrip, however ascii-casing is ignored during character comparisons"),
	TYPE_KWMETHOD_F("caserstrip", &bytes_caserstrip,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES,
	                "(mask?:?X3?.?Dstring?Dint,max=!-1)->?.\n"
	                "Same as ?#rstrip, however ascii-casing is ignored during character comparisons"),
	TYPE_METHOD_F("casesstrip", &bytes_casesstrip,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES,
	              "(needle:?X3?.?Dstring?Dint)->?.\n"
	              "Same as ?#sstrip, however ascii-casing is ignored during character comparisons"),
	TYPE_KWMETHOD_F("caselsstrip", &bytes_caselsstrip,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES,
	                "(needle:?X3?.?Dstring?Dint,max=!-1)->?.\n"
	                "Same as ?#lsstrip, however ascii-casing is ignored during character comparisons"),
	TYPE_KWMETHOD_F("casersstrip", &bytes_casersstrip,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES,
	                "(needle:?X3?.?Dstring?Dint,max=!-1)->?.\n"
	                "Same as ?#rsstrip, however ascii-casing is ignored during character comparisons"),
	TYPE_METHOD_F("casestriplines", &bytes_casestriplines,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES,
	              "(mask?:?X3?.?Dstring?Dint)->?.\n"
	              "Same as ?#striplines, however ascii-casing is ignored during character comparisons"),
	TYPE_METHOD_F("caselstriplines", &bytes_caselstriplines,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES,
	              "(mask?:?X3?.?Dstring?Dint)->?.\n"
	              "Same as ?#lstriplines, however ascii-casing is ignored during character comparisons"),
	TYPE_METHOD_F("caserstriplines", &bytes_caserstriplines,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES,
	              "(mask?:?X3?.?Dstring?Dint)->?.\n"
	              "Same as ?#rstriplines, however ascii-casing is ignored during character comparisons"),
	TYPE_METHOD_F("casesstriplines", &bytes_casesstriplines,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES,
	              "(needle:?X3?.?Dstring?Dint)->?.\n"
	              "Same as ?#sstriplines, however ascii-casing is ignored during character comparisons"),
	TYPE_METHOD_F("caselsstriplines", &bytes_caselsstriplines,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES,
	              "(needle:?X3?.?Dstring?Dint)->?.\n"
	              "Same as ?#lsstriplines, however ascii-casing is ignored during character comparisons"),
	TYPE_METHOD_F("casersstriplines", &bytes_casersstriplines,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES,
	              "(needle:?X3?.?Dstring?Dint)->?.\n"
	              "Same as ?#rsstriplines, however ascii-casing is ignored during character comparisons"),
	TYPE_KWMETHOD_F("casestartswith", &bytes_casestartswith,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	                "(" bytes_casestartswith_params ")->?Dbool\n"
	                "Same as ?#startswith, however ascii-casing is ignored during character comparisons"),
	TYPE_KWMETHOD_F("caseendswith", &bytes_caseendswith,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	                "(" bytes_caseendswith_params ")->?Dbool\n"
	                "Same as ?#endswith, however ascii-casing is ignored during character comparisons"),
	TYPE_KWMETHOD_F("casepartition", &bytes_casepartition,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES,
	                "(" bytes_casepartition_params ")->?T3?.?.?.\n"
	                "Same as ?#partition, however ascii-casing is ignored during character comparisons"),
	TYPE_KWMETHOD_F("caserpartition", &bytes_caserpartition,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES,
	                "(" bytes_caserpartition_params ")->?T3?.?.?.\n"
	                "Same as ?#rpartition, however ascii-casing is ignored during character comparisons"),
	TYPE_METHOD_F("casecompare", &bytes_casecompare,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	              "" get_bcompare_decl("other", "->?Dint") "\n"
	              "Same as ?#compare, however ascii-casing is ignored during character comparisons"),
	TYPE_METHOD_F("casevercompare", &bytes_casevercompare,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	              "" get_bcompare_decl("other", "->?Dint") "\n"
	              "Same as ?#vercompare, however ascii-casing is ignored during character comparisons"),
	TYPE_METHOD_F("casewildcompare", &bytes_casewildcompare,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	              "" get_bcompare_decl("pattern", "->?Dint") "\n"
	              "Same as ?#wildcompare, however ascii-casing is ignored during character comparisons"),
	TYPE_METHOD_F("casefuzzycompare", &bytes_casefuzzycompare,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	              "" get_bcompare_decl("other", "->?Dint") "\n"
	              "Same as ?#fuzzycompare, however ascii-casing is ignored during character comparisons"),
	TYPE_METHOD_F("casewmatch", &bytes_casewmatch,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	              "" get_bcompare_decl("pattern", "->?Dbool") "\n"
	              "Same as ?#casewmatch, however ascii-casing is ignored during character comparisons"),

	/* Bytes alignment functions. */
	TYPE_METHOD_F("center", &bytes_center,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES,
	              "(" bytes_center_params ")->?.\n"
	              "Use a writable copy of @this ?. object as result, then evenly "
	              /**/ "insert @filler at the front and back to pad its length to @width bytes"),
	TYPE_METHOD_F("ljust", &bytes_ljust,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES,
	              "(" bytes_ljust_params ")->?.\n"
	              "Use a writable copy of @this ?. object as result, then "
	              /**/ "insert @filler at the back to pad its length to @width bytes"),
	TYPE_METHOD_F("rjust", &bytes_rjust,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES,
	              "(" bytes_rjust_params ")->?.\n"
	              "Use a writable copy of @this ?. object as result, then "
	              /**/ "insert @filler at the front to pad its length to @width bytes"),
	TYPE_METHOD_F("zfill", &bytes_zfill,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES,
	              "(" bytes_zfill_params ")->?.\n"
	              "Skip leading ${\'+\'} and ${\'-\'} ascii-characters, then insert @filler "
	              /**/ "to pad the resulting string to a length of @width bytes"),
	TYPE_KWMETHOD_F("reversed", &bytes_reversed,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	                "(" bytes_reversed_params ")->?.\n"
	                "Return a copy of the sub-string ${this.substr(start, end)} with its byte order reversed"),
	TYPE_METHOD_F("expandtabs", &bytes_expandtabs,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES,
	              "(" bytes_expandtabs_params ")->?.\n"
	              "Expand tab characters with whitespace offset from the "
	              /**/ "start of their respective line at multiples of @tabwidth\n"
	              "Note that in the event of no tabs being found, @this ?. object may be re-returned"),
	TYPE_METHOD_F("unifylines", &bytes_unifylines,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES,
	              "(" bytes_unifylines_params ")->?.\n"
	              "Unify all ascii-linefeed character sequences ($\"\\n\", $\"\\r\" and $\"\\r\\n\") "
	              "found in @this ?. object to make exclusive use of @replacement\n"
	              "Note that in the event of no line-feeds differing from @replacement being found, "
	              /**/ "@this ?. object may be re-returned"),

	/* Bytes splitter functions. */
	TYPE_METHOD_F("join", &bytes_join,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGSELEM_CONSTSTR_ROBYTES | METHOD_FNOREFESCAPE,
	              "(" bytes_join_params ")->?.\n"
	              "Iterate @seq and convert all items into string, inserting @this "
	              /**/ "?. object before each string's ?Abytes?Dstring representation element, "
	              /**/ "starting only with the second. ?. objects contained in @seq are not "
	              /**/ "converted to and from strings, but inserted directly"),
	TYPE_METHOD_F("split", &bytes_split,
	              /* CONSTCALL even though "Bytes" are mutable, because this returns a proxy */
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	              "(" bytes_split_params ")->?S?.\n"
	              "Split @this ?. object at each instance of @sep, "
	              /**/ "returning a sequence of the resulting parts\n"
	              "The returned bytes objects are views of @this byte object, meaning they "
	              /**/ "have the same ?#iswritable characteristics as @this, and refer to the same "
	              /**/ "memory"),
	TYPE_METHOD_F("casesplit", &bytes_casesplit,
	              /* CONSTCALL even though "Bytes" are mutable, because this returns a proxy */
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	              "(" bytes_casesplit_params ")->?S?.\n"
	              "Same as ?#split, however ascii-casing is ignored during character comparisons\n"
	              "The returned bytes objects are views of @this byte object, meaning they "
	              /**/ "have the same ?#iswritable characteristics as @this, and refer to the same "
	              /**/ "memory"),
	TYPE_METHOD_F("splitlines", &bytes_splitlines,
	              /* CONSTCALL even though "Bytes" are mutable, because this returns a proxy */
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	              "(" bytes_splitlines_params ")->?S?.\n"
	              "Split @this ?. object at each linefeed, returning a sequence of all contained lines\n"
	              "When @keepends is ?f, this is identical to ${this.unifylines().split(\"\\n\")}\n"
	              "When @keepends is ?t, items found in the returned sequence will still have their "
	              /**/ "original, trailing line-feed appended\n"
	              "This function recognizes $\"\\n\", $\"\\r\" and $\"\\r\\n\" as linefeed sequences\n"
	              "The returned bytes objects are views of @this byte object, meaning they "
	              /**/ "have the same ?#iswritable characteristics as @this, and refer to the same "
	              /**/ "memory"),

	/* String indentation. */
	TYPE_METHOD_F("indent", &bytes_indent,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES,
	              "(" bytes_indent_params ")->?.\n"
	              "Using @this ?. object as result, insert @filler at the front, as well as after "
	              /**/ "every ascii-linefeed with the exception of one that may be located at its end\n"
	              "The intended use is for generating strings from structured data, such as HTML:\n"
	              "${"
	              "text = \"<html>\n{}\n</html>\".format({\n"
	              "	get_html_bytes().strip().indent()\n"
	              "});"
	              "}"),
	TYPE_METHOD_F("dedent", &bytes_dedent,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES,
	              "(" bytes_dedent_params ")->?.\n"
	              "Using @this string as result, remove up to @max_chars whitespace "
	              /**/ "(s.a. ?#isspace) characters, or if given characters apart of @mask "
	              /**/ "from the front, as well as following any linefeed"),

	/* Common-character search functions. */
	TYPE_METHOD_F("common", &bytes_common,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	              "" get_bcompare_decl("other", "->?Dint") "\n"
	              "Returns the number of common leading bytes shared between @this and @other, "
	              /**/ "or in other words: the lowest index $i for which ${this[i] != other.bytes()[i]} is true"),
	TYPE_METHOD_F("rcommon", &bytes_rcommon,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	              "" get_bcompare_decl("other", "->?Dint") "\n"
	              "Returns the number of common trailing bytes shared between @this and @other"),
	TYPE_METHOD_F("casecommon", &bytes_casecommon,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	              "" get_bcompare_decl("other", "->?Dint") "\n"
	              "Same as ?#common, however ascii-casing is ignored during character comparisons"),
	TYPE_METHOD_F("casercommon", &bytes_casercommon,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	              "" get_bcompare_decl("other", "->?Dint") "\n"
	              "Same as ?#rcommon, however ascii-casing is ignored during character comparisons"),

	/* Find match character sequences */
	TYPE_METHOD_F("findmatch", &bytes_findmatch,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	              "(" bytes_findmatch_params ")->?Dint\n"
	              "Similar to ?#find, but do a recursive search for the "
	              /**/ "first @close that doesn't have a match @{open}\n"
	              "For more information, see :string.findmatch"),
	TYPE_METHOD_F("indexmatch", &bytes_indexmatch,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	              "(" bytes_indexmatch_params ")->?Dint\n"
	              "#tItemNotFound{No instance of @close without a matching @open exists within ${this.substr(start, end)}}"
	              "Same as ?#findmatch, but throw :ItemNotFound instead of "
	              /**/ "returning ${-1} if no @close without a matching @open exists"),
	TYPE_METHOD_F("casefindmatch", &bytes_casefindmatch,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	              "(" bytes_casefindmatch_params ")->?X2?T2?Dint?Dint?N\n"
	              "Same as ?#findmatch, however casing is ignored during character comparisons\n"
	              "Upon success, the second returned integer is equal to ${return[0] + ?#close}\n"
	              "If no match if found, ?N is returned"),
	TYPE_METHOD_F("caseindexmatch", &bytes_caseindexmatch,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	              "(" bytes_caseindexmatch_params ")->?T2?Dint?Dint\n"
	              "#tIndexError{No instance of @close without a matching @open exists within ${this.substr(start, end)}}"
	              "Same as ?#indexmatch, however casing is ignored during character comparisons\n"
	              "Upon success, the second returned integer is equal to ${return[0] + ?#close}"),
	TYPE_METHOD_F("rfindmatch", &bytes_rfindmatch,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	              "(" bytes_rfindmatch_params ")->?Dint\n"
	              "Similar to ?#findmatch, but operate in a mirrored fashion, searching for the "
	              /**/ "last instance of @open that has no matching @close within ${this.substr(start, end)}:\n"
	              "${"
	              /**/ "s = \"get_string().foo(bar(), baz(42), 7).length\".bytes();\n"
	              /**/ "lcol = s.rfind(\")\");\n"
	              /**/ "print lcol; /* 34 */\n"
	              /**/ "mtch = s.rfindmatch(\"(\", \")\", 0, lcol);\n"
	              /**/ "print repr s[mtch:lcol + 1]; /* \"(bar(), baz(42), 7)\".bytes() */"
	              "}\n"
	              "If no @open without a matching @close exists, ${-1} is returned"),
	TYPE_METHOD_F("rindexmatch", &bytes_rindexmatch,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	              "(" bytes_rindexmatch_params ")->?Dint\n"
	              "#tItemNotFound{No instance of @open without a matching @close exists within ${this.substr(start, end)}}"
	              "Same as ?#rfindmatch, but throw :ItemNotFound instead of returning ${-1} if no @open without a matching @close exists"),
	TYPE_METHOD_F("caserfindmatch", &bytes_caserfindmatch,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	              "(" bytes_caserfindmatch_params ")->?X2?T2?Dint?Dint?N\n"
	              "Same as ?#rfindmatch, however ascii-casing is ignored during character comparisons\n"
	              "Upon success, the second returned integer is equal to ${return[0] + ?#open}\n"
	              "If no match if found, ?N is returned"),
	TYPE_METHOD_F("caserindexmatch", &bytes_caserindexmatch,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	              "(" bytes_caserindexmatch_params ")->?T2?Dint?Dint\n"
	              "#tItemNotFound{No instance of @open without a matching @close exists within ${this.substr(start, end)}}"
	              "Same as ?#rindexmatch, however ascii-casing is ignored during character comparisons\n"
	              "Upon success, the second returned integer is equal to ${return[0] + ?#open}"),

	/* Using the find-match functionality, also provide a partitioning version */
	TYPE_METHOD_F("partitionmatch", &bytes_partitionmatch,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES,
	              "(" bytes_partitionmatch_params ")->?T3?.?.?.\n"
	              "A hybrid between ?#find, ?#findmatch and ?#partition that returns the strings surrounding "
	              /**/ "the matched string portion, the first being the substring prior to the match, "
	              /**/ "the second being the matched string itself (including the @open and @close strings), "
	              /**/ "and the third being the substring after the match\n"
	              "For more information see ?Apartitionmatch?Dstring"),
	TYPE_METHOD_F("rpartitionmatch", &bytes_rpartitionmatch,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES,
	              "(" bytes_rpartitionmatch_params ")->?T3?.?.?.\n"
	              "A hybrid between ?#rfind, ?#rfindmatch and ?#rpartition that returns the strings surrounding "
	              /**/ "the matched string portion, the first being the substring prior to the match, "
	              /**/ "the second being the matched string itself (including the @open and @close strings), "
	              /**/ "and the third being the substring after the match.\n"
	              "For more information see ?Arpartitionmatch?Dstring"),
	TYPE_METHOD_F("casepartitionmatch", &bytes_casepartitionmatch,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES,
	              "(" bytes_casepartitionmatch_params ")->?T3?.?.?.\n"
	              "Same as ?#partitionmatch, however casing is ignored during character comparisons"),
	TYPE_METHOD_F("caserpartitionmatch", &bytes_caserpartitionmatch,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES,
	              "(" bytes_caserpartitionmatch_params ")->?T3?.?.?.\n"
	              "Same as ?#rpartitionmatch, however casing is ignored during character comparisons"),

	TYPE_METHOD_F("segments", &bytes_segments,
	              /* CONSTCALL even though "Bytes" are mutable, because this returns a proxy */
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	              "(" bytes_segments_params ")->?S?.\n"
	              "Split @this ?. object into segments, each exactly @substring_length characters long, with the "
	              /**/ "last segment containing the remaining characters and having a length of between "
	              /**/ "$1 and @substring_length characters.\n"
	              "This function is similar to ?#distribute, but instead of being given the "
	              /**/ "amount of sub-strings and figuring out their lengths, this function takes "
	              /**/ "the length of sub-strings and figures out their amount"),
	TYPE_METHOD_F("distribute", &bytes_distribute,
	              /* CONSTCALL even though "Bytes" are mutable, because this returns a proxy */
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	              "(" bytes_distribute_params ")->?S?.\n"
	              "Split @this ?. object into @substring_count similarly sized sub-strings, each with a "
	              /**/ "length of ${(##this + (substring_count - 1)) / substring_count}, followed by a last, optional "
	              /**/ "sub-string containing all remaining characters.\n"
	              "This function is similar to ?#segments, but instead of being given the "
	              /**/ "length of sub-strings and figuring out their amount, this function takes "
	              /**/ "the amount of sub-strings and figures out their lengths"),

	/* Regex functions. */
	TYPE_KWMETHOD_F("rematch", &bytes_rematch,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	                "(pattern:?Dstring,start=!0,end=!-1,rules=!P{})->?X2?Dint?N\n"
	                "#tValueError{The given @pattern is malformed}"
	                "#r{The number of leading bytes in ${this.substr(start, end)} "
	                /*    */ "matched by @pattern, or ?N if @pattern doesn't match}"
	                "Check if ${this.substr(start, end)} matches the given regular expression @pattern\n"
	                "This function behaves similar to ${this.encode(\"ascii\").rematch(pattern)}, "
	                "except that the given pattern may match non-ASCII bytes with #C{\\xAB} or #C{\\0377} "
	                "escape sequences. Furthermore, unicode character escape sequences cannot be used in "
	                "@pattern. For more information, see ?Arematch?Dstring"),
	TYPE_KWMETHOD_F("rematches", &bytes_rematches,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	                "(pattern:?Dstring,start=!0,end=!-1,rules=!P{})->?Dbool\n"
	                "#ppattern{The regular expression pattern (s.a. ?#rematch)}"
	                "#prules{The regular expression rules (s.a. ?#rematch)}"
	                "#tValueError{The given @pattern is malformed}"
	                "Check if @pattern matches the entirety of the specified range of @this ?.\n"
	                "This function behaves identical to ${this.rematch(...) == ?#this}"),
	TYPE_KWMETHOD_F("refind", &bytes_refind,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	                "(pattern:?Dstring,start=!0,end=!-1,range=!-1,rules=!P{})->?X2?T2?Dint?Dint?N\n"
	                "#ppattern{The regular expression pattern (s.a. ?#rematch)}"
	                "#prange{The max number of search attempts to perform}"
	                "#prules{The regular expression rules (s.a. ?#rematch)}"
	                "#tValueError{The given @pattern is malformed}"
	                "Find the first sub-string matched by @pattern, and return its start/end indices, or ?N if no match exists\n"
	                "Note that using ?N in an expand expression will expand to the all ?N-values"),
	TYPE_KWMETHOD_F("rerfind", &bytes_rerfind,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	                "(pattern:?Dstring,start=!0,end=!-1,range=!-1,rules=!P{})->?X2?T2?Dint?Dint?N\n"
	                "#ppattern{The regular expression pattern (s.a. ?#rematch)}"
	                "#prange{The max number of search attempts to perform}"
	                "#prules{The regular expression rules (s.a. ?#rematch)}"
	                "#tValueError{The given @pattern is malformed}"
	                "Find the last sub-string matched by @pattern, and return its start/end indices, "
	                /**/ "or ?N if no match exists (s.a. #refind)"),
	TYPE_KWMETHOD_F("reindex", &bytes_reindex,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	                "(pattern:?Dstring,start=!0,end=!-1,range=!-1,rules=!P{})->?T2?Dint?Dint\n"
	                "#ppattern{The regular expression pattern (s.a. ?#rematch)}"
	                "#prange{The max number of search attempts to perform}"
	                "#prules{The regular expression rules (s.a. ?#rematch)}"
	                "#tValueError{The given @pattern is malformed}"
	                "#tRegexNotFound{No substring matching the given @pattern could be found}"
	                "Same as ?#refind, but throw :RegexNotFound when no match can be found"),
	TYPE_KWMETHOD_F("rerindex", &bytes_rerindex,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	                "(pattern:?Dstring,start=!0,end=!-1,range=!-1,rules=!P{})->?T2?Dint?Dint\n"
	                "#ppattern{The regular expression pattern (s.a. ?#rematch)}"
	                "#prange{The max number of search attempts to perform}"
	                "#prules{The regular expression rules (s.a. ?#rematch)}"
	                "#tValueError{The given @pattern is malformed}"
	                "#tRegexNotFound{No substring matching the given @pattern could be found}"
	                "Same as ?#rerfind, but throw :RegexNotFound when no match can be found"),
	TYPE_KWMETHOD_F("relocate", &bytes_relocate,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES,
	                "(pattern:?Dstring,start=!0,end=!-1,range=!-1,rules=!P{})->?X2?.?N\n"
	                "#ppattern{The regular expression pattern (s.a. ?#rematch)}"
	                "#prange{The max number of search attempts to perform}"
	                "#prules{The regular expression rules (s.a. ?#rematch)}"
	                "#tValueError{The given @pattern is malformed}"
	                "Same as ${this.substr(this.refind(pattern, start, end, rules)...)}\n"
	                "In other words: return the first sub-string matched by the "
	                /**/ "given regular expression, or ?N if not found\n"
	                "This function has nothing to do with relocations! - it's pronounced R.E. locate"),
	TYPE_KWMETHOD_F("rerlocate", &bytes_rerlocate,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES,
	                "(pattern:?Dstring,start=!0,end=!-1,range=!-1,rules=!P{})->?X2?.?N\n"
	                "#ppattern{The regular expression pattern (s.a. ?#rematch)}"
	                "#prange{The max number of search attempts to perform}"
	                "#prules{The regular expression rules (s.a. ?#rematch)}"
	                "#tValueError{The given @pattern is malformed}"
	                "Same as ${this.substr(this.rerfind(pattern, start, end, rules)...)}\n"
	                "In other words: return the last sub-string matched by the "
	                /**/ "given regular expression, or ?N if not found"),
	TYPE_KWMETHOD_F("repartition", &bytes_repartition,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES,
	                "(pattern:?Dstring,start=!0,end=!-1,range=!-1,rules=!P{})->?T3?.?.?.\n"
	                "#ppattern{The regular expression pattern (s.a. ?#rematch)}"
	                "#prange{The max number of search attempts to perform}"
	                "#prules{The regular expression rules (s.a. ?#rematch)}"
	                "#tValueError{The given @pattern is malformed}"
	                "A hybrid between ?#refind and ?#partition\n${"
	                "function repartition(pattern: string, start: int, end: int, rules: string) {\n"
	                "	local start, end = this.refind(pattern, start, end, rules)...;\n"
	                "	if (start is none)\n"
	                "		return (this, \"\".bytes(), \"\".bytes());\n"
	                "	return (\n"
	                "		this.substr(0, start),\n"
	                "		this.substr(start, end),\n"
	                "		this.substr(end, -1)\n"
	                "	);\n"
	                "}}"),
	TYPE_KWMETHOD_F("rerpartition", &bytes_rerpartition,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES,
	                "(pattern:?Dstring,start=!0,end=!-1,range=!-1,rules=!P{})->?T3?.?.?.\n"
	                "#ppattern{The regular expression pattern (s.a. ?#rematch)}"
	                "#prange{The max number of search attempts to perform}"
	                "#prules{The regular expression rules (s.a. ?#rematch)}"
	                "#tValueError{The given @pattern is malformed}"
	                "A hybrid between ?#rerfind and ?#rpartition\n${"
	                "function rerpartition(pattern: string, start: int, end: int, rules: string) {\n"
	                "	local start, end = this.rerfind(pattern, start, end, rules)...;\n"
	                "	if (start is none)\n"
	                "		return (\"\".bytes(), \"\".bytes(), this);\n"
	                "	return (\n"
	                "		this.substr(0, start),\n"
	                "		this.substr(start, end), \n"
	                "		this.substr(end, -1)\n"
	                "	);\n"
	                "}}"),
	TYPE_KWMETHOD_F("rereplace", &bytes_rereplace,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES,
	                "(pattern:?Dstring,replace:?.,max=!-1,rules=!P{})->?.\n"
	                "#ppattern{The regular expression pattern (s.a. ?#rematch)}"
	                "#prules{The regular expression rules (s.a. ?#rematch)}"
	                "#tValueError{The given @pattern is malformed}"
	                "Similar to ?#replace, however the ?. to search for is implemented as a regular expression "
	                "pattern, with the sub-string matched by it then getting replaced by @replace\n"
	                "Locations where @pattern matches epsilon are not replaced\n"
	                "Additionally, @replace may contain sed-like match sequences:\n"
	                "#T{Expression|Description~"
	                /**/ "#C{&}|Replaced with the entire sub-string matched by @pattern&"
	                /**/ "#C{\\n}|Where $n is a digit ${1-9} specifying the n'th (1-based) group in "
	                /**/ /*   */ "@pattern (groups are determined by parenthesis in regex patterns)&"
	                /**/ "#C{\\\\\\\\}|Outputs a literal $r\"\\\" into the returned ?.&"
	                /**/ "#C{\\#&}|Outputs a literal $r\"#&\" into the returned ?."
	                "}"),
	TYPE_KWMETHOD_F("refindall", &bytes_refindall,
	                /* CONSTCALL even though "Bytes" are mutable, because this returns a proxy */
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	                "(pattern:?Dstring,start=!0,end=!-1,rules=!P{})->?S?T2?Dint?Dint\n"
	                "#ppattern{The regular expression pattern (s.a. ?#rematch)}"
	                "#prules{The regular expression rules (s.a. ?#rematch)}"
	                "#tValueError{The given @pattern is malformed}"
	                "Similar to ?#refind, but return a sequence of all matches found within ${this.substr(start, end)}\n"
	                "Locations where @pattern matches epsilon are not included in the returned sequence\n"
	                "Note that the matches returned are ordered ascendingly"),
	TYPE_KWMETHOD_F("relocateall", &bytes_relocateall,
	                /* CONSTCALL even though "Bytes" are mutable, because this returns a proxy */
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	                "(pattern:?Dstring,start=!0,end=!-1,rules=!P{})->?S?.\n"
	                "#ppattern{The regular expression pattern (s.a. ?#rematch)}"
	                "#prules{The regular expression rules (s.a. ?#rematch)}"
	                "#tValueError{The given @pattern is malformed}"
	                "Similar to ?#relocate, but return a sequence of all matched "
	                "sub-strings found within ${this.substr(start, end)}\n"
	                "Note that the matches returned are ordered ascendingly\n"
	                "Locations where @pattern matches epsilon are not included in the returned sequence\n"
	                "This function has nothing to do with relocations! - it's pronounced R.E. locate all"),
	TYPE_KWMETHOD_F("resplit", &bytes_resplit,
	                /* CONSTCALL even though "Bytes" are mutable, because this returns a proxy */
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	                "(pattern:?Dstring,start=!0,end=!-1,rules=!P{})->?S?.\n"
	                "#ppattern{The regular expression pattern (s.a. ?#rematch)}"
	                "#prules{The regular expression rules (s.a. ?#rematch)}"
	                "#tValueError{The given @pattern is malformed}"
	                "Similar to ?#split, but use a regular expression in order to "
	                "express the sections of the ?. around which to perform the split\n"
	                "Locations where @pattern matches epsilon do not trigger a split\n"

	                "${"
	                /**/ "local data = \"10 , 20,30 40, 50\".bytes();\n"
	                /**/ "for (local x: data.resplit(r\"[[:space:],]+\"))\n"
	                /**/ "	print x; /* `10' `20' `30' `40' `50' */"
	                "}\n"

	                "If you wish to do the inverse and enumerate matches, rather than the "
	                "strings between matches, use ?#relocateall instead, which also behaves "
	                "as a sequence"),
	TYPE_KWMETHOD_F("restartswith", &bytes_restartswith,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	                "(pattern:?Dstring,start=!0,end=!-1,rules=!P{})->?Dbool\n"
	                "#ppattern{The regular expression pattern (s.a. ?#rematch)}"
	                "#prules{The regular expression rules (s.a. ?#rematch)}"
	                "#tValueError{The given @pattern is malformed}"
	                "Check if @this ?. starts with a regular expression described by @pattern (s.a. ?#startswith)\n"
	                "${"
	                /**/ "function restartswith(pattern: string) {\n"
	                /**/ "	return this.rematch(pattern) !is none;\n"
	                /**/ "}"
	                "}"),
	TYPE_KWMETHOD_F("reendswith", &bytes_reendswith,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	                "(pattern:?Dstring,start=!0,end=!-1,rules=!P{})->?Dbool\n"
	                "#ppattern{The regular expression pattern (s.a. ?#rematch)}"
	                "#prules{The regular expression rules (s.a. ?#rematch)}"
	                "#tValueError{The given @pattern is malformed}"
	                "Check if @this ?. ends with a regular expression described by @pattern (s.a. ?#endswith)\n"
	                "${"
	                /**/ "function restartswith(pattern: string) {\n"
	                /**/ "	local rpos = this.rerfind(pattern);\n"
	                /**/ "	return rpos !is none && rpos[1] == ##this;\n"
	                /**/ "}"
	                "}"),
	TYPE_KWMETHOD_F("restrip", &bytes_restrip,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES,
	                "(pattern:?Dstring,start=!0,end=!-1,rules=!P{})->?.\n"
	                "#ppattern{The regular expression pattern (s.a. ?#rematch)}"
	                "#prules{The regular expression rules (s.a. ?#rematch)}"
	                "#tValueError{The given @pattern is malformed}"
	                "Strip all leading and trailing matches for @pattern from @this ?. and return the result (s.a. ?#strip)"),
	TYPE_KWMETHOD_F("relstrip", &bytes_relstrip,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES,
	                "(pattern:?Dstring,start=!0,end=!-1,rules=!P{})->?.\n"
	                "#ppattern{The regular expression pattern (s.a. ?#rematch)}"
	                "#prules{The regular expression rules (s.a. ?#rematch)}"
	                "#tValueError{The given @pattern is malformed}"
	                "Strip all leading matches for @pattern from @this ?. and return the result (s.a. ?#lstrip)"),
	TYPE_KWMETHOD_F("rerstrip", &bytes_rerstrip,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES,
	                "(pattern:?Dstring,start=!0,end=!-1,rules=!P{})->?.\n"
	                "#ppattern{The regular expression pattern (s.a. ?#rematch)}"
	                "#prules{The regular expression rules (s.a. ?#rematch)}"
	                "#tValueError{The given @pattern is malformed}"
	                "Strip all trailing matches for @pattern from @this ?. and return the result (s.a. ?#lstrip)"),
	TYPE_KWMETHOD_F("recount", &bytes_recount,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	                "(pattern:?Dstring,start=!0,end=!-1,rules=!P{})->?Dint\n"
	                "#ppattern{The regular expression pattern (s.a. ?#rematch)}"
	                "#prules{The regular expression rules (s.a. ?#rematch)}"
	                "#tValueError{The given @pattern is malformed}"
	                "Count the number of matches of a given regular expression @pattern (s.a. ?#count)\n"
	                "Hint: This is the same as ${##this.refindall(pattern)} or ${##this.relocateall(pattern)}\n"
	                "Instances where @pattern matches epsilon are not counted"),
	TYPE_KWMETHOD_F("recontains", &bytes_recontains,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	                "(pattern:?Dstring,start=!0,end=!-1,rules=!P{})->?Dbool\n"
	                "#ppattern{The regular expression pattern (s.a. ?#rematch)}"
	                "#prules{The regular expression rules (s.a. ?#rematch)}"
	                "#tValueError{The given @pattern is malformed}"
	                "Check if @this contains a match for the given regular expression @pattern (s.a. ?#contains)\n"
	                "Hint: This is the same as ${!!this.refindall(pattern)} or ${!!this.relocateall(pattern)}"),
	TYPE_KWMETHOD_F("rescanf", &bytes_rescanf,
	                /* CONSTCALL even though "Bytes" are mutable, because this returns a proxy */
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	                "(pattern:?.,start=!0,end=!-1,rules=!P{})->?S?X2?.?N\n"
	                "#ppattern{The regular expression pattern (s.a. ?#rematch)}"
	                "#prange{The max number of search attempts to perform}"
	                "#prules{The regular expression rules (s.a. ?#rematch)}"
	                "#tValueError{The given @pattern is malformed}"
	                "Similar to ?#regfind, but rather than return a list of matched offsets, a sequence of "
	                "matched strings is returned, allowing the user to easily extract matched text in a way "
	                "that is similar to ?#scanf. Returns an empty sequence when @pattern can't be matched."),

	/* Regex functions that return the start-/end-offsets of all groups (rather than only the whole match) */
	TYPE_KWMETHOD_F("regmatch", &bytes_regmatch,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	                "(pattern:?Dstring,start=!0,end=!-1,rules=!P{})->?S?X2?T2?Dint?Dint?N\n"
	                "Similar to ?#rematch, but rather than only return the number of characters that were "
	                /**/ "matched by the regular expression as a whole, return a sequence of start-/end-"
	                /**/ "offsets for both the whole match itself (in ${return[0]}), as well as the "
	                /**/ "start-/end-offsets of each individual group referenced by @pattern. Groups "
	                /**/ "that didn't get matched (because they might be optional) appear as ?N in the "
	                /**/ "returned sequence.\n"
	                "When nothing was matched, an empty sequence is returned.\n"
	                "Example:\n"
	                "${"
	                /**/ "local groups = \"foo bar foobar\".bytes().regmatch(r\"fo(o) (b(x)r|bar) fo(o?bar)\");\n"
	                /**/ "assert groups == {\n"
	                /**/ "	{0, 14},  /* Whole match */\n"
	                /**/ "	{2, 3},   /* \"o\" */\n"
	                /**/ "	{4, 7},   /* \"bar\" */\n"
	                /**/ "	none,     /* never matched: \"x\" */\n"
	                /**/ "	{10, 14}, /* \"obar\" */\n"
	                /**/ "};"
	                "}\n"
	                "Note that (if something was matched), this function still only matches at the "
	                "start of @this ?.. If you want to search for @pattern and get the offsets of "
	                "all of the matched groups, you should use ?#regfind instead."),
	TYPE_KWMETHOD_F("regfind", &bytes_regfind,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	                "(pattern:?Dstring,start=!0,end=!-1,rules=!P{})->?S?X2?T2?Dint?Dint?N\n"
	                "Similar to ?#refind, but rather than only return the character-range "
	                /**/ "matched by the regular expression as a whole, return a sequence of start-/end-"
	                /**/ "offsets for both the whole match itself (in ${return[0]}), as well as the "
	                /**/ "start-/end-offsets of each individual group referenced by @pattern.\n"
	                "When nothing was matched, an empty sequence is returned (s.a. ?#regmatch)."),
	TYPE_KWMETHOD_F("regrfind", &bytes_regrfind,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	                "(pattern:?Dstring,start=!0,end=!-1,rules=!P{})->?S?X2?T2?Dint?Dint?N\n"
	                "Similar to ?#rerfind, but rather than only return the character-range "
	                /**/ "matched by the regular expression as a whole, return a sequence of start-/end-"
	                /**/ "offsets for both the whole match itself (in ${return[0]}), as well as the "
	                /**/ "start-/end-offsets of each individual group referenced by @pattern.\n"
	                "When nothing was matched, an empty sequence is returned (s.a. ?#regmatch)."),
	TYPE_KWMETHOD_F("regfindall", &bytes_regfindall,
	                /* CONSTCALL even though "Bytes" are mutable, because this returns a proxy */
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	                "(pattern:?Dstring,start=!0,end=!-1,rules=!P{})->?S?S?X2?T2?Dint?Dint?N\n"
	                "Similar to ?#refindall, but rather than only return the character-ranges "
	                /**/ "matched by the regular expression as a whole, return a sequence of start-/end-"
	                /**/ "offsets for both the whole match itself (in ${return.each[0]}), as well as the "
	                /**/ "start-/end-offsets of each individual group referenced by @pattern.\n"
	                "When nothing was matched, an empty sequence is returned (s.a. ?#regmatch)."),
	TYPE_KWMETHOD_F("regindex", &bytes_regindex,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	                "(pattern:?Dstring,start=!0,end=!-1,rules=!P{})->?S?X2?T2?Dint?Dint?N\n"
	                "#ppattern{The regular expression pattern (s.a. ?#rematch)}"
	                "#prange{The max number of search attempts to perform}"
	                "#prules{The regular expression rules (s.a. ?#rematch)}"
	                "#tValueError{The given @pattern is malformed}"
	                "#tRegexNotFound{No substring matching the given @pattern could be found}"
	                "Same as ?#regfind, but throw :RegexNotFound when no match can be found"),
	TYPE_KWMETHOD_F("regrindex", &bytes_regrindex,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	                "(pattern:?Dstring,start=!0,end=!-1,rules=!P{})->?S?X2?T2?Dint?Dint?N\n"
	                "#ppattern{The regular expression pattern (s.a. ?#rematch)}"
	                "#prange{The max number of search attempts to perform}"
	                "#prules{The regular expression rules (s.a. ?#rematch)}"
	                "#tValueError{The given @pattern is malformed}"
	                "#tRegexNotFound{No substring matching the given @pattern could be found}"
	                "Same as ?#regrfind, but throw :RegexNotFound when no match can be found"),

	TYPE_KWMETHOD_F("reglocate", &bytes_reglocate,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	                "(pattern:?.,start=!0,end=!-1,range=!-1,rules=!P{})->?S?X2?.?N\n"
	                "#ppattern{The regular expression pattern (s.a. ?#rematch)}"
	                "#prange{The max number of search attempts to perform}"
	                "#prules{The regular expression rules (s.a. ?#rematch)}"
	                "#tValueError{The given @pattern is malformed}"
	                "Same as ?#regfind, but returns a sequence of the sub-strings matched by each group, "
	                /**/ "rather than the start/end bounds of each group (still returns !n for unmatched "
	                /**/ "groups, and an empty sequence if @pattern isn't matched at all)\n"
	                "Behaves the same as ${this.rescanf(f\".*({pattern})\", ...)}"),
	TYPE_KWMETHOD_F("regrlocate", &bytes_regrlocate,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	                "(pattern:?.,start=!0,end=!-1,range=!-1,rules=!P{})->?S?X2?.?N\n"
	                "#ppattern{The regular expression pattern (s.a. ?#rematch)}"
	                "#prange{The max number of search attempts to perform}"
	                "#prules{The regular expression rules (s.a. ?#rematch)}"
	                "#tValueError{The given @pattern is malformed}"
	                "Same as ?#regrfind, but returns a sequence of the sub-strings matched by each group, "
	                /**/ "rather than the start/end bounds of each group (still returns !n for unmatched "
	                /**/ "groups, and an empty sequence if @pattern isn't matched at all)"),
	TYPE_KWMETHOD_F("reglocateall", &bytes_reglocateall,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	                "(pattern:?.,start=!0,end=!-1,range=!-1,rules=!P{})->?S?S?X2?.?N\n"
	                "#ppattern{The regular expression pattern (s.a. ?#rematch)}"
	                "#prange{The max number of search attempts to perform}"
	                "#prules{The regular expression rules (s.a. ?#rematch)}"
	                "#tValueError{The given @pattern is malformed}"
	                "Same as ?#regfindall, but returns a sequence of the sub-strings matched by each group of each match, "
	                /**/ "rather than the start/end bounds of each group (still returns !n for unmatched groups)"),

	TYPE_METHOD_HINTREF(Sequence_xchitem),
	TYPE_METHOD_END
};

INTERN_TPCONST struct type_method_hint tpconst bytes_method_hints[] = {
	TYPE_METHOD_HINT_F(seq_xchitem_index, &bytes_xchitem_index, METHOD_FNOREFESCAPE),
	/* TODO: All the other method hints (after all we *do* define our own "find" and
	 *       so on, so we *really* need to override these so that "Sequence" doesn't
	 *       think that our "find" is compatible with *its*) */
	TYPE_METHOD_HINT_END
};


DECL_END

#ifndef __INTELLISENSE__
#include "bytes_finder.c.inl"
#include "bytes_segments.c.inl"
#include "bytes_split.c.inl"
#endif /* !__INTELLISENSE__ */

#endif /* !GUARD_DEEMON_OBJECTS_UNICODE_BYTES_FUNCTIONS_C_INL */
