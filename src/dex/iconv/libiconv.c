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

#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/bytes.h>
#include <deemon/dex.h> /* DEX_*, Dee_DEXSYM_READONLY */
#include <deemon/seq.h>
#include <deemon/error.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/objmethod.h>
#include <deemon/string.h>
#include <deemon/tuple.h>

DECL_BEGIN

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

/*[[[deemon (print_CMethod from rt.gen.unpack)("codec_and_flags_byname", """
	DeeStringObject *name
""", libname: "deemon_iconv");]]]*/
#define deemon_iconv_codec_and_flags_byname_params "name:?Dstring"
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL deemon_iconv_codec_and_flags_byname_f_impl(DeeStringObject *name);
PRIVATE DEFINE_CMETHOD1(deemon_iconv_codec_and_flags_byname, &deemon_iconv_codec_and_flags_byname_f_impl, METHOD_FNORMAL);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL deemon_iconv_codec_and_flags_byname_f_impl(DeeStringObject *name)
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

/*[[[deemon (print_CMethod from rt.gen.unpack)("same_codec_name", """
	DeeStringObject *name1,
	DeeStringObject *name2,
""", libname: "deemon_iconv");]]]*/
#define deemon_iconv_same_codec_name_params "name1:?Dstring,name2:?Dstring"
FORCELOCAL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL deemon_iconv_same_codec_name_f_impl(DeeStringObject *name1, DeeStringObject *name2);
PRIVATE WUNUSED DREF DeeObject *DCALL deemon_iconv_same_codec_name_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeStringObject *name1;
		DeeStringObject *name2;
	} args;
	DeeArg_UnpackStruct2(err, argc, argv, "same_codec_name", &args, &args.name1, &args.name2);
	return deemon_iconv_same_codec_name_f_impl(args.name1, args.name2);
err:
	return NULL;
}
PRIVATE DEFINE_CMETHOD(deemon_iconv_same_codec_name, &deemon_iconv_same_codec_name_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL deemon_iconv_same_codec_name_f_impl(DeeStringObject *name1, DeeStringObject *name2)
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

/*[[[deemon (print_CMethod from rt.gen.unpack)("detect_codec", """
	data:?X2?DBytes?Dstring
""", libname: "deemon_iconv");]]]*/
#define deemon_iconv_detect_codec_params "data:?X2?DBytes?Dstring"
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL deemon_iconv_detect_codec_f_impl(DeeObject *data);
PRIVATE DEFINE_CMETHOD1(deemon_iconv_detect_codec, &deemon_iconv_detect_codec_f_impl, METHOD_FNORMAL);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL deemon_iconv_detect_codec_f_impl(DeeObject *data)
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
	unsigned int what:?Dint = ICONV_TRANSLITERATE_F_ALL =!GTRANSLITERATE_ALL
""", libname: "deemon_iconv");]]]*/
#define deemon_iconv__transliterate_params "ord:?Dint,nth:?Dint,what:?Dint=!GTRANSLITERATE_ALL"
FORCELOCAL WUNUSED DREF DeeObject *DCALL deemon_iconv__transliterate_f_impl(uint32_t ord, size_t nth, unsigned int what);
PRIVATE WUNUSED DREF DeeObject *DCALL deemon_iconv__transliterate_f(size_t argc, DeeObject *const *argv) {
	struct {
		uint32_t ord;
		size_t nth;
		unsigned int what;
	} args;
	args.what = ICONV_TRANSLITERATE_F_ALL;
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


DEX_BEGIN
DEX_MEMBER_F("codecbyname", &deemon_iconv_codecbyname, Dee_DEXSYM_READONLY,
             "(" deemon_iconv_codecbyname_params ")->?X2?Dint?N\n"
             "#r{Internal codec ID (s.a. ?Ggetcodecnames), or !N when unrecognized}"
             "Return the internal ID of the codec associated with `name'. "
             /**/ "Casing is ignored and codec aliases are respected."),

DEX_MEMBER_F("getcodecnames", &deemon_iconv_getcodecnames, Dee_DEXSYM_READONLY,
             "(" deemon_iconv_getcodecnames_params ")->?X2?S?Dstring?N\n"
             "Return the list of names for the codec @id. The list is "
             /**/ "sorted such that the most important name comes first.\n"
             "When @id is invalid, return !N. Note that all valid codecs have at "
             /**/ "least 1 valid name. As such, supported codecs as well as their names can "
             /**/ "be enumerated by using ?GICONV_CODEC_FIRST as followed:\n"
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

DEX_MEMBER_F("codec_and_flags_byname", &deemon_iconv_codec_and_flags_byname, Dee_DEXSYM_READONLY,
             "(" deemon_iconv_codec_and_flags_byname_params ")->?X2?T2?Dint?Dint?N\n"
             "Same as ?Gcodecbyname, but also parse possible flag-relation options."),
DEX_MEMBER_F("same_codec_name", &deemon_iconv_same_codec_name, Dee_DEXSYM_READONLY,
             "(" deemon_iconv_same_codec_name_params ")->?Dbool\n"
             "Check if the 2 given strings reference the same codec name.\n"
             "This differs from same codec ID as this function doesn't actually "
             /**/ "search the codec database but will simply strip potential flags, "
             /**/ "normalize the underlying codec names, and check if the resulting "
             /**/ "strings ?Acasecompare?Dstring to be equal."),

DEX_MEMBER_F("detect_codec", &deemon_iconv_detect_codec, Dee_DEXSYM_READONLY,
             "(" deemon_iconv_detect_codec_params ")->?X2?Dint?N\n"
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

             "If the function is unable to determine the codec to-be used, it will return !N"),

/* TODO: "class Decoder: File from deemon { ... }"    -- _libiconv_decode_init */
/* TODO: "class Encoder: File from deemon { ... }"    -- _libiconv_encode_init */
/* TODO: "class Transcoder: File from deemon { ... }" -- _libiconv_transcode_init */

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
             /**/ "starting at $0 and ending as soon as this function returns !N "
             /**/ "to indicate that no more possible transliterations are available}"
             "#pwhat{What to try (set of #C{TRANSLITERATE_*})}"
             "#r{The @nth transliteration of @ord, or !N if there are no more}"
             "Generate and return the @nth transliteration for @ord. When no (more) "
             /**/ "transliterations exist for @ord (where available ones are indexed "
             /**/ "via @nth, starting at $0), return !N.\n"
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
