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
#ifndef GUARD_DEX_ICONV_LIBICONV_C
#define GUARD_DEX_ICONV_LIBICONV_C 1
#define CONFIG_BUILDING_LIBICONV
#define DEE_SOURCE

#include "libiconv.h"
/**/

#include <deemon/api.h>

#include <deemon/alloc.h>           /* DeeObject_MALLOC */
#include <deemon/arg.h>             /* DeeArg_UnpackStruct*, UNPu32, UNPuSIZ */
#include <deemon/bool.h>            /* return_bool */
#include <deemon/bytes.h>           /* DeeBytes*, Dee_bytes_printer, Dee_bytes_printer_* */
#include <deemon/dex.h>             /* DEX_*, Dee_DEXSYM_READONLY */
#include <deemon/error.h>           /* DeeError_* */
#include <deemon/format.h>          /* PCKuN, PRFuSIZ */
#include <deemon/int.h>             /* DeeInt_NEWU */
#include <deemon/none.h>            /* return_none */
#include <deemon/object.h>          /* DREF, DeeObject, DeeObject_*, Dee_AsObject, Dee_formatprinter_t, Dee_ssize_t */
#include <deemon/objmethod.h>
#include <deemon/string.h>          /* DeeString*, Dee_STRING_ERROR_FIGNORE, Dee_UNICODE_PRINTER_INIT, Dee_unicode_printer*, STRING_ERROR_FSTRICT, WSTR_LENGTH */
#include <deemon/system-features.h> /* strcmp */
#include <deemon/tuple.h>           /* DeeTuple_Newf */
#include <deemon/type.h>            /* DeeObject_InitStatic, METHOD_FNORMAL */

#include <hybrid/typecore.h> /* __SIZEOF_INTPTR_HALF_T__ */

#include <stdbool.h> /* bool, false */
#include <stddef.h>  /* NULL, size_t */
#include <stdint.h>  /* uint16_t, uint32_t */

DECL_BEGIN

INTERN ATTR_COLD int DCALL
err_unicode_decode_error(iconv_codec_t codec, size_t offset) {
	return DeeError_Throwf(&DeeError_UnicodeDecodeError,
	                       "Failed to decode %s at offset %" PRFuSIZ,
	                       libiconv_getcodecnames(codec), offset);
}

INTERN ATTR_COLD int DCALL
err_unicode_encode_error(iconv_codec_t codec, size_t offset) {
	return DeeError_Throwf(&DeeError_UnicodeEncodeError,
	                       "Failed to encode %s at offset %" PRFuSIZ,
	                       libiconv_getcodecnames(codec), offset);
}

INTERN ATTR_COLD int DCALL
err_unicode_reencode_error(iconv_codec_t codec) {
	return DeeError_Throwf(&DeeError_UnicodeEncodeError,
	                       "Failed to re-encode data as %s",
	                       libiconv_getcodecnames(codec));
}


INTERN ATTR_COLD int DCALL
err_unknown_codec(iconv_codec_t codec) {
	return DeeError_Throwf(&DeeError_ValueError, "Unknown codec: %u", (unsigned int)codec);
}


/*[[[deemon (print_CMethod from rt.gen.unpack)("codecbyname", """
	DeeStringObject *name
""", libname: "deemon_iconv");]]]*/
#define deemon_iconv_codecbyname_params "name:?Dstring"
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL deemon_iconv_codecbyname_f_impl(DeeStringObject *name);
PRIVATE DEFINE_CMETHOD1(deemon_iconv_codecbyname, &deemon_iconv_codecbyname_f_impl, METHOD_FNORMAL);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL deemon_iconv_codecbyname_f_impl(DeeStringObject *name)
/*[[[end]]]*/
{
	iconv_codec_t result;
	if (DeeObject_AssertTypeExact(name, &DeeString_Type))
		goto err;
	result = libiconv_codecbynamez(DeeString_STR(name), DeeString_SIZE(name));
	if (result == ICONV_CODEC_UNKNOWN)
		return_none;
	return DeeInt_NEWU(result);
err:
	return NULL;
}

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
do_iconv_normalize_codec_name(DeeStringObject *__restrict str);

/*[[[deemon (print_CMethod from rt.gen.unpack)("normalizecodecname", """
	DeeStringObject *name
""", libname: "deemon_iconv");]]]*/
#define deemon_iconv_normalizecodecname_params "name:?Dstring"
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL deemon_iconv_normalizecodecname_f_impl(DeeStringObject *name);
PRIVATE DEFINE_CMETHOD1(deemon_iconv_normalizecodecname, &deemon_iconv_normalizecodecname_f_impl, METHOD_FNORMAL);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL deemon_iconv_normalizecodecname_f_impl(DeeStringObject *name)
/*[[[end]]]*/
{
	if (DeeObject_AssertTypeExact(name, &DeeString_Type))
		goto err;
	return do_iconv_normalize_codec_name(name);
err:
	return NULL;
}

/*[[[deemon (print_CMethod from rt.gen.unpack)("getcodecnames", """
	unsigned int id
""", libname: "deemon_iconv");]]]*/
#define deemon_iconv_getcodecnames_params "id:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL deemon_iconv_getcodecnames_f_impl(unsigned int id);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL deemon_iconv_getcodecnames_f(DeeObject *__restrict arg0) {
	unsigned int id;
	if (DeeObject_AsUInt(arg0, &id))
		goto err;
	return deemon_iconv_getcodecnames_f_impl(id);
err:
	return NULL;
}
PRIVATE DEFINE_CMETHOD1(deemon_iconv_getcodecnames, &deemon_iconv_getcodecnames_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL deemon_iconv_getcodecnames_f_impl(unsigned int id)
/*[[[end]]]*/
{
	DREF StrNulArray *result;
	char const *names = libiconv_getcodecnames((iconv_codec_t)id);
	if unlikely(!names)
		return_none;
	result = DeeObject_MALLOC(StrNulArray);
	if unlikely(!result)
		goto err;
	result->sna_base = names;
	DeeObject_InitStatic(result, &StrNulArray_Type);
	return Dee_AsObject(result);
err:
	return NULL;
}

/*[[[deemon (print_CMethod from rt.gen.unpack)("parsecodecname", """
	DeeStringObject *name
""", libname: "deemon_iconv");]]]*/
#define deemon_iconv_parsecodecname_params "name:?Dstring"
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL deemon_iconv_parsecodecname_f_impl(DeeStringObject *name);
PRIVATE DEFINE_CMETHOD1(deemon_iconv_parsecodecname, &deemon_iconv_parsecodecname_f_impl, METHOD_FNORMAL);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL deemon_iconv_parsecodecname_f_impl(DeeStringObject *name)
/*[[[end]]]*/
{
	iconv_codec_t result;
	uintptr_half_t flags;
	if (DeeObject_AssertTypeExact(name, &DeeString_Type))
		goto err;
	flags  = 0;
	result = libiconv_codec_and_flags_byname(DeeString_STR(name), &flags);
	if (result == ICONV_CODEC_UNKNOWN)
		return_none;
	return DeeTuple_Newf("u" PCKuN(__SIZEOF_INTPTR_HALF_T__),
	                     (unsigned int)result, flags);
err:
	return NULL;
}

/*[[[deemon (print_CMethod from rt.gen.unpack)("samecodec", """
	DeeStringObject *name1,
	DeeStringObject *name2,
""", libname: "deemon_iconv");]]]*/
#define deemon_iconv_samecodec_params "name1:?Dstring,name2:?Dstring"
FORCELOCAL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL deemon_iconv_samecodec_f_impl(DeeStringObject *name1, DeeStringObject *name2);
PRIVATE WUNUSED DREF DeeObject *DCALL deemon_iconv_samecodec_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeStringObject *name1;
		DeeStringObject *name2;
	} args;
	DeeArg_UnpackStruct2(err, argc, argv, "samecodec", &args, &args.name1, &args.name2);
	return deemon_iconv_samecodec_f_impl(args.name1, args.name2);
err:
	return NULL;
}
PRIVATE DEFINE_CMETHOD(deemon_iconv_samecodec, &deemon_iconv_samecodec_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL deemon_iconv_samecodec_f_impl(DeeStringObject *name1, DeeStringObject *name2)
/*[[[end]]]*/
{
	bool result;
	if (DeeObject_AssertTypeExact(name1, &DeeString_Type))
		goto err;
	if (DeeObject_AssertTypeExact(name2, &DeeString_Type))
		goto err;
	result = libiconv_same_codec_name(DeeString_STR(name1),
	                                  DeeString_STR(name2));
	return_bool(result);
err:
	return NULL;
}

/*[[[deemon (print_CMethod from rt.gen.unpack)("detect", """
	data:?X2?DBytes?Dstring
""", libname: "deemon_iconv");]]]*/
#define deemon_iconv_detect_params "data:?X2?DBytes?Dstring"
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL deemon_iconv_detect_f_impl(DeeObject *data);
PRIVATE DEFINE_CMETHOD1(deemon_iconv_detect, &deemon_iconv_detect_f_impl, METHOD_FNORMAL);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL deemon_iconv_detect_f_impl(DeeObject *data)
/*[[[end]]]*/
{
	iconv_codec_t result;
	void const *data_base;
	size_t data_size;
	if (DeeBytes_Check(data)) {
		data_base = DeeBytes_DATA(data);
		data_size = DeeBytes_SIZE(data);
	} else if (DeeString_Check(data)) {
		data_base = DeeString_AsBytes(data, false);
		if unlikely(!data_base)
			goto err;
		data_size = WSTR_LENGTH(data_base);
	} else {
		DeeObject_TypeAssertFailed2(data, &DeeBytes_Type, &DeeString_Type);
		goto err;
	}
	result = libiconv_detect_codec(data_base, data_size);
	if (result == ICONV_CODEC_UNKNOWN)
		return_none;
	return DeeInt_NEWU(result);
err:
	return NULL;
}

/*[[[deemon
import * from deemon;
import * from rt.gen.dexutils;
MODULE_NAME = "iconv";

include("libiconv-constants.def");
function Xgi(name, doc = none) {
	gi(f"TRANSLITERATE_{name}", value: f"ICONV_TRANSLITERATE_F_{name}", doc: doc);
}

gi("ICONV_CODEC_UNKNOWN", doc: "Unknown/unsupported codec.");
gi("ICONV_CODEC_FIRST",   doc: "First valid codec. Which codec this references is implementation-defined. "
                               "Actual codec IDs are internal and the actual codecs may only be referenced "
                               "by (one of) their names.");
Xgi("LOWER",    "For ?#transliterate / ?#_transliterate: attempt to use ?Alower?Dstring");
Xgi("UPPER",    "For ?#transliterate / ?#_transliterate: attempt to use ?Aupper?Dstring");
Xgi("TITLE",    "For ?#transliterate / ?#_transliterate: attempt to use ?Atitle?Dstring");
Xgi("FOLD",     "For ?#transliterate / ?#_transliterate: attempt to use ?Acasefold?Dstring");
Xgi("TRANSLIT", "For ?#transliterate / ?#_transliterate: replace with similar-looking character(s)");
Xgi("ALL",      "For ?#transliterate / ?#_transliterate: attempt everything");
]]]*/
#include "libiconv-constants.def"
/*[[[end]]]*/


/*[[[deemon (print_CMethod from rt.gen.unpack)("_transliterate", """
	uint32_t ord:?Dint,
	size_t nth:?Dint,
	unsigned int what:?Dint = ICONV_TRANSLITERATE_F_TRANSLIT =!GTRANSLITERATE_TRANSLIT
""", libname: "deemon_iconv");]]]*/
#define deemon_iconv__transliterate_params "ord:?Dint,nth:?Dint,what:?Dint=!GTRANSLITERATE_TRANSLIT"
FORCELOCAL WUNUSED DREF DeeObject *DCALL deemon_iconv__transliterate_f_impl(uint32_t ord, size_t nth, unsigned int what);
PRIVATE WUNUSED DREF DeeObject *DCALL deemon_iconv__transliterate_f(size_t argc, DeeObject *const *argv) {
	struct {
		uint32_t ord;
		size_t nth;
		unsigned int what;
	} args;
	args.what = ICONV_TRANSLITERATE_F_TRANSLIT;
	if (DeeArg_UnpackStruct(argc, argv, UNPu32 UNPuSIZ "|u:_transliterate", &args))
		goto err;
	return deemon_iconv__transliterate_f_impl(args.ord, args.nth, args.what);
err:
	return NULL;
}
PRIVATE DEFINE_CMETHOD(deemon_iconv__transliterate, &deemon_iconv__transliterate_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL deemon_iconv__transliterate_f_impl(uint32_t ord, size_t nth, unsigned int what)
/*[[[end]]]*/
{
	size_t result_len;
	char32_t result[ICONV_TRANSLITERATE_MAXLEN];
	result_len = libiconv_transliterate(result, ord, nth, what);
	if (result_len == (size_t)-1)
		return_none;
	return DeeString_NewUtf32((uint32_t const *)result, result_len, Dee_STRING_ERROR_FIGNORE);
}


struct codec_error {
	char     name[8];
	uint16_t flags;
};

PRIVATE struct codec_error const codec_error_db[] = {
	{ "strict",  ICONV_ERR_ERROR },
	{ "replace", ICONV_ERR_REPLACE },
	{ "ignore",  ICONV_ERR_IGNORE },
	/* Extra mode: discard data that cannot be encoded/decoded */
	{ "discard", ICONV_ERR_DISCARD },
};

PRIVATE WUNUSED uintptr_half_t DCALL
deemon_iconv_parse_error_mode(char const *errors) {
	size_t i;
	if (errors == NULL)
		return STRING_ERROR_FSTRICT;
	for (i = 0; i < COMPILER_LENOF(codec_error_db); ++i) {
		if (strcmp(codec_error_db[i].name, errors) != 0)
			continue;
		return (uintptr_half_t)codec_error_db[i].flags;
	}
	return (uintptr_half_t)DeeError_Throwf(&DeeError_ValueError,
	                                       "Invalid error mode %q",
	                                       errors);
}

/* @return: ICONV_CODEC_UNKNOWN: An error was thrown */
INTERN WUNUSED NONNULL((1, 3)) iconv_codec_t DCALL
deemon_iconv_parse_codec_name_and_error_mode(DeeObject *codec, DeeObject *errors,
                                             uintptr_half_t *__restrict p_flags) {
	iconv_codec_t result;
	if (errors == NULL) {
		*p_flags = ICONV_ERR_ERROR;
	} else if (DeeString_Check(errors)) {
		*p_flags = deemon_iconv_parse_error_mode(DeeString_STR(errors));
	} else {
		if (DeeObject_AsUIntX(errors, p_flags))
			goto err;
	}
	if (DeeString_Check(codec)) {
		result = libiconv_codec_and_flags_byname(DeeString_STR(codec), p_flags);
		if unlikely(result == ICONV_CODEC_UNKNOWN) {
			DeeError_Throwf(&DeeError_ValueError,
			                "Unknown codec: %q",
			                DeeString_STR(codec));
			goto err;
		}
	} else {
		if (DeeObject_AsUIntX(codec, &result))
			goto err;
		if unlikely(result == ICONV_CODEC_UNKNOWN) {
			err_unknown_codec(result);
			goto err;
		}
	}
	return result;
err:
	return ICONV_CODEC_UNKNOWN;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
iconv_do_decode(void const *data_base, size_t data_size,
                iconv_codec_t codec, uintptr_half_t flags) {
	Dee_ssize_t status;
	struct iconv_decode decoder;
	struct Dee_unicode_printer out_printer = Dee_UNICODE_PRINTER_INIT;
	struct iconv_printer input_printer;
	decoder.icd_output.ii_printer = &Dee_unicode_printer_print;
	decoder.icd_output.ii_arg     = &out_printer;
	decoder.icd_flags             = flags;
	decoder.icd_codec             = codec;
	if unlikely(do_libiconv_decode_init(&decoder, &input_printer))
		goto no_such_codec;
	status = (*input_printer.ii_printer)(input_printer.ii_arg, (char const *)data_base, data_size);
	if unlikely(status < 0)
		goto maybe_handle_iconv_error;
#if 0
	if (!libiconv_decode_isshiftzero(&decoder)) {
		/* Could potentially throw an error here, but don't... */
	}
#endif
	return Dee_unicode_printer_pack(&out_printer);
no_such_codec:
	err_unknown_codec(codec);
err_printer:
	Dee_unicode_printer_fini(&out_printer);
/*err:*/
	return NULL;
maybe_handle_iconv_error:
	if (decoder.icd_flags & ICONV_HASERR) {
		size_t offset = (size_t)(data_size + status);
		err_unicode_decode_error(codec, offset);
	} else {
		ASSERTF(status == -1, "The used printer 'Dee_unicode_printer_print' "
		                      "should only ever return `-1' to indicate errors");
	}
	goto err_printer;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
iconv_do_encode(void const *data_base, size_t data_size,
                iconv_codec_t codec, uintptr_half_t flags) {
	Dee_ssize_t status;
	struct iconv_encode encoder;
	struct Dee_bytes_printer out_printer;
	struct iconv_printer input_printer;
	Dee_bytes_printer_init_ex(&out_printer, data_size);
	encoder.ice_output.ii_printer = (Dee_formatprinter_t)&Dee_bytes_printer_append;
	encoder.ice_output.ii_arg     = &out_printer;
	encoder.ice_flags             = flags;
	encoder.ice_codec             = codec;
	if unlikely(do_libiconv_encode_init(&encoder, &input_printer))
		goto no_such_codec;
	status = (*input_printer.ii_printer)(input_printer.ii_arg, (char const *)data_base, data_size);
	if unlikely(status < 0)
		goto maybe_handle_iconv_error;
#if 0
	if (!libiconv_encode_isinputshiftzero(&encoder)) {
		/* Could potentially throw an error here, but don't... */
	}
#endif
	if unlikely(libiconv_encode_flush(&encoder) < 0)
		goto err_printer;
	return Dee_bytes_printer_pack(&out_printer);
no_such_codec:
	err_unknown_codec(codec);
err_printer:
	Dee_bytes_printer_fini(&out_printer);
/*err:*/
	return NULL;
maybe_handle_iconv_error:
	if (encoder.ice_flags & ICONV_HASERR) {
		size_t offset = (size_t)(data_size + status);
		err_unicode_encode_error(codec, offset);
	} else {
		ASSERTF(status == -1, "The used printer 'Dee_bytes_printer_append' "
		                      "should only ever return `-1' to indicate errors");
	}
	goto err_printer;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
iconv_do_transcode(void const *data_base, size_t data_size,
                   iconv_codec_t incodec, iconv_codec_t outcodec,
                   uintptr_half_t inflags, uintptr_half_t outflags) {
	Dee_ssize_t status;
	struct iconv_decode decoder;
	struct iconv_encode encoder;
	struct iconv_printer input_printer;
	struct Dee_bytes_printer out_printer;
	Dee_bytes_printer_init_ex(&out_printer, data_size);

	encoder.ice_output.ii_printer = (Dee_formatprinter_t)&Dee_bytes_printer_append;
	encoder.ice_output.ii_arg     = &out_printer;
	encoder.ice_flags             = outflags;
	encoder.ice_codec             = outcodec;
	decoder.icd_flags             = inflags;
	decoder.icd_codec             = incodec;

	/* Check for special case: when input and output  codecs are the same,  then
	 *                         it really shouldn't matter if we don't know them! */
	if (decoder.icd_codec == encoder.ice_codec) {
		input_printer = encoder.ice_output;
		encoder.ice_codec = CODEC_UNKNOWN;
	} else {
		/* Initialize the encoder and set-up its input pipe for use as output by the decoder. */
		if unlikely(do_libiconv_encode_init(&encoder, &decoder.icd_output))
			goto no_such_out_codec;

		/* Initialize the decoder (note that it's output printer was already set-up
		 * as the input descriptor for the  encode function in the previous  step!) */
		if (do_libiconv_decode_init(&decoder, &input_printer))
			goto no_such_in_codec;

		/* And that's already it! */
	}
	status = (*input_printer.ii_printer)(input_printer.ii_arg, (char const *)data_base, data_size);
	if unlikely(status < 0)
		goto maybe_handle_iconv_error;
#if 0
	if (!libiconv_decode_isshiftzero(&decoder)) {
		/* Could potentially throw an error here, but don't... */
	}
	if (!libiconv_encode_isinputshiftzero(&encoder)) {
		/* Could potentially throw an error here, but don't... */
	}
#endif
	if unlikely(libiconv_encode_flush(&encoder) < 0)
		goto err_printer;
	return Dee_bytes_printer_pack(&out_printer);
no_such_in_codec:
	err_unknown_codec(incodec);
	goto err_printer;
no_such_out_codec:
	err_unknown_codec(outcodec);
err_printer:
	Dee_bytes_printer_fini(&out_printer);
/*err:*/
	return NULL;
maybe_handle_iconv_error:
	if (decoder.icd_flags & ICONV_HASERR) {
		size_t offset = (size_t)(data_size + status);
		err_unicode_decode_error(incodec, offset);
	} else if (encoder.ice_flags & ICONV_HASERR) {
		err_unicode_reencode_error(outcodec);
	} else {
		ASSERTF(status == -1, "The used printer 'Dee_bytes_printer_append' "
		                      "should only ever return `-1' to indicate errors");
	}
	goto err_printer;
}

/*[[[deemon (print_KwCMethod from rt.gen.unpack)("decode", """
	data:?X2?Dstring?DBytes,
	codec:?X2?Dstring?Dint,
	errors:?X2?Dstring?Dint = NULL = !Pstrict
""", libname: "deemon_iconv");]]]*/
#define deemon_iconv_decode_params "data:?X2?Dstring?DBytes,codec:?X2?Dstring?Dint,errors:?X2?Dstring?Dint=!Pstrict"
FORCELOCAL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL deemon_iconv_decode_f_impl(DeeObject *data, DeeObject *codec, DeeObject *errors);
#ifndef DEFINED_kwlist__data_codec_errors
#define DEFINED_kwlist__data_codec_errors
PRIVATE DEFINE_KWLIST(kwlist__data_codec_errors, { KEX("data", 0x3af4b6d3, 0xb0164401a9853128), KEX("codec", 0x91dfc790, 0x678d4474a4f58564), KEX("errors", 0xd327c5ea, 0x88b9782b6de95122), KEND });
#endif /* !DEFINED_kwlist__data_codec_errors */
PRIVATE WUNUSED DREF DeeObject *DCALL deemon_iconv_decode_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *data;
		DeeObject *codec;
		DeeObject *errors;
	} args;
	args.errors = NULL;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__data_codec_errors, "oo|o:decode", &args))
		goto err;
	return deemon_iconv_decode_f_impl(args.data, args.codec, args.errors);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(deemon_iconv_decode, &deemon_iconv_decode_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL deemon_iconv_decode_f_impl(DeeObject *data, DeeObject *codec, DeeObject *errors)
/*[[[end]]]*/
{
	iconv_codec_t codec_id;
	uintptr_half_t flags;
	void const *data_base;
	size_t data_size;
	if (DeeBytes_Check(data)) {
		data_base = DeeBytes_DATA(data);
		data_size = DeeBytes_SIZE(data);
	} else if (DeeString_Check(data)) {
		data_base = DeeString_AsBytes(data, false);
		if unlikely(!data_base)
			goto err;
		data_size = WSTR_LENGTH(data_base);
	} else {
		DeeObject_TypeAssertFailed2(data, &DeeBytes_Type, &DeeString_Type);
		goto err;
	}
	codec_id = deemon_iconv_parse_codec_name_and_error_mode(codec, errors, &flags);
	if unlikely(codec_id == ICONV_CODEC_UNKNOWN)
		goto err;
	return iconv_do_decode(data_base, data_size, codec_id, flags);
err:
	return NULL;
}


/*[[[deemon (print_KwCMethod from rt.gen.unpack)("encode", """
	data:?X2?Dstring?DBytes,
	codec:?X2?Dstring?Dint,
	errors:?X2?Dstring?Dint = NULL = !Pstrict
""", libname: "deemon_iconv");]]]*/
#define deemon_iconv_encode_params "data:?X2?Dstring?DBytes,codec:?X2?Dstring?Dint,errors:?X2?Dstring?Dint=!Pstrict"
FORCELOCAL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL deemon_iconv_encode_f_impl(DeeObject *data, DeeObject *codec, DeeObject *errors);
#ifndef DEFINED_kwlist__data_codec_errors
#define DEFINED_kwlist__data_codec_errors
PRIVATE DEFINE_KWLIST(kwlist__data_codec_errors, { KEX("data", 0x3af4b6d3, 0xb0164401a9853128), KEX("codec", 0x91dfc790, 0x678d4474a4f58564), KEX("errors", 0xd327c5ea, 0x88b9782b6de95122), KEND });
#endif /* !DEFINED_kwlist__data_codec_errors */
PRIVATE WUNUSED DREF DeeObject *DCALL deemon_iconv_encode_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *data;
		DeeObject *codec;
		DeeObject *errors;
	} args;
	args.errors = NULL;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__data_codec_errors, "oo|o:encode", &args))
		goto err;
	return deemon_iconv_encode_f_impl(args.data, args.codec, args.errors);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(deemon_iconv_encode, &deemon_iconv_encode_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL deemon_iconv_encode_f_impl(DeeObject *data, DeeObject *codec, DeeObject *errors)
/*[[[end]]]*/
{
	iconv_codec_t codec_id;
	uintptr_half_t flags;
	void const *data_base;
	size_t data_size;
	if (DeeBytes_Check(data)) {
		data_base = DeeBytes_DATA(data);
		data_size = DeeBytes_SIZE(data);
	} else if (DeeString_Check(data)) {
		data_base = DeeString_AsUtf8(data);
		if unlikely(!data_base)
			goto err;
		data_size = WSTR_LENGTH(data_base);
	} else {
		DeeObject_TypeAssertFailed2(data, &DeeBytes_Type, &DeeString_Type);
		goto err;
	}
	codec_id = deemon_iconv_parse_codec_name_and_error_mode(codec, errors, &flags);
	if unlikely(codec_id == ICONV_CODEC_UNKNOWN)
		goto err;
	return iconv_do_encode(data_base, data_size, codec_id, flags);
err:
	return NULL;
}


/*[[[deemon (print_KwCMethod from rt.gen.unpack)("transcode", """
	data:?X2?Dstring?DBytes,
	incodec:?X2?Dstring?Dint,
	outcodec:?X2?Dstring?Dint,
	errors:?X2?Dstring?Dint = NULL = !Pstrict
""", libname: "deemon_iconv");]]]*/
#define deemon_iconv_transcode_params "data:?X2?Dstring?DBytes,incodec:?X2?Dstring?Dint,outcodec:?X2?Dstring?Dint,errors:?X2?Dstring?Dint=!Pstrict"
FORCELOCAL WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL deemon_iconv_transcode_f_impl(DeeObject *data, DeeObject *incodec, DeeObject *outcodec, DeeObject *errors);
#ifndef DEFINED_kwlist__data_incodec_outcodec_errors
#define DEFINED_kwlist__data_incodec_outcodec_errors
PRIVATE DEFINE_KWLIST(kwlist__data_incodec_outcodec_errors, { KEX("data", 0x3af4b6d3, 0xb0164401a9853128), KEX("incodec", 0x556c3790, 0x67f1690ff39bd316), KEX("outcodec", 0x2f41262c, 0xfb14bcb3b213f6d6), KEX("errors", 0xd327c5ea, 0x88b9782b6de95122), KEND });
#endif /* !DEFINED_kwlist__data_incodec_outcodec_errors */
PRIVATE WUNUSED DREF DeeObject *DCALL deemon_iconv_transcode_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *data;
		DeeObject *incodec;
		DeeObject *outcodec;
		DeeObject *errors;
	} args;
	args.errors = NULL;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__data_incodec_outcodec_errors, "ooo|o:transcode", &args))
		goto err;
	return deemon_iconv_transcode_f_impl(args.data, args.incodec, args.outcodec, args.errors);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(deemon_iconv_transcode, &deemon_iconv_transcode_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL deemon_iconv_transcode_f_impl(DeeObject *data, DeeObject *incodec, DeeObject *outcodec, DeeObject *errors)
/*[[[end]]]*/
{
	iconv_codec_t in_codec_id, out_codec_id;
	uintptr_half_t in_flags, out_flags;
	void const *data_base;
	size_t data_size;
	if (DeeBytes_Check(data)) {
		data_base = DeeBytes_DATA(data);
		data_size = DeeBytes_SIZE(data);
	} else if (DeeString_Check(data)) {
		data_base = DeeString_AsBytes(data, false);
		if unlikely(!data_base)
			goto err;
		data_size = WSTR_LENGTH(data_base);
	} else {
		DeeObject_TypeAssertFailed2(data, &DeeBytes_Type, &DeeString_Type);
		goto err;
	}
	in_codec_id = deemon_iconv_parse_codec_name_and_error_mode(incodec, errors, &in_flags);
	if unlikely(in_codec_id == ICONV_CODEC_UNKNOWN)
		goto err;
	out_codec_id = deemon_iconv_parse_codec_name_and_error_mode(outcodec, errors, &out_flags);
	if unlikely(out_codec_id == ICONV_CODEC_UNKNOWN)
		goto err;
	return iconv_do_transcode(data_base, data_size,
	                          in_codec_id, out_codec_id,
	                          in_flags, out_flags);
err:
	return NULL;
}

#define DOC_param_errors "#perrors{One of $\"strict\", $\"replace\", $\"ignore\" or $\"discard\"}"

DEX_BEGIN
DEX_MEMBER_F("codecbyname", &deemon_iconv_codecbyname, Dee_DEXSYM_READONLY,
             "(" deemon_iconv_codecbyname_params ")->?X2?Dint?N\n"
             "#r{Internal codec ID (s.a. ?Ggetcodecnames), or ?N when unrecognized}"
             "Return the internal ID of the codec associated with @name. "
             /**/ "Casing is ignored, codec aliases (s.a. ?Ggetcodecnames) are respected, "
             /**/ "and @name is normalized (s.a. ?Gnormalizecodecname) prior to being "
             /**/ "searched-for within the database.\n"
             "NOTE: Other API functions like ?Gencode / ?Gdecode don't actually "
             /**/ "use this function to convert codec names into IDs, but rather "
             /**/ "use ?Gparsecodecname"),

DEX_MEMBER_F("normalizecodecname", &deemon_iconv_normalizecodecname, Dee_DEXSYM_READONLY,
             "(" deemon_iconv_normalizecodecname_params ")->?X2?Dstring?N\n"
             "Normalize a given codec @name. This function is used by ?Gcodecbyname to "
             /**/ "simplify some more-complex codec names such that they are recognized "
             /**/ "as known codecs more easily. If the resulting name would be too long "
             /**/ "to still be a valid codec, return ?N instead.\n"
             "${"
             /**/ "function normalizecodecname(name: string): string | none {\n"
             /**/ "	name = name.strip();\n"
             /**/ "	local result = File.Writer(hint: \"string\");\n"
             /**/ "	for (local pfx: {\"oem\", \"ibm\", \"iso\", \"cp\", \"latin\", \"koi\", \"l\"}) {\n"
             /**/ "		if (name.casestartswith(pfx) && ##name >= (##pfx + 2) &&\n"
             /**/ "		    name[##pfx] in \"-_ \" && name.isdigit(##pfx + 1)) {\n"
             /**/ "			result << pfx;\n"
             /**/ "			name = name[##pfx + 1:];\n"
             /**/ "			break;\n"
             /**/ "		}\n"
             /**/ "	}\n"
             /**/ "	for (local i: [##name]) {\n"
             /**/ "		local ch = name[i];\n"
             /**/ "		if (ch == \"0\" && ((i + 1) < ##name) && name.isdigit(i + 1) &&\n"
             /**/ "		    !result.string || !result.string.last.isdigit()) \n"
             /**/ "			continue; /* Skip leading 0s in number-strings. */\n"
             /**/ "		if (ch in \"_ \")\n"
             /**/ "			ch = \"-\"; /* '-', '_' and ' ' work interchangeably. */\n"
             /**/ "		result << ch;\n"
             /**/ "	}\n"
             /**/ "	result = result.string;\n"
             /**/ "	/* CODE_NAME_MAXLEN = max(FOREACH_CODEC(getcodecnames(codec).each.length)) */\n"
             /**/ "	if (##result > CODE_NAME_MAXLEN)\n"
             /**/ "		return none;\n"
             /**/ "	return result;\n"
             /**/ "}"
             "}"),

DEX_MEMBER_F("getcodecnames", &deemon_iconv_getcodecnames, Dee_DEXSYM_READONLY,
             "(" deemon_iconv_getcodecnames_params ")->?X2?S?Dstring?N\n"
             "Return the list of names for the codec @id. The list is "
             /**/ "sorted such that the most important name comes first.\n"
             "When @id is invalid, return ?N. Note that all valid codecs have at "
             /**/ "least 1 valid name. As such, supported codecs as well as their "
             /**/ "names can be enumerated by using ?GICONV_CODEC_FIRST as followed:\n"
             "${"
             /**/ "for (local id = ICONV_CODEC_FIRST;; ++id) {\n"
             /**/ "	local names = getcodecnames(id);\n"
             /**/ "	if (names is none)\n"
             /**/ "		break;\n"
             /**/ "	print \"\\t\".join(names);\n"
             /**/ "}"
             "}"),
ICONV_ICONV_CODEC_UNKNOWN_DEF
ICONV_ICONV_CODEC_FIRST_DEF

DEX_MEMBER_F("parsecodecname", &deemon_iconv_parsecodecname, Dee_DEXSYM_READONLY,
             "(" deemon_iconv_parsecodecname_params ")->?X2?T2?Dint?Dint?N\n"
             "#r{Pair of #C{codec-id}, #C{coded-flags}. The later can "
             /**/ "be used as the #Cerrors argument of other functions}"
             "Same as ?Gcodecbyname, but also parse possible flag-relation options. "
             /**/ "These flags can then be used as the #Cerrors argument of ?Gencode/"
             /**/ "?Gdecode or the associated ?GEncoder/?GDecoder. Flags may appear "
             /**/ "as a suffix in codec names. The following flags are defined:\n"
             "#T{Flag|Effect~"
             /**/ "$\"//IGNORE\"|Override the #Cerrors argument as $\"discard\". "
             /**/ /*         */ "Useful as the builtin ?Aencode?Dstring/?Adecode?Dstring "
             /**/ /*         */ "functions only accept $\"strict\", $\"replace\" and $\"ignore\" "
             /**/ /*         */ "as valid error handlers&"
             /**/ "$\"//TRANSLIT\"|Only meaningful for ?Gencode and ?GEncoder: make use "
             /**/ /*           */ "of ?G_transliterate to replace unicode literals that "
             /**/ /*           */ "cannot be represented in the output codec with some "
             /**/ /*           */ "other (sequence of) characters that can, whilst having "
             /**/ /*           */ "a similar appearance/meaning. Note that this is always "
             /**/ /*           */ "a #Ilossy operation, meaning that ?G{decode}ing one "
             /**/ /*           */ "such ?G{_transliterate}-encoded may not re-produce the "
             /**/ /*           */ "originally encoded unicode string."
             "}"),
DEX_MEMBER_F("samecodec", &deemon_iconv_samecodec, Dee_DEXSYM_READONLY,
             "(" deemon_iconv_samecodec_params ")->?Dbool\n"
             "Check if the 2 given strings reference the same codec name.\n"
             "This differs from same codec ID as this function doesn't actually "
             /**/ "search the codec database but will simply strip potential flags, "
             /**/ "normalize the underlying codec names, and check if the resulting "
             /**/ "strings ?Acasecompare?Dstring to be equal."),

DEX_MEMBER_F("detect", &deemon_iconv_detect, Dee_DEXSYM_READONLY,
             "(" deemon_iconv_detect_params ")->?X2?Dint?N\n"
             "Try to automatically detect the codec of the given data-blob, which should "
             /**/ "represent the memory-mapping of a text-file. This function will then try to "
             /**/ "inspect its beginning for comment-style indicators which might inform about "
             /**/ "which codec the file uses (e.g. xml, python, etc.), as well as analysis of "
             /**/ "NUL-bytes for multi-byte codecs.\n"

             "In case of a single-byte codec, go through all bytes that appear in the file "
             /**/ "and count which of them occur how often before narrowing down candidates by "
             /**/ "excluding any where decoding would result in non-printable characters other "
             /**/ "than those needed for text (i.e. line-feeds, spaces, and unicode prefixes).\n"

             "Once the set of codecs capable of decoding the file into something that looks "
             /**/ "like text is determined, use each of them to try and decode the text to UTF-8 "
             /**/ "and count how often each bytes occurs within the UTF-8 stream. The results of "
             /**/ "this are then fuzzy-compared against a known-good heuristic of byte usage in "
             /**/ "normal text, and the codec which is closest to this heuristic is used.\n"

             "If the function is unable to determine the codec to-be used, it will return ?N"),

DEX_MEMBER_F_NODOC("Encoder", &IconvEncoder_Type.ft_base, Dee_DEXSYM_READONLY),
DEX_MEMBER_F_NODOC("EncodeWriter", &IconvEncoder_Type.ft_base, Dee_DEXSYM_READONLY), /* Alias for "Encoder" */
DEX_MEMBER_F_NODOC("DecodeWriter", &IconvDecodeWriter_Type.ft_base, Dee_DEXSYM_READONLY),
DEX_MEMBER_F_NODOC("TranscodeWriter", &IconvTranscodeWriter_Type.ft_base, Dee_DEXSYM_READONLY),

/* TODO: Decoder */
/* TODO: DecodeReader */ /* Alias for "Decoder" */
/* TODO: EncodeReader */
/* TODO: TranscodeReader */

/* TODO: DecodeReader -- Similar to Decoder, but instead of requiring you to *write* encoded
 *                       data to have its decoded equivalent be written to another file, this
 *                       one let's you to specify a file to read from, such that reading from
 *                       the "DecodeReader" wrapper file will return decoded data:
 * >> @@Open a file for reading, auto-detect its encoding, and return a file
 * >> @@that allows you to read the file contents as though they were utf-8.
 * >> function readFileAutoDetect(filename: string): File {
 * >>     local fp = File.open(filename, "rb");
 * >>     local head = fp.read(4096);
 * >>     local codec = iconv.detect(head) ?? "utf-8";
 * >>     fp.rewind();
 * >>     return iconv.DecodeReader(codec, fp);
 * >> }
 *
 * Also add "EncodeReader" and "TranscodeReader"
 *
 * XXX: I don't really like those class names; maybe come up with some better name?
 */

/* Fast-pass encode/decode functions (drop-in replacements for equivalents from "codec") */
DEX_MEMBER_F("decode", &deemon_iconv_decode, Dee_DEXSYM_READONLY,
             "(" deemon_iconv_decode_params ")->?Dstring\n"
             "#pcodec{Name of the codec that was used to encode @data. May also "
             /*   */ "contain some additional flags (s.a. ?Gparsecodecname)}"
             DOC_param_errors
             "Convenience wrapper around ?GDecoder:\n"
             "${"
             /**/ "function decode(data: string | Bytes, codec: string,\n"
             /**/ "                errors: string = \"strict\"): string {\n"
             /**/ "	local out = import.deemon.File.Writer(hint: \"string\");\n"
             /**/ "	with (local decoder = iconv.DecodeWriter(codec, out, errors))\n"
             /**/ "		decoder.write(data);\n"
             /**/ "	return out.string;\n"
             /**/ "}"
             "}"),
DEX_MEMBER_F("encode", &deemon_iconv_encode, Dee_DEXSYM_READONLY,
             "(" deemon_iconv_encode_params ")->?DBytes\n"
             "#pcodec{Name of the codec that should be used to encode @data. May "
             /*   */ "also contain some additional flags (s.a. ?Gparsecodecname)}"
             DOC_param_errors
             "Convenience wrapper around ?GEncoder:\n"
             "${"
             /**/ "function encode(data: string | Bytes, codec: string,\n"
             /**/ "                errors: string = \"strict\"): Bytes {\n"
             /**/ "	local out = import.deemon.File.Writer(hint: \"bytes\");\n"
             /**/ "	with (local encoder = iconv.Encoder(codec, out, errors)) {\n"
             /**/ "		if (data is string) {\n"
             /**/ "			print encoder: data,;\n"
             /**/ "		} else {\n"
             /**/ "			encoder.write(data);\n"
             /**/ "		}\n"
             /**/ "	}\n"
             /**/ "	return out.bytes;\n"
             /**/ "}"
             "}"),
DEX_MEMBER_F("transcode", &deemon_iconv_transcode, Dee_DEXSYM_READONLY,
             "(" deemon_iconv_transcode_params ")->?DBytes\n"
             "#pincodec{Name of the codec that was used to encode @data. May also "
             /*     */ "contain some additional flags (s.a. ?Gparsecodecname)}"
             "#poutcodec{Name of the codec that should be used to re-encode @data "
             /*      */ "after it was internally decoded using @incodec. May also "
             /*      */ "contain some additional flags (s.a. ?Gparsecodecname)}"
             DOC_param_errors
             "Convenience wrapper around ?GTranscoder:\n"
             "${"
             /**/ "function transcode(data: string | Bytes,\n"
             /**/ "                   incodec: string, outcodec: string,\n"
             /**/ "                   errors: string = \"strict\"): Bytes {\n"
             /**/ "	local out = import.deemon.File.Writer(hint: \"bytes\");\n"
             /**/ "	with (local transcoder = iconv.TranscodeWriter(incodec, outcodec, out, errors)) {\n"
             /**/ "		if (data is string) {\n"
             /**/ "			print transcoder: data,;\n"
             /**/ "		} else {\n"
             /**/ "			transcoder.write(data);\n"
             /**/ "		}\n"
             /**/ "	}\n"
             /**/ "	return out.bytes;\n"
             /**/ "}"
             "}"),


/* TODO: >> function transliterate(ord: int, what: int = TRANSLITERATE_ALL): {string...} {
 *       >>     for (local nth = 0;; ++nth) {
 *       >>         local result = _transliterate(ord, nth, what);
 *       >>         if (result is none)
 *       >>             break;
 *       >>         yield result;
 *       >>     }
 *       >> } */

DEX_MEMBER_F("_transliterate", &deemon_iconv__transliterate, Dee_DEXSYM_READONLY,
             "(" deemon_iconv__transliterate_params ")->?X2?Dstring?N\n"
             "#pord{The character that should be transliterated}"
             "#pnth{Specifies that the nth transliteration of @ord be generated, "
             /**/ "starting at $0 and ending as soon as this function returns ?N "
             /**/ "to indicate that no more possible transliterations are available}"
             "#pwhat{What to try (set of #C{TRANSLITERATE_*})}"
             "#r{The @nth transliteration of @ord, or ?N if there are no more}"
             "Generate and return the @nth transliteration for @ord. When no (more) "
             /**/ "transliterations exist for @ord (where available ones are indexed "
             /**/ "via @nth, starting at $0), return ?N.\n"
             "Note that in the case of multi-character transliterations, all possible "
             /**/ "transliterations for replacement characters are already attempted by "
             /**/ "this function itself, meaning that in these cases all those are all "
             /**/ "tried as well.\n"
             "Note that this function may or may not re-return ${string.chr(ord)}. When "
             /**/ "this is the case, simply ignore the call any try again with ${nth + 1}"),


ICONV_TRANSLITERATE_LOWER_DEF
ICONV_TRANSLITERATE_UPPER_DEF
ICONV_TRANSLITERATE_TITLE_DEF
ICONV_TRANSLITERATE_FOLD_DEF
ICONV_TRANSLITERATE_TRANSLIT_DEF
ICONV_TRANSLITERATE_ALL_DEF

/* Internal types */
DEX_MEMBER_F_NODOC("_StrNulArray", &StrNulArray_Type, Dee_DEXSYM_READONLY),
DEX_MEMBER_F_NODOC("_StrNulArrayIterator", &StrNulArrayIterator_Type, Dee_DEXSYM_READONLY),

DEX_END(NULL, NULL, NULL);

DECL_END


#endif /* !GUARD_DEX_ICONV_LIBICONV_C */
