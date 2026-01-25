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
#ifndef GUARD_DEX_JSON_LIBJSON_C
#define GUARD_DEX_JSON_LIBJSON_C 1
#define CONFIG_BUILDING_LIBWIN32
#define DEE_SOURCE

#include "libjson.h" /* Must be first to configure libjson */
/**/

#include <deemon/api.h>

#include <deemon/alloc.h>           /* DeeObject_FREE, DeeObject_MALLOC, Dee_TYPE_CONSTRUCTOR_INIT_FIXED */
#include <deemon/arg.h>             /* DEFINE_KWLIST, DeeArg_Unpack1, DeeArg_UnpackStructKw */
#include <deemon/bool.h>
#include <deemon/bytes.h>
#include <deemon/class.h>
#include <deemon/dex.h>
#include <deemon/error-rt.h>
#include <deemon/error.h>
#include <deemon/file.h>
#include <deemon/float.h>
#include <deemon/format.h>
#include <deemon/hashset.h>
#include <deemon/int.h>
#include <deemon/list.h>
#include <deemon/map.h>
#include <deemon/module.h>
#include <deemon/mro.h>
#include <deemon/none.h>
#include <deemon/numeric.h>
#include <deemon/object.h>
#include <deemon/objmethod.h>
#include <deemon/seq.h>
#include <deemon/serial.h>
#include <deemon/set.h>
#include <deemon/string.h>
#include <deemon/system-features.h>
#include <deemon/tuple.h>
#include <deemon/util/atomic.h>     /* atomic_cmpxch_or_write, atomic_read */
#include <deemon/util/lock.h>       /* Dee_atomic_rwlock_init */
#include <deemon/util/objectlist.h> /* objectlist, objectlist_* */

#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* NULL, offsetof, size_t */
#include <stdint.h>  /* intptr_t, uint8_t, uint16_t, uint32_t */
/**/

/* Enable KOS compatibility emulation */
#include <deemon/util/kos-compat.h>

/* Extra includes needed by libjson */
/* clang-format off */
#include <hybrid/byteorder.h>
#include <hybrid/byteswap.h>
#include <hybrid/unaligned.h>
/* clang-format on */

/* Define everything with PRIVATE scoping. */
#undef INTDEF
#undef INTERN
#undef DEFINE_PUBLIC_ALIAS
#define INTDEF PRIVATE
#define INTERN PRIVATE
#define DEFINE_PUBLIC_ALIAS(new, old) /* Disable exports */

/* clang-format off */
#include "../../libjson/include/parser.h" /* 1 */
#include "../../libjson/include/writer.h" /* 2 */
#include "../../libjson/parser.c"         /* 3 */
#include "../../libjson/writer.c"         /* 4 */
/* clang-format on */

/* Restore normal binding macros */
#undef INTDEF
#undef INTERN
#undef DEFINE_PUBLIC_ALIAS
#define INTDEF __INTDEF
#define INTERN __INTERN
#define DEFINE_PUBLIC_ALIAS __DEFINE_PUBLIC_ALIAS

DECL_BEGIN

PRIVATE char const empty_json_array[] = "[]";
PRIVATE char const empty_json_object[] = "{}";

/* Implementation of JSON sequence proxies. */

PRIVATE ATTR_COLD int DCALL err_json_syntax(void) {
	return DeeError_Throwf(&DeeError_ValueError, "JSON syntax error");
}

PRIVATE ATTR_COLD NONNULL((1, 2)) int DCALL
err_json_cannot_parse_type_as_type(DeeTypeObject *native_json_type,
                                   DeeTypeObject *into_type) {
	return DeeError_Throwf(&DeeError_TypeError,
	                       "Cannot parse instance of `%k' into `%k'",
	                       native_json_type, into_type);
}



PRIVATE WUNUSED NONNULL((1)) int DCALL
jseqiter_ctor(DeeJsonIteratorObject *__restrict self) {
	self->ji_owner              = DeeNone_NewRef();
	self->ji_parser.jp_encoding = JSON_ENCODING_UTF8;
	self->ji_parser.jp_start    = empty_json_array;
	self->ji_parser.jp_pos      = empty_json_array + 1;
	self->ji_parser.jp_end      = empty_json_array + 2;
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
jmapiter_ctor(DeeJsonIteratorObject *__restrict self) {
	self->ji_owner              = DeeNone_NewRef();
	self->ji_parser.jp_encoding = JSON_ENCODING_UTF8;
	self->ji_parser.jp_start    = empty_json_object;
	self->ji_parser.jp_pos      = empty_json_object + 1;
	self->ji_parser.jp_end      = empty_json_object + 2;
	return 0;
}

#define jseqiter_copy jiter_copy
#define jmapiter_copy jiter_copy
#define jseqiter_deep jiter_copy
#define jmapiter_deep jiter_copy
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
jiter_copy(DeeJsonIteratorObject *__restrict self,
           DeeJsonIteratorObject *__restrict other) {
	self->ji_owner = other->ji_owner;
	Dee_Incref(self->ji_owner);
	DeeJsonIterator_GetParser(other, &self->ji_parser);
	return 0;
}

#define jseqiter_serialize jiter_serialize
#define jmapiter_serialize jiter_serialize
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
jiter_serialize(DeeJsonIteratorObject *__restrict self,
                DeeSerial *__restrict writer, Dee_seraddr_t addr) {
	DeeJsonIteratorObject *out;
#define ADDROF(field) (addr + offsetof(DeeJsonIteratorObject, field))
	out = DeeSerial_Addr2Mem(writer, addr, DeeJsonIteratorObject);
	out->ji_parser.jp_encoding = self->ji_parser.jp_encoding;
	if (DeeSerial_PutObject(writer, ADDROF(ji_owner), self->ji_owner))
		goto err;
	if (DeeSerial_XPutPointer(writer, ADDROF(ji_parser.jp_start), self->ji_parser.jp_start))
		goto err;
	if (DeeSerial_XPutPointer(writer, ADDROF(ji_parser.jp_end), self->ji_parser.jp_end))
		goto err;
	return DeeSerial_XPutPointer(writer, ADDROF(ji_parser.jp_pos), atomic_read(&self->ji_parser.jp_pos));
err:
	return -1;
#undef ADDROF
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
jseqiter_init(DeeJsonIteratorObject *__restrict self,
              size_t argc, DeeObject *const *argv) {
	DeeJsonSequenceObject *seq;
	DeeArg_Unpack1(err, argc, argv, "_JsonSequenceIterator", &seq);
	if (DeeObject_AssertTypeExact(seq, &DeeJsonSequence_Type))
		goto err;
	self->ji_owner = seq->js_owner;
	Dee_Incref(self->ji_owner);
	DeeJsonSequence_LockRead(seq);
	self->ji_parser = seq->js_parser;
	DeeJsonSequence_LockEndRead(seq);
	if unlikely(libjson_parser_rewind(&self->ji_parser) != JSON_PARSER_ARRAY)
		goto err_syntax;
	return 0;
err_syntax:
	err_json_syntax();
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
jmapiter_init(DeeJsonIteratorObject *__restrict self,
              size_t argc, DeeObject *const *argv) {
	DeeJsonMappingObject *seq;
	DeeArg_Unpack1(err, argc, argv, "_JsonMappingIterator", &seq);
	if (DeeObject_AssertTypeExact(seq, &DeeJsonMapping_Type))
		goto err;
	self->ji_owner = seq->jm_owner;
	Dee_Incref(self->ji_owner);
	DeeJsonMapping_LockRead(seq);
	self->ji_parser = seq->jm_parser;
	DeeJsonMapping_LockEndRead(seq);
	if unlikely(libjson_parser_rewind(&self->ji_parser) != JSON_PARSER_OBJECT)
		goto err_syntax;
	return 0;
err_syntax:
	err_json_syntax();
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
jseqiter_nii_peek(DeeJsonIteratorObject *__restrict self) {
	DeeJsonParser parser;
	DeeJsonIterator_GetParserEx(self, &parser);
	if (libjson_parser_peeknext(&parser.djp_parser) == JSON_PARSER_ENDARRAY)
		return ITER_DONE;
	return DeeJson_ParseObject(&parser, false);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
jmapiter_nii_peek(DeeJsonIteratorObject *__restrict self) {
	DREF DeeObject *key_and_value[2], *result;
	DeeJsonParser parser;
	DeeJsonIterator_GetParserEx(self, &parser);
	if (libjson_parser_peeknext(&parser.djp_parser) == JSON_PARSER_ENDARRAY)
		return ITER_DONE;
	key_and_value[0] = DeeJson_ParseString(&parser.djp_parser);
	if unlikely(!key_and_value[0])
		goto err;
	if (libjson_parser_yield(&parser.djp_parser) != JSON_PARSER_COLON)
		goto err_key_syntax;
	key_and_value[1] = DeeJson_ParseObject(&parser, false);
	if unlikely(!key_and_value[1])
		goto err_key;
	result = DeeTuple_NewVectorSymbolic(2, key_and_value);
	if unlikely(!result)
		goto err_key_value;
	return result;
err_key_value:
	Dee_Decref(key_and_value[1]);
	goto err_key;
err_key_syntax:
	err_json_syntax();
err_key:
	Dee_Decref_likely(key_and_value[0]);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
jseqiter_next(DeeJsonIteratorObject *__restrict self) {
	char const *orig_pos;
	DREF DeeObject *result;
	DeeJsonParser parser;
again:
	DeeJsonIterator_GetParserEx(self, &parser);
	orig_pos = parser.djp_parser.jp_pos;
	if (libjson_parser_peeknext(&parser.djp_parser) == JSON_PARSER_ENDARRAY)
		return ITER_DONE;
	result = DeeJson_ParseObject(&parser, true);
	if unlikely(!result)
		goto err;

	/* Parse the ',' token following the element. */
	{
		char const *saved_pos = parser.djp_parser.jp_pos;
		int tok = libjson_parser_yield(&parser.djp_parser);
		if (tok != JSON_PARSER_COMMA) {
			if (tok != JSON_PARSER_ENDARRAY)
				goto err_syntax_r;
			parser.djp_parser.jp_pos = saved_pos;
		}
	}

	/* Try to write-back the new parser position. */
	if (!atomic_cmpxch_or_write(&self->ji_parser.jp_pos, orig_pos,
	                            parser.djp_parser.jp_pos)) {
		Dee_Decref(result);
		goto again;
	}
	return result;
err_syntax_r:
	Dee_Decref(result);
/*err_syntax:*/
	err_json_syntax();
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
jmapiter_nextpair(DeeJsonIteratorObject *__restrict self,
                  DREF DeeObject *key_and_value[2]) {
	int tok;
	char const *orig_pos, *temp;
	DeeJsonParser parser;
again:
	DeeJsonIterator_GetParserEx(self, &parser);
	orig_pos = parser.djp_parser.jp_pos;
	if (libjson_parser_peeknext(&parser.djp_parser) == JSON_PARSER_ENDOBJECT)
		return 1;
	key_and_value[0] = DeeJson_ParseString(&parser.djp_parser);
	if unlikely(!key_and_value[0])
		goto err;
	if (libjson_parser_yield(&parser.djp_parser) != JSON_PARSER_COLON)
		goto err_key_syntax;
	key_and_value[1] = DeeJson_ParseObject(&parser, true);
	if unlikely(!key_and_value[1])
		goto err_key;

	/* Parse the trailing ',' that following the mapping value. */
	temp = parser.djp_parser.jp_pos;
	tok  = libjson_parser_yield(&parser.djp_parser);
	if (tok != JSON_PARSER_COMMA) {
		if (tok != JSON_PARSER_ENDOBJECT)
			goto err_key_value_syntax;
		parser.djp_parser.jp_pos = temp; /* Rewind to the start of the '}' token. */
	}

	/* Try to write-back the new parser position. */
	if (!atomic_cmpxch_or_write(&self->ji_parser.jp_pos, orig_pos,
	                            parser.djp_parser.jp_pos)) {
		Dee_Decref(key_and_value[0]);
		Dee_Decref(key_and_value[1]);
		goto again;
	}
	return 0;
err_key_value_syntax:
	err_json_syntax();
/*err_key_value:*/
	Dee_Decref(key_and_value[1]);
	goto err_key;
err_key_syntax:
	err_json_syntax();
err_key:
	Dee_Decref_likely(key_and_value[0]);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
jmapiter_nextkey(DeeJsonIteratorObject *__restrict self) {
	int tok;
	char const *orig_pos;
	DREF DeeObject *key;
	DeeJsonParser parser;
again:
	DeeJsonIterator_GetParserEx(self, &parser);
	orig_pos = parser.djp_parser.jp_pos;
	if (libjson_parser_peeknext(&parser.djp_parser) == JSON_PARSER_ENDARRAY)
		return ITER_DONE;
	key = DeeJson_ParseString(&parser.djp_parser);
	if unlikely(!key)
		goto err;
	if (libjson_parser_yield(&parser.djp_parser) != JSON_PARSER_COLON)
		goto err_key_syntax;
	tok = libjson_parser_next(&parser.djp_parser);
	if (tok == JSON_ERROR_SYNTAX)
		goto err_key_syntax;

	/* Try to write-back the new parser position. */
	if (!atomic_cmpxch_or_write(&self->ji_parser.jp_pos, orig_pos,
	                            parser.djp_parser.jp_pos)) {
		Dee_Decref(key);
		goto again;
	}
	return key;
err_key_syntax:
	err_json_syntax();
/*err_key:*/
	Dee_Decref_likely(key);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
jmapiter_nextvalue(DeeJsonIteratorObject *__restrict self) {
	int tok;
	char const *orig_pos, *temp;
	DREF DeeObject *value;
	DeeJsonParser parser;
again:
	DeeJsonIterator_GetParserEx(self, &parser);
	orig_pos = parser.djp_parser.jp_pos;
	if (libjson_parser_peeknext(&parser.djp_parser) == JSON_PARSER_ENDARRAY)
		return ITER_DONE;
	if (libjson_parser_yield(&parser.djp_parser) != JSON_PARSER_STRING)
		goto err_syntax;
	if (libjson_parser_yield(&parser.djp_parser) != JSON_PARSER_COLON)
		goto err_syntax;
	value = DeeJson_ParseObject(&parser, true);
	if unlikely(!value)
		goto err;

	/* Parse the trailing ',' that following the mapping value. */
	temp = parser.djp_parser.jp_pos;
	tok  = libjson_parser_yield(&parser.djp_parser);
	if (tok != JSON_PARSER_COMMA) {
		if (tok != JSON_PARSER_ENDOBJECT)
			goto err_syntax_value;
		parser.djp_parser.jp_pos = temp; /* Rewind to the start of the '}' token. */
	}

	/* Try to write-back the new parser position. */
	if (!atomic_cmpxch_or_write(&self->ji_parser.jp_pos, orig_pos,
	                            parser.djp_parser.jp_pos)) {
		Dee_Decref(value);
		goto again;
	}
	return value;
err_syntax_value:
	Dee_Decref(value);
err_syntax:
	err_json_syntax();
err:
	return NULL;
}

PRIVATE struct type_iterator jmapiter_iterator = {
	/* .tp_nextpair  = */ (int (DCALL *)(DeeObject *__restrict, DREF DeeObject *[2]))&jmapiter_nextpair,
	/* .tp_nextkey   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&jmapiter_nextkey,
	/* .tp_nextvalue = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&jmapiter_nextvalue,
	/* .tp_advance   = */ NULL, // TODO: (size_t (DCALL *)(DeeObject *__restrict, size_t))&jmapiter_advance,
};

PRIVATE WUNUSED NONNULL((1)) int DCALL
jseqiter_bool(DeeJsonIteratorObject *__restrict self) {
	int tok;
	struct json_parser parser;
	DeeJsonIterator_GetParser(self, &parser);
	tok = libjson_parser_peeknext(&parser);
	if (tok == JSON_PARSER_ENDARRAY)
		return 0;
	if unlikely(tok == JSON_ERROR_SYNTAX)
		return err_json_syntax();
	return 1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
jmapiter_bool(DeeJsonIteratorObject *__restrict self) {
	int tok;
	struct json_parser parser;
	DeeJsonIterator_GetParser(self, &parser);
	tok = libjson_parser_peeknext(&parser);
	if (tok == JSON_PARSER_ENDOBJECT)
		return 0;
	if unlikely(tok == JSON_ERROR_SYNTAX)
		return err_json_syntax();
	return 1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeJsonSequenceObject *DCALL
jseqiter_getseq(DeeJsonIteratorObject *__restrict self) {
	DREF DeeJsonSequenceObject *result;
	result = DeeObject_MALLOC(DeeJsonSequenceObject);
	if unlikely(!result)
		goto done;
	result->js_owner = self->ji_owner;
	Dee_Incref(result->js_owner);
	DeeJsonIterator_GetParser(self, &result->js_parser);
	if unlikely(libjson_parser_rewind(&result->js_parser) != JSON_PARSER_ARRAY)
		goto err_r_syntax;
	result->js_index = 0;
	result->js_size  = 0;
	Dee_atomic_rwlock_init(&result->js_lock);
	DeeObject_Init(result, &DeeJsonSequence_Type);
done:
	return result;
err_r_syntax:
	DeeObject_FREE(result);
	err_json_syntax();
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeJsonMappingObject *DCALL
jmapiter_getseq(DeeJsonIteratorObject *__restrict self) {
	DREF DeeJsonMappingObject *result;
	result = DeeObject_MALLOC(DeeJsonMappingObject);
	if unlikely(!result)
		goto done;
	result->jm_owner = self->ji_owner;
	Dee_Incref(result->jm_owner);
	DeeJsonIterator_GetParser(self, &result->jm_parser);
	if unlikely(libjson_parser_rewind(&result->jm_parser) != JSON_PARSER_OBJECT)
		goto err_r_syntax;
	Dee_atomic_rwlock_init(&result->jm_lock);
	DeeObject_Init(result, &DeeJsonMapping_Type);
done:
	return result;
err_r_syntax:
	DeeObject_FREE(result);
	err_json_syntax();
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
jiter_hash(DeeJsonIteratorObject *self) {
	return Dee_HashPointer(atomic_read(&self->ji_parser.jp_pos));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
jiter_compare(DeeJsonIteratorObject *self, DeeJsonIteratorObject *other) {
	if (DeeObject_AssertTypeExact(other, Dee_TYPE(self)))
		goto err;
	Dee_return_compareT(char const *, atomic_read(&self->ji_parser.jp_pos),
	                    /*         */ atomic_read(&other->ji_parser.jp_pos));
err:
	return Dee_COMPARE_ERR;
}

PRIVATE WUNUSED_T NONNULL_T((1)) int DCALL
jseqiter_nii_rewind(DeeJsonIteratorObject *__restrict self) {
	char const *orig_pos;
	struct json_parser parser;
again:
	DeeJsonIterator_GetParser(self, &parser);
	orig_pos = parser.jp_pos;
	if unlikely(libjson_parser_rewind(&parser) != JSON_PARSER_ARRAY)
		goto err_syntax;
	if (!atomic_cmpxch_or_write(&self->ji_parser.jp_pos, orig_pos, parser.jp_pos))
		goto again;
	return 0;
err_syntax:
	return err_json_syntax();
}

PRIVATE WUNUSED_T NONNULL_T((1)) int DCALL
jmapiter_nii_rewind(DeeJsonIteratorObject *__restrict self) {
	char const *orig_pos;
	struct json_parser parser;
again:
	DeeJsonIterator_GetParser(self, &parser);
	orig_pos = parser.jp_pos;
	if unlikely(libjson_parser_rewind(&parser) != JSON_PARSER_OBJECT)
		goto err_syntax;
	if (!atomic_cmpxch_or_write(&self->ji_parser.jp_pos, orig_pos, parser.jp_pos))
		goto again;
	return 0;
err_syntax:
	return err_json_syntax();
}

PRIVATE WUNUSED_T NONNULL_T((1)) int DCALL
jiter_nii_prev(DeeJsonIteratorObject *__restrict self) {
	int status;
	char const *orig_pos;
	struct json_parser parser;
again:
	DeeJsonIterator_GetParser(self, &parser);
	orig_pos = parser.jp_pos;
	status = libjson_parser_prev(&parser, false);
	if (status == JSON_ERROR_NOOBJ)
		return 1;
	if (status == JSON_ERROR_SYNTAX)
		goto err_syntax;
	if (!atomic_cmpxch_or_write(&self->ji_parser.jp_pos, orig_pos, parser.jp_pos))
		goto again;
	return 0;
err_syntax:
	return err_json_syntax();
}

PRIVATE WUNUSED_T NONNULL_T((1)) int DCALL
jiter_nii_next(DeeJsonIteratorObject *__restrict self) {
	int status;
	char const *orig_pos;
	struct json_parser parser;
again:
	DeeJsonIterator_GetParser(self, &parser);
	orig_pos = parser.jp_pos;
	status = libjson_parser_next(&parser);
	if (status == JSON_ERROR_NOOBJ)
		return 1;
	if (status == JSON_ERROR_SYNTAX)
		goto err_syntax;
	if (!atomic_cmpxch_or_write(&self->ji_parser.jp_pos, orig_pos, parser.jp_pos))
		goto again;
	return 0;
err_syntax:
	return err_json_syntax();
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
jseqiter_nii_hasprev(DeeJsonIteratorObject *__restrict self) {
	int tok;
	struct json_parser parser;
	DeeJsonIterator_GetParser(self, &parser);
	tok = libjson_parser_peekprev(&parser);
	if (tok == JSON_PARSER_ARRAY)
		return 0;
	if unlikely(tok == JSON_ERROR_SYNTAX)
		return err_json_syntax();
	return 1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
jmapiter_nii_hasprev(DeeJsonIteratorObject *__restrict self) {
	int tok;
	struct json_parser parser;
	DeeJsonIterator_GetParser(self, &parser);
	tok = libjson_parser_peekprev(&parser);
	if (tok == JSON_PARSER_OBJECT)
		return 0;
	if unlikely(tok == JSON_ERROR_SYNTAX)
		return err_json_syntax();
	return 1;
}


PRIVATE struct type_nii tpconst jseqiter_nii = {
	/* .nii_class = */ Dee_TYPE_ITERX_CLASS_BIDIRECTIONAL,
	/* .nii_flags = */ Dee_TYPE_ITERX_FNORMAL,
	{
		/* .nii_common = */ {
			/* .nii_getseq   = */ (Dee_funptr_t)&jseqiter_getseq,
			/* .nii_getindex = */ NULL,
			/* .nii_setindex = */ NULL,
			/* .nii_rewind   = */ (Dee_funptr_t)&jseqiter_nii_rewind,
			/* .nii_revert   = */ NULL,
			/* .nii_advance  = */ NULL,
			/* .nii_prev     = */ (Dee_funptr_t)&jiter_nii_prev,
			/* .nii_next     = */ (Dee_funptr_t)&jiter_nii_next,
			/* .nii_hasprev  = */ (Dee_funptr_t)&jseqiter_nii_hasprev,
			/* .nii_peek     = */ (Dee_funptr_t)&jseqiter_nii_peek
		}
	}
};

PRIVATE struct type_nii tpconst jmapiter_nii = {
	/* .nii_class = */ Dee_TYPE_ITERX_CLASS_BIDIRECTIONAL,
	/* .nii_flags = */ Dee_TYPE_ITERX_FNORMAL,
	{
		/* .nii_common = */ {
			/* .nii_getseq   = */ (Dee_funptr_t)&jmapiter_getseq,
			/* .nii_getindex = */ NULL,
			/* .nii_setindex = */ NULL,
			/* .nii_rewind   = */ (Dee_funptr_t)&jmapiter_nii_rewind,
			/* .nii_revert   = */ NULL,
			/* .nii_advance  = */ NULL,
			/* .nii_prev     = */ (Dee_funptr_t)&jiter_nii_prev,
			/* .nii_next     = */ (Dee_funptr_t)&jiter_nii_next,
			/* .nii_hasprev  = */ (Dee_funptr_t)&jmapiter_nii_hasprev,
			/* .nii_peek     = */ (Dee_funptr_t)&jmapiter_nii_peek
		}
	}
};

PRIVATE struct type_cmp jseqiter_cmp = {
	/* .tp_hash          = */ (Dee_hash_t (DCALL *)(DeeObject *))&jiter_hash,
	/* .tp_compare_eq    = */ NULL,
	/* .tp_compare       = */ (int (DCALL *)(DeeObject *, DeeObject *))&jiter_compare,
	/* .tp_trycompare_eq = */ NULL,
	/* .tp_eq            = */ NULL,
	/* .tp_ne            = */ NULL,
	/* .tp_lo            = */ NULL,
	/* .tp_le            = */ NULL,
	/* .tp_gr            = */ NULL,
	/* .tp_ge            = */ NULL,
	/* .tp_nii           = */ &jseqiter_nii,
};

PRIVATE struct type_cmp jmapiter_cmp = {
	/* .tp_hash          = */ (Dee_hash_t (DCALL *)(DeeObject *))&jiter_hash,
	/* .tp_compare_eq    = */ NULL,
	/* .tp_compare       = */ (int (DCALL *)(DeeObject *, DeeObject *))&jiter_compare,
	/* .tp_trycompare_eq = */ NULL,
	/* .tp_eq            = */ NULL,
	/* .tp_ne            = */ NULL,
	/* .tp_lo            = */ NULL,
	/* .tp_le            = */ NULL,
	/* .tp_gr            = */ NULL,
	/* .tp_ge            = */ NULL,
	/* .tp_nii           = */ &jmapiter_nii,
};

PRIVATE struct type_getset tpconst jseqiter_getsets[] = {
	TYPE_GETTER_F("seq", &jseqiter_getseq, METHOD_FNOREFESCAPE, "->?GSequence"),
	TYPE_GETSET_END
};

PRIVATE struct type_getset tpconst jmapiter_getsets[] = {
	TYPE_GETTER_F("seq", &jmapiter_getseq, METHOD_FNOREFESCAPE, "->?GMapping"),
	TYPE_GETSET_END
};


PRIVATE WUNUSED NONNULL((1)) int DCALL
jseq_ctor(DeeJsonSequenceObject *__restrict self) {
	self->js_owner              = DeeNone_NewRef();
	self->js_parser.jp_encoding = JSON_ENCODING_UTF8;
	self->js_parser.jp_start    = empty_json_array;
	self->js_parser.jp_pos      = empty_json_array + 1;
	self->js_parser.jp_end      = empty_json_array + 2;
	self->js_size               = 0;
	Dee_atomic_rwlock_init(&self->js_lock);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
jseq_copy(DeeJsonSequenceObject *__restrict self,
          DeeJsonSequenceObject *__restrict other) {
	DeeJsonSequence_LockRead(other);
	self->js_owner  = other->js_owner;
	self->js_parser = other->js_parser;
	self->js_index  = other->js_index;
	self->js_size   = other->js_size;
	Dee_Incref(self->js_owner);
	DeeJsonSequence_LockEndRead(other);
	Dee_atomic_rwlock_init(&self->js_lock);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
jseq_serialize(DeeJsonSequenceObject *__restrict self,
               DeeSerial *__restrict writer, Dee_seraddr_t addr) {
#define ADDROF(field) (addr + offsetof(DeeJsonSequenceObject, field))
	DeeJsonSequenceObject *out = DeeSerial_Addr2Mem(writer, addr, DeeJsonSequenceObject);
	struct json_parser self__js_parser;
	DeeJsonSequence_LockRead(self);
	self__js_parser = self->js_parser;
	out->js_index   = self->js_index;
	out->js_size    = self->js_size;
	DeeJsonSequence_LockEndRead(self);
	out->js_parser.jp_encoding = self__js_parser.jp_encoding;
	Dee_atomic_rwlock_init(&out->js_lock);
	if (DeeSerial_PutObject(writer, ADDROF(js_owner), self->js_owner))
		goto err;
	if (DeeSerial_XPutPointer(writer, ADDROF(js_parser.jp_start), self__js_parser.jp_start))
		goto err;
	if (DeeSerial_XPutPointer(writer, ADDROF(js_parser.jp_end), self__js_parser.jp_end))
		goto err;
	return DeeSerial_XPutPointer(writer, ADDROF(js_parser.jp_pos), self__js_parser.jp_pos);
err:
	return -1;
#undef ADDROF
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
jmap_serialize(DeeJsonMappingObject *__restrict self,
               DeeSerial *__restrict writer, Dee_seraddr_t addr) {
#define ADDROF(field) (addr + offsetof(DeeJsonMappingObject, field))
	DeeJsonMappingObject *out = DeeSerial_Addr2Mem(writer, addr, DeeJsonMappingObject);
	struct json_parser self__js_parser;
	DeeJsonMapping_LockRead(self);
	self__js_parser = self->jm_parser;
	DeeJsonMapping_LockEndRead(self);
	out->jm_parser.jp_encoding = self__js_parser.jp_encoding;
	Dee_atomic_rwlock_init(&out->jm_lock);
	if (DeeSerial_PutObject(writer, ADDROF(jm_owner), self->jm_owner))
		goto err;
	if (DeeSerial_XPutPointer(writer, ADDROF(jm_parser.jp_start), self__js_parser.jp_start))
		goto err;
	if (DeeSerial_XPutPointer(writer, ADDROF(jm_parser.jp_end), self__js_parser.jp_end))
		goto err;
	return DeeSerial_XPutPointer(writer, ADDROF(jm_parser.jp_pos), self__js_parser.jp_pos);
err:
	return -1;
#undef ADDROF
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
jmap_ctor(DeeJsonMappingObject *__restrict self) {
	self->jm_owner              = DeeNone_NewRef();
	self->jm_parser.jp_encoding = JSON_ENCODING_UTF8;
	self->jm_parser.jp_start    = empty_json_object;
	self->jm_parser.jp_pos      = empty_json_object + 1;
	self->jm_parser.jp_end      = empty_json_object + 2;
	Dee_atomic_rwlock_init(&self->jm_lock);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
jmap_copy(DeeJsonMappingObject *__restrict self,
          DeeJsonMappingObject *__restrict other) {
	DeeJsonMapping_LockRead(other);
	self->jm_owner  = other->jm_owner;
	self->jm_parser = other->jm_parser;
	Dee_Incref(self->jm_owner);
	DeeJsonMapping_LockEndRead(other);
	Dee_atomic_rwlock_init(&self->jm_lock);
	return 0;
}


static_assert(offsetof(DeeJsonSequenceObject, js_owner) == offsetof(DeeJsonMappingObject, jm_owner));
static_assert(offsetof(DeeJsonSequenceObject, js_owner) == offsetof(DeeJsonIteratorObject, ji_owner));
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
jseq_or_map_init_parser(DeeJsonSequenceObject *__restrict self,
                        DeeObject *__restrict data) {
	if (DeeBytes_Check(data)) {
		/* Parse raw bytes as JSON. */
		void *start = DeeBytes_DATA(data);
		void *end   = DeeBytes_END(data); /* TODO: CONFIG_EXPERIMENTAL_BYTES_INUSE */
		libjson_parser_init(&self->js_parser, start, end);
		Dee_Incref(data);
	} else if (DeeFile_Check(data)) {
		void *start, *end;
		data = DeeFile_ReadBytes(data, (size_t)-1, true);
		if unlikely(!data)
			goto err;
		start = DeeBytes_DATA(data); /* TODO: CONFIG_EXPERIMENTAL_BYTES_INUSE */
		end   = DeeBytes_END(data);
		libjson_parser_init(&self->js_parser, start, end);
	} else {
		void const *start, *end;
		DeeStringObject *input;
		if (DeeObject_AssertTypeExact(data, &DeeString_Type))
			goto err;
		input = (DeeStringObject *)data;
		if (input->s_data) {
			struct Dee_string_utf *utf = input->s_data;
			/* JSON is smart enough to automatically detect larger encodings,
			 * so in the case of larger-than-utf-8, we can just pass with
			 * width-representation of the string.
			 * 
			 * Still though, if the string already has a utf-8 cache, then we
			 * always use that one (simply because it's more compact, as well
			 * as because libjson originating from KOS, is primarily optimized
			 * for processing UTF-8 data) */
			if (utf->u_utf8) {
				start = utf->u_utf8;
				end   = (char const *)start + WSTR_LENGTH(start);
			} else {
				size_t length;
				start  = utf->u_data[utf->u_width];
				length = WSTR_LENGTH(start);
				length = Dee_STRING_MUL_SIZEOF_WIDTH(length, utf->u_width);
				end    = (byte_t const *)start + length;
			}
		} else {
			char const *utf8 = DeeString_AsUtf8((DeeObject *)input);
			if unlikely(!utf8)
				goto err;
			start = utf8;
			end   = utf8 + WSTR_LENGTH(utf8);
		}
		libjson_parser_init(&self->js_parser, start, end);
		Dee_Incref(data);
	}
	self->js_owner = data; /* Inherit reference */
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
jseq_init(DeeJsonSequenceObject *__restrict self,
          size_t argc, DeeObject *const *argv) {
	DeeObject *data;
	DeeArg_Unpack1(err, argc, argv, "_JsonSequence", &data);
	if unlikely(jseq_or_map_init_parser(self, data))
		goto err;

	/* Parse the leading '['-token */
	if (libjson_parser_yield(&self->js_parser) != JSON_PARSER_ARRAY)
		goto err_syntax_parser_data;
	self->js_index = 0;
	self->js_size  = 0;
	Dee_atomic_rwlock_init(&self->js_lock);
	return 0;
err_syntax_parser_data:
	json_parser_fini(&self->js_parser);
	Dee_Decref(self->js_owner);
	err_json_syntax();
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
jmap_init(DeeJsonMappingObject *__restrict self,
          size_t argc, DeeObject *const *argv) {
	DeeObject *data;
	DeeArg_Unpack1(err, argc, argv, "_JsonMapping", &data);
	if unlikely(jseq_or_map_init_parser((DeeJsonSequenceObject *)self, data))
		goto err;

	/* Parse the leading '{'-token */
	if (libjson_parser_yield(&self->jm_parser) != JSON_PARSER_ARRAY)
		goto err_syntax_parser_data;
	Dee_atomic_rwlock_init(&self->jm_lock);
	return 0;
err_syntax_parser_data:
	json_parser_fini(&self->jm_parser);
	Dee_Decref(self->jm_owner);
	err_json_syntax();
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1)) int DCALL
jseq_bool(DeeJsonSequenceObject *__restrict self) {
	int tok;
	struct json_parser parser;
	DeeJsonSequence_LockRead(self);
	if (self->js_size > 0 || self->js_index != 0) {
		/* If the parser isn't at the first index, then the sequence is non-empty. */
		DeeJsonSequence_LockEndRead(self);
		return 1;
	}
	parser = self->js_parser;
	DeeJsonSequence_LockEndRead(self);

	/* If we're at the first position, then the next token mustn't be ']' */
	tok = libjson_parser_peeknext(&parser);
	if (tok == JSON_PARSER_ENDARRAY)
		return 0;
	if unlikely(tok == JSON_ERROR_SYNTAX)
		return err_json_syntax();
	return 1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
jmap_bool(DeeJsonMappingObject *__restrict self) {
	int tok;
	struct json_parser parser;
	DeeJsonMapping_LockRead(self);
	parser = self->jm_parser;
	DeeJsonMapping_LockEndRead(self);

	/* The current token mustn't be '}' */
	tok = libjson_parser_peeknext(&parser);
	if (tok != JSON_PARSER_ENDOBJECT) {
		if unlikely(tok == JSON_ERROR_SYNTAX)
			return err_json_syntax();
		return 1;
	}

	/* If the current token is '}', the preceding token mustn't be '{' */
	tok = libjson_parser_peekprev(&parser);
	if (tok == JSON_PARSER_OBJECT)
		return 0;
	if unlikely(tok == JSON_ERROR_SYNTAX)
		return err_json_syntax();
	return 1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeJsonIteratorObject *DCALL
jseq_iter(DeeJsonSequenceObject *__restrict self) {
	DREF DeeJsonIteratorObject *result;
	result = DeeObject_MALLOC(DeeJsonIteratorObject);
	if unlikely(!result)
		goto done;
	DeeJsonSequence_LockRead(self);
	result->ji_parser = self->js_parser;
	DeeJsonSequence_LockEndRead(self);
	if unlikely(libjson_parser_rewind(&result->ji_parser) != JSON_PARSER_ARRAY)
		goto err_r_syntax;
	result->ji_owner = self->js_owner;
	Dee_Incref(result->ji_owner);
	DeeObject_Init(result, &DeeJsonSequenceIterator_Type);
done:
	return result;
err_r_syntax:
	DeeObject_FREE(result);
	err_json_syntax();
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeJsonIteratorObject *DCALL
jmap_iter(DeeJsonMappingObject *__restrict self) {
	DREF DeeJsonIteratorObject *result;
	result = DeeObject_MALLOC(DeeJsonIteratorObject);
	if unlikely(!result)
		goto done;
	DeeJsonMapping_LockRead(self);
	result->ji_parser = self->jm_parser;
	DeeJsonMapping_LockEndRead(self);
	if unlikely(libjson_parser_rewind(&result->ji_parser) != JSON_PARSER_OBJECT)
		goto err_r_syntax;
	result->ji_owner = self->jm_owner;
	Dee_Incref(result->ji_owner);
	DeeObject_Init(result, &DeeJsonMappingIterator_Type);
done:
	return result;
err_r_syntax:
	DeeObject_FREE(result);
	err_json_syntax();
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
jseq_size(DeeJsonSequenceObject *__restrict self) {
	int tok;
	size_t result;
	struct json_parser parser;
	if (self->js_size != 0)
		return self->js_size;
	DeeJsonSequence_LockRead(self);
	parser = self->js_parser;
	DeeJsonSequence_LockEndRead(self);
	if unlikely(libjson_parser_rewind(&parser) != JSON_PARSER_ARRAY)
		goto err_syntax;
	result = 0;
	if ((tok = libjson_parser_peeknext(&parser)) != JSON_PARSER_ENDARRAY) {
		if unlikely(tok == JSON_ERROR_SYNTAX)
			goto err_syntax;
		do {
			++result;
		} while ((tok = libjson_parser_next(&parser)) == JSON_ERROR_OK);
		if unlikely(tok == JSON_ERROR_SYNTAX)
			goto err_syntax;
	}
	DeeJsonSequence_LockWrite(self);
	if unlikely(self->js_size != 0) {
		DeeJsonSequence_LockEndWrite(self);
		return self->js_size;
	}
	self->js_size = result;
	DeeJsonSequence_LockEndWrite(self);
	return result;
err_syntax:
	return (size_t)err_json_syntax();
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
jseq_getitem_index(DeeJsonSequenceObject *__restrict self, size_t index) {
	int tok;
	DeeJsonParser parser;
	DREF DeeObject *result;
	size_t old_index;
	DeeJsonSequence_LockRead(self);
	if (self->js_size != 0 && index >= self->js_size) {
		DeeJsonSequence_LockEndRead(self);
		goto err_out_of_bounds;
	}
	parser.djp_parser = self->js_parser;
	old_index         = self->js_index;
	DeeJsonSequence_LockEndRead(self);

	/* Move the parser to the requested item */
	if (index < old_index) {
		size_t delta = old_index - index;
		do {
			if (libjson_parser_prev(&parser.djp_parser, false) != JSON_ERROR_OK)
				goto err_syntax;
		} while (--delta);
	} else if (index > old_index) {
		size_t delta = index - old_index;
		do {
			tok = libjson_parser_next(&parser.djp_parser);
			if (tok != JSON_ERROR_OK) {
				if (tok == JSON_ERROR_SYNTAX)
					goto err_syntax;
				/* Index is out-of-bounds. */
				DeeJsonSequence_LockRead(self);
				self->js_size = index - delta;
				DeeJsonSequence_LockEndRead(self);
				goto err_out_of_bounds;
			}
		} while (--delta);
	}

	/* Parse the requested array element. */
	parser.djp_owner = self->js_owner;
	result = DeeJson_ParseObject(&parser, true);
	if unlikely(!result)
		goto err;

	/* Parse the ',' token following the element. */
	tok = libjson_parser_yield(&parser.djp_parser);
	if (tok != JSON_PARSER_COMMA) {
		if (tok != JSON_PARSER_ENDARRAY)
			goto err_syntax_r;
		DeeJsonSequence_LockWrite(self);
		self->js_size = index + 1;
		DeeJsonSequence_LockEndWrite(self);
		goto done;
	}

	/* Cache the parser as it points to the element *after* the ',' */
	DeeJsonSequence_LockWrite(self);
	self->js_parser.jp_pos = parser.djp_parser.jp_pos;
	self->js_index         = index + 1;
	DeeJsonSequence_LockEndWrite(self);
done:
	return result;
err_out_of_bounds:
	DeeRT_ErrIndexOutOfBounds(Dee_AsObject(self), index, self->js_size);
	goto err;
err_syntax_r:
	Dee_Decref(result);
err_syntax:
	err_json_syntax();
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
jmap_size(DeeJsonMappingObject *__restrict self) {
	int tok;
	size_t result;
	struct json_parser parser;
	DeeJsonMapping_LockRead(self);
	parser = self->jm_parser;
	DeeJsonMapping_LockEndRead(self);
	if unlikely(libjson_parser_rewind(&parser) != JSON_PARSER_OBJECT)
		goto err_syntax;
	result = 0;
	if ((tok = libjson_parser_peeknext(&parser)) != JSON_PARSER_ENDOBJECT) {
		if unlikely(tok == JSON_ERROR_SYNTAX)
			goto err_syntax;
		do {
			++result;
		} while ((tok = libjson_parser_next(&parser)) == JSON_ERROR_OK);
		if unlikely(tok == JSON_ERROR_SYNTAX)
			goto err_syntax;
	}
	return result;
err_syntax:
	return (size_t)err_json_syntax();
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
jmap_trygetitem_string_len_hash(DeeJsonMappingObject *self, char const *key,
                                size_t keylen, Dee_hash_t hash) {
	int error;
	DeeJsonParser parser;
	DREF DeeObject *result;
	(void)hash;
	DeeJsonMapping_LockRead(self);
	parser.djp_parser = self->jm_parser;
	DeeJsonMapping_LockEndRead(self);
	error = libjson_parser_findkey(&parser.djp_parser, key, keylen);
	if unlikely(error == JSON_ERROR_SYNTAX)
		goto err_syntax;
	if (error == JSON_ERROR_NOOBJ)
		return ITER_DONE;

	/* Parse a JSON object at the key we've just found. */
	parser.djp_owner = self->jm_owner;
	result = DeeJson_ParseObject(&parser, true);
	if unlikely(!result)
		goto err;

	/* Try to cache the updated parser (so the next usage works off of a different parser position). */
	DeeJsonMapping_LockWrite(self);
	self->jm_parser.jp_pos = parser.djp_parser.jp_pos;
	DeeJsonMapping_LockEndWrite(self);
	return result;
err_syntax:
	err_json_syntax();
err:
	return  NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
jseq_foreach(DeeJsonSequenceObject *__restrict self, Dee_foreach_t proc, void *arg) {
	Dee_ssize_t temp, result = 0;
	DeeJsonParser parser;
	parser.djp_owner = self->js_owner;
	DeeJsonSequence_LockRead(self);
	parser.djp_parser = self->js_parser;
	DeeJsonSequence_LockEndRead(self);
	if unlikely(libjson_parser_rewind(&parser.djp_parser) != JSON_PARSER_ARRAY)
		goto err_syntax;
	if (libjson_parser_peeknext(&parser.djp_parser) != JSON_PARSER_ENDARRAY) {
		for (;;) {
			int tok;
			DREF DeeObject *elem;
			elem = DeeJson_ParseObject(&parser, true);
			if unlikely(!elem)
				goto err;
			temp = (*proc)(arg, elem);
			Dee_Decref(elem);
			if unlikely(temp < 0)
				goto err_temp;
			result += temp;

			/* Parse the following "," or "]" */
			tok = libjson_parser_yield(&parser.djp_parser);
			if (tok != JSON_PARSER_COMMA) {
				if (tok != JSON_PARSER_ENDARRAY)
					goto err_syntax;
				break;
			}
		}
	}
	return result;
err_temp:
	return temp;
err_syntax:
	err_json_syntax();
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
jmap_foreach_pair(DeeJsonMappingObject *__restrict self, Dee_foreach_pair_t proc, void *arg) {
	DREF DeeObject *key, *value;
	Dee_ssize_t temp, result = 0;
	DeeJsonParser parser;
	parser.djp_owner = self->jm_owner;
	DeeJsonMapping_LockRead(self);
	parser.djp_parser = self->jm_parser;
	DeeJsonMapping_LockEndRead(self);
	if unlikely(libjson_parser_rewind(&parser.djp_parser) != JSON_PARSER_OBJECT)
		goto err_syntax;
	if (libjson_parser_peeknext(&parser.djp_parser) != JSON_PARSER_ENDOBJECT) {
		for (;;) {
			int tok;
			key = DeeJson_ParseString(&parser.djp_parser);
			if unlikely(!key)
				goto err;
			if (libjson_parser_yield(&parser.djp_parser) != JSON_PARSER_COLON)
				goto err_key_syntax;
			value = DeeJson_ParseObject(&parser, true);
			if unlikely(!value)
				goto err_key;
			temp = (*proc)(arg, key, value);
			Dee_Decref(value);
			Dee_Decref(key);
			if unlikely(temp < 0)
				goto err_temp;
			result += temp;

			/* Parse the following "," or "}" */
			tok = libjson_parser_yield(&parser.djp_parser);
			if (tok != JSON_PARSER_COMMA) {
				if (tok != JSON_PARSER_ENDOBJECT)
					goto err_syntax;
				break;
			}
		}
	}
	return result;
err_temp:
	return temp;
err_key:
	Dee_Decref_likely(key);
	return -1;
err_key_syntax:
	Dee_Decref_likely(key);
err_syntax:
	err_json_syntax();
err:
	return -1;
}

PRIVATE struct type_seq jseq_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&jseq_iter,
	/* .tp_sizeob                     = */ NULL,
	/* .tp_contains                   = */ NULL,
	/* .tp_getitem                    = */ NULL,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&jseq_foreach,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&jseq_size,
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&jseq_getitem_index,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ NULL,
	/* .tp_setitem_index              = */ NULL,
	/* .tp_bounditem_index            = */ NULL,
	/* .tp_hasitem_index              = */ NULL,
	/* .tp_getrange_index             = */ NULL,
	/* .tp_delrange_index             = */ NULL,
	/* .tp_setrange_index             = */ NULL,
	/* .tp_getrange_index_n           = */ NULL,
	/* .tp_delrange_index_n           = */ NULL,
	/* .tp_setrange_index_n           = */ NULL,
	/* .tp_trygetitem                 = */ NULL,
	/* .tp_trygetitem_index           = */ NULL,
	/* .tp_trygetitem_string_hash     = */ NULL,
	/* .tp_getitem_string_hash        = */ NULL,
	/* .tp_delitem_string_hash        = */ NULL,
	/* .tp_setitem_string_hash        = */ NULL,
	/* .tp_bounditem_string_hash      = */ NULL,
	/* .tp_hasitem_string_hash        = */ NULL,
	/* .tp_trygetitem_string_len_hash = */ NULL,
	/* .tp_getitem_string_len_hash    = */ NULL,
	/* .tp_delitem_string_len_hash    = */ NULL,
	/* .tp_setitem_string_len_hash    = */ NULL,
	/* .tp_bounditem_string_len_hash  = */ NULL,
	/* .tp_hasitem_string_len_hash    = */ NULL,
};

PRIVATE struct type_seq jmap_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&jmap_iter,
	/* .tp_sizeob                     = */ NULL,
	/* .tp_contains                   = */ NULL,
	/* .tp_getitem                    = */ NULL,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ NULL,
	/* .tp_foreach_pair               = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&jmap_foreach_pair,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&jmap_size,
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ NULL,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ NULL,
	/* .tp_setitem_index              = */ NULL,
	/* .tp_bounditem_index            = */ NULL,
	/* .tp_hasitem_index              = */ NULL,
	/* .tp_getrange_index             = */ NULL,
	/* .tp_delrange_index             = */ NULL,
	/* .tp_setrange_index             = */ NULL,
	/* .tp_getrange_index_n           = */ NULL,
	/* .tp_delrange_index_n           = */ NULL,
	/* .tp_setrange_index_n           = */ NULL,
	/* .tp_trygetitem                 = */ NULL,
	/* .tp_trygetitem_index           = */ NULL,
	/* .tp_trygetitem_string_hash     = */ NULL,
	/* .tp_getitem_string_hash        = */ NULL,
	/* .tp_delitem_string_hash        = */ NULL,
	/* .tp_setitem_string_hash        = */ NULL,
	/* .tp_bounditem_string_hash      = */ NULL,
	/* .tp_hasitem_string_hash        = */ NULL,
	/* .tp_trygetitem_string_len_hash = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&jmap_trygetitem_string_len_hash,
	/* .tp_getitem_string_len_hash    = */ NULL,
	/* .tp_delitem_string_len_hash    = */ NULL,
	/* .tp_setitem_string_len_hash    = */ NULL,
	/* .tp_bounditem_string_len_hash  = */ NULL,
	/* .tp_hasitem_string_len_hash    = */ NULL,
};

PRIVATE NONNULL((1)) void DCALL
jseq_fini(DeeJsonSequenceObject *__restrict self) {
	Dee_Decref(self->js_owner);
}

PRIVATE NONNULL((1, 2)) void DCALL
jseq_visit(DeeJsonSequenceObject *__restrict self,
           Dee_visit_t proc, void *arg) {
	Dee_Visit(self->js_owner);
}

PRIVATE struct type_member tpconst jseq_members[] = {
	TYPE_MEMBER_FIELD("__owner__", STRUCT_OBJECT, offsetof(DeeJsonSequenceObject, js_owner)),
	TYPE_MEMBER_END
};

static_assert(offsetof(DeeJsonSequenceObject, js_owner) == offsetof(DeeJsonMappingObject, jm_owner));
static_assert(offsetof(DeeJsonSequenceObject, js_owner) == offsetof(DeeJsonIteratorObject, ji_owner));
#define jseqiter_fini    jseq_fini
#define jmapiter_fini    jseq_fini
#define jmap_fini        jseq_fini
#define jseqiter_visit   jseq_visit
#define jmapiter_visit   jseq_visit
#define jmap_visit       jseq_visit
#define jseqiter_members jseq_members
#define jmapiter_members jseq_members
#define jmap_members     jseq_members

PRIVATE struct type_member tpconst jseq_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &DeeJsonSequenceIterator_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst jmap_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &DeeJsonMappingIterator_Type),
	TYPE_MEMBER_CONST("KeyType", &DeeString_Type),
	TYPE_MEMBER_END
};


/* Internal types used for sequence proxies that perform JIT parsing of underlying JSON blobs. */
INTERN DeeTypeObject DeeJsonSequenceIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_JsonSequenceIterator",
	/* .tp_doc      = */ DOC("(seq?:?GSequence)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DeeJsonIteratorObject,
			/* tp_ctor:        */ &jseqiter_ctor,
			/* tp_copy_ctor:   */ &jseqiter_copy,
			/* tp_deep_ctor:   */ &jseqiter_deep,
			/* tp_any_ctor:    */ &jseqiter_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &jseqiter_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&jseqiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&jseqiter_bool
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&jseqiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &jseqiter_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&jseqiter_next,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ jseqiter_getsets,
	/* .tp_members       = */ jseqiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

INTERN DeeTypeObject DeeJsonMappingIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_JsonMappingIterator",
	/* .tp_doc      = */ DOC("(seq?:?GMapping)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DeeJsonIteratorObject,
			/* tp_ctor:        */ &jmapiter_ctor,
			/* tp_copy_ctor:   */ &jmapiter_copy,
			/* tp_deep_ctor:   */ &jmapiter_deep,
			/* tp_any_ctor:    */ &jmapiter_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &jmapiter_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&jmapiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&jmapiter_bool
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&jmapiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &jmapiter_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ &jmapiter_iterator,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ jmapiter_getsets,
	/* .tp_members       = */ jmapiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

INTERN DeeTypeObject DeeJsonSequence_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "Sequence",
	/* .tp_doc      = */ DOC("An optimized sequence type for JIT-parsing of JSON arrays.\n"
	                         "\n"

	                         "()\n"
	                         "(data:?X3?DFile?DBytes?Dstring)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DeeJsonSequenceObject,
			/* tp_ctor:        */ &jseq_ctor,
			/* tp_copy_ctor:   */ &jseq_copy,
			/* tp_deep_ctor:   */ &jseq_copy,
			/* tp_any_ctor:    */ &jseq_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &jseq_serialize
		),
		/* .tp_dtor = */ (void (DCALL *)(DeeObject *__restrict))&jseq_fini
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&jseq_bool
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&jseq_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &jseq_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ jseq_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ jseq_class_members
};

INTERN DeeTypeObject DeeJsonMapping_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "Mapping",
	/* .tp_doc      = */ DOC("An optimized mapping type for JIT-parsing of JSON mappings.\n"
	                         "\n"

	                         "()\n"
	                         "(data:?X3?DFile?DBytes?Dstring)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeMapping_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DeeJsonMappingObject,
			/* tp_ctor:        */ &jmap_ctor,
			/* tp_copy_ctor:   */ &jmap_copy,
			/* tp_deep_ctor:   */ &jmap_copy,
			/* tp_any_ctor:    */ &jmap_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &jmap_serialize
		),
		/* .tp_dtor = */ (void (DCALL *)(DeeObject *__restrict))&jmap_fini
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&jmap_bool
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&jmap_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &jmap_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ jmap_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ jmap_class_members
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeJsonMappingObject *DCALL
DeeJsonMapping_New(DeeJsonParser *__restrict self, bool must_advance_parser) {
	DREF DeeJsonMappingObject *result;
	result = DeeObject_MALLOC(DeeJsonMappingObject);
	if unlikely(!result)
		goto done;
	result->jm_parser = self->djp_parser;
	if unlikely(libjson_parser_yield(&result->jm_parser) != JSON_PARSER_OBJECT)
		goto err_syntax_object_retval;
	if (must_advance_parser) {
		if (libjson_parser_next(&self->djp_parser) == JSON_ERROR_SYNTAX)
			goto err_syntax_object_retval;
	}
	Dee_atomic_rwlock_init(&result->jm_lock);
	result->jm_owner = self->djp_owner;
	Dee_Incref(self->djp_owner);
	DeeObject_Init(result, &DeeJsonMapping_Type);
done:
	return result;
err_syntax_object_retval:
	DeeObject_FREE(result);
/*err_syntax:*/
	err_json_syntax();
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeJsonSequenceObject *DCALL
DeeJsonSequence_New(DeeJsonParser *__restrict self, bool must_advance_parser) {
	DREF DeeJsonSequenceObject *result;
	result = DeeObject_MALLOC(DeeJsonSequenceObject);
	if unlikely(!result)
		goto done;
	result->js_parser = self->djp_parser;
	if unlikely(libjson_parser_yield(&result->js_parser) != JSON_PARSER_ARRAY)
		goto err_syntax_array_retval;
	if (must_advance_parser) {
		if (libjson_parser_next(&self->djp_parser) == JSON_ERROR_SYNTAX)
			goto err_syntax_array_retval;
	}
	Dee_atomic_rwlock_init(&result->js_lock);
	result->js_index = 0;
	result->js_size  = 0;
	result->js_owner = self->djp_owner;
	Dee_Incref(self->djp_owner);
	DeeObject_Init(result, &DeeJsonSequence_Type);
done:
	return result;
err_syntax_array_retval:
	DeeObject_FREE(result);
/*err_syntax:*/
	err_json_syntax();
	return NULL;
}


/* Parse a single JSON element and convert it into an object.
 * Upon success, the given parser `self' will point at the first
 * token *after* the just-parsed JSON element (though only when
 * the `must_advance_parser' parameter is set to true).
 *
 * Json elements are converted to deemon objects as follows:
 * - `null'           ->  `DeeNone_Type'
 * - `true', `false'  ->  `DeeBool_Type'
 * - `123'            ->  `DeeInt_Type'
 * - `123.4'          ->  `DeeFloat_Type'
 * - `"foo"'          ->  `DeeString_Type'
 * - `[x, y, z]'      ->  `DeeJsonSequence_Type'
 * - `{ "foo": 42 }'  ->  `DeeJsonMapping_Type'
 *
 * Note that this covers all valid JSON constructs, and that those
 * constructs that allow for the recursive inclusion of other
 * constructs are parsed and returned as abstract sequence proxies
 * that will parse contained JSON constructs as they are accessed.
 *
 * @param: must_advance_parser: When set to `false', there is no
 *                              need to advance `self', though this
 *                              may still be done (meaning that when
 *                              this parameter is `false', `self'
 *                              will be left in an undefined state)
 * @return: * : The equivalent deemon object of the just-parsed JSON
 * @return: NULL: An error was thrown. */
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeJson_ParseObject(DeeJsonParser *__restrict self,
                    bool must_advance_parser) {
	DREF DeeObject *result;
	int tok = libjson_parser_peeknext(&self->djp_parser);
	switch (tok) {

	case JSON_PARSER_STRING:
		result = DeeJson_ParseString(&self->djp_parser);
		break;

	case JSON_PARSER_NUMBER:
		result = DeeJson_ParseNumber(&self->djp_parser);
		break;

	case JSON_PARSER_OBJECT:
		result = Dee_AsObject(DeeJsonMapping_New(self, must_advance_parser));
		break;

	case JSON_PARSER_ARRAY:
		result = Dee_AsObject(DeeJsonSequence_New(self, must_advance_parser));
		break;

	case JSON_PARSER_NULL: {
		if (must_advance_parser && libjson_parser_yield(&self->djp_parser) != JSON_PARSER_NULL)
			goto err_syntax;
		result = Dee_None;
		Dee_Incref(result);
	}	break;

	case JSON_PARSER_TRUE: {
		if (must_advance_parser && libjson_parser_yield(&self->djp_parser) != JSON_PARSER_TRUE)
			goto err_syntax;
		result = Dee_True;
		Dee_Incref(result);
	}	break;

	case JSON_PARSER_FALSE: {
		if (must_advance_parser && libjson_parser_yield(&self->djp_parser) != JSON_PARSER_FALSE)
			goto err_syntax;
		result = Dee_False;
		Dee_Incref(result);
	}	break;

	default:
		goto err_syntax;
	}
	return result;
err_syntax:
	err_json_syntax();
/*err:*/
	return NULL;
}


/* Helper to specifically parse integers / floats.
 * @return: JSON_ERROR_OK:     Yes, it's a float
 * @return: JSON_ERROR_SYNTAX: Syntax error
 * @return: JSON_ERROR_NOOBJ:  No, it's not a float  */
PRIVATE WUNUSED NONNULL((1)) int DCALL
json_parser_peek_number_is_float(struct json_parser const *__restrict self) {
	struct json_parser me = *self;
	char32_t ch = json_getc(&me);
	unsigned int radix = 10;
	if (ch == '-')
		ch = json_getc(&me);
	if (!unicode_isdigit(ch))
		goto err_syntax; /* Not an integer. */
	if (ch == '0') {
		ch  = json_getc(&me);
		if (ch == 'x' || ch == 'X') {
			radix = 16;
			ch    = json_getc(&me);
		} else if (ch == 'b' || ch == 'B') {
			radix = 2;
			ch    = json_getc(&me);
		} else {
			radix = 8;
		}
		if (!unicode_isxdigit(ch)) {
			if (radix != 8)
				goto err_syntax;

			/* Check if this is a floating-point number. */
			if (ch == '.' || ch == 'e' || ch == 'E')
				goto parse_float;

			/* Special case: '0' */
			return JSON_ERROR_NOOBJ;
		}
	}
	for (;;) {
		uint8_t digit;
		if unlikely(!unicode_asdigit(ch, radix, &digit)) {
			if ((ch == '.' || ch == 'e' || ch == 'E') && radix == 10)
				goto parse_float;
			goto err_syntax;
		}
		ch = json_getc(&me);
		if (unicode_isdigit(ch))
			continue;
		if (radix >= 16) {
			if (unicode_ishex(ch))
				continue;
		}
		break;
	}

	/* Fast-path for signed integers that fits into a CPU pointer register. */
	return JSON_ERROR_NOOBJ;
err_syntax:
	return JSON_ERROR_SYNTAX;
parse_float:
	return JSON_ERROR_OK;
}


/* Check what's the canonical type of whatever is about to be parsed by `self' */
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) DeeTypeObject *DCALL
DeeJson_PeekCanonicalObjectType(struct json_parser const *__restrict self) {
	int tok = libjson_parser_peeknext(self);
	switch (tok) {

	case JSON_PARSER_STRING:
		return &DeeString_Type;

	case JSON_PARSER_NUMBER:
		/* Check if it's a float, or an integer. */
		if (json_parser_peek_number_is_float(self) == JSON_ERROR_OK)
			return &DeeFloat_Type;
		return &DeeInt_Type;

	case JSON_PARSER_OBJECT:
		return &DeeJsonMapping_Type;

	case JSON_PARSER_ARRAY:
		return &DeeJsonSequence_Type;

	case JSON_PARSER_NULL:
		return &DeeNone_Type;

	case JSON_PARSER_TRUE:
	case JSON_PARSER_FALSE:
		return &DeeBool_Type;

	default:
		break;
	}
	return &DeeObject_Type;
}




/* Helper to specifically parse strings. */
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeJson_ParseString(struct json_parser *__restrict self) {
	int status;
	Dee_ssize_t error;
	struct Dee_unicode_printer printer = Dee_UNICODE_PRINTER_INIT;
	/* Print the JSON string into a unicode-printer to convert it into a deemon string. */
	status = libjson_parser_printstring(self, &Dee_unicode_printer_print, &printer, &error);
	if (status != JSON_ERROR_OK) {
		Dee_unicode_printer_fini(&printer);
		if (status == JSON_ERROR_SYSERR)
			goto err;    /* `Dee_unicode_printer_print()' returned a negative value. */
		goto err_syntax; /* Either a *true* syntax error, or current token isn't a string. */
	}
	return Dee_unicode_printer_pack(&printer);
err_syntax:
	err_json_syntax();
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeJson_ParseFloat(struct json_parser *__restrict self) {
#ifdef LIBJSON_NO_PARSER_GETFLOAT
	(void)self;
	DeeError_Throwf(&DeeError_UnsupportedAPI, "Cannot decode floats: No FPU support");
	return NULL;
#else /* LIBJSON_NO_PARSER_GETFLOAT */
	double value;
	if (libjson_parser_getfloat(self, &value) != JSON_ERROR_OK)
		goto err_syntax;
	return DeeFloat_New(value);
err_syntax:
	err_json_syntax();
	return NULL;
#endif /* !LIBJSON_NO_PARSER_GETFLOAT */
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeJson_ParseLargeInteger(struct json_parser *__restrict self, char const *start) {
	DREF DeeObject *result;
	char const *end = self->jp_pos;

	/* Parse the actual number itself. */
	switch (self->jp_encoding) {

	case JSON_ENCODING_UTF8:
		result = DeeInt_FromString(start, (size_t)(end - start),
		                           Dee_INT_STRING(0, Dee_INT_STRING_FNOSEPS));
		break;

	case JSON_ENCODING_UTF16LE:
	case JSON_ENCODING_UTF32LE:
	case JSON_ENCODING_UTF16BE:
	case JSON_ENCODING_UTF32BE: {
		/* When a multi-byte encoding is used, parsing gets more complicated
		 * since we first have to convert the input blob into utf-8. */
		DREF DeeObject *input_str;
		char const *input_utf8;
		switch (self->jp_encoding) {
		case JSON_ENCODING_UTF16LE:
			input_str = DeeString_NewUtf16Le((uint16_t const *)start,
			                                 (size_t)((uint16_t const *)end -
			                                          (uint16_t const *)start),
			                                 STRING_ERROR_FSTRICT);
			break;
		case JSON_ENCODING_UTF32LE:
			input_str = DeeString_NewUtf32Le((uint32_t const *)start,
			                                 (size_t)((uint32_t const *)end -
			                                          (uint32_t const *)start),
			                                 STRING_ERROR_FSTRICT);
			break;
		case JSON_ENCODING_UTF16BE:
			input_str = DeeString_NewUtf16Be((uint16_t const *)start,
			                                 (size_t)((uint16_t const *)end -
			                                          (uint16_t const *)start),
			                                 STRING_ERROR_FSTRICT);
			break;
		case JSON_ENCODING_UTF32BE:
			input_str = DeeString_NewUtf32Be((uint32_t const *)start,
			                                 (size_t)((uint32_t const *)end -
			                                          (uint32_t const *)start),
			                                 STRING_ERROR_FSTRICT);
			break;
		default: __builtin_unreachable();
		}
		if unlikely(!input_str)
			goto err;
		input_utf8 = DeeString_AsUtf8(input_str);
		if unlikely(!input_utf8) {
			Dee_Decref(input_str);
			goto err;
		}
		result = DeeInt_FromString(input_utf8, WSTR_LENGTH(input_utf8),
		                           Dee_INT_STRING(0, Dee_INT_STRING_FNOSEPS));
		Dee_Decref(input_str);
	}	break;

	default: __builtin_unreachable();
	}
	if unlikely(!result)
		goto err;

	/* Skip trailing whitespace. */
	json_skip_whitespace(self);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeJson_ParseLargeNumber(struct json_parser *__restrict self,
                         char const *orig_pos, unsigned int radix) {
	char32_t ch;
	char const *pos;
	/* Check if this might be a large floating-point value. */
	for (;;) {
		uint8_t digit;
		pos = self->jp_pos;
		ch  = json_getc(self);
		if unlikely(!unicode_asdigit(ch, radix, &digit)) {
			if ((ch == '.' || ch == 'e' || ch == 'E') && radix == 10)
				goto parse_float;
			self->jp_pos = pos;
			break;
		}
	}

	/* Nope: it's just a large integer value. */
	return DeeJson_ParseLargeInteger(self, orig_pos);
parse_float:
	self->jp_pos = orig_pos;
	return DeeJson_ParseFloat(self);
}

/* Helper to specifically parse integers / floats. */
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeJson_ParseNumber(struct json_parser *__restrict self) {
	char const *orig_pos = self->jp_pos;
	char const *start    = self->jp_pos;
	char32_t ch = json_getc(self);
	bool negative = false;
	unsigned int radix = 10;
	intptr_t result, new_result;
	if (ch == '-') {
		ch = json_getc(self);
		negative = true;
	}
	if (!unicode_isdigit(ch))
		goto err_syntax; /* Not an integer. */
	result = 0;
	if (ch == '0') {
		char const *pos;
		pos = self->jp_pos;
		ch  = json_getc(self);
		if (ch == 'x' || ch == 'X') {
			radix = 16;
			ch    = json_getc(self);
		} else if (ch == 'b' || ch == 'B') {
			radix = 2;
			ch    = json_getc(self);
		} else {
			radix = 8;
		}
		if (!unicode_isxdigit(ch)) {
			if (radix != 8)
				goto err_syntax;

			/* Check if this is a floating-point number. */
			if (ch == '.' || ch == 'e' || ch == 'E')
				goto parse_float;

			/* Special case: '0' */
			json_skip_whitespace_at(self, ch, pos);
			return DeeInt_NewZero();
		}
	}
	for (;;) {
		uint8_t digit;
		if unlikely(!unicode_asdigit(ch, radix, &digit)) {
			if ((ch == '.' || ch == 'e' || ch == 'E') && radix == 10)
				goto parse_float;
			break;
		}
		new_result = (result * radix) + digit;
		if (new_result < result)
			return DeeJson_ParseLargeNumber(self, orig_pos, radix);
		result = new_result;
		start  = self->jp_pos;
		ch     = json_getc(self);
	}

	/* Skip trailing whitespace. */
	json_skip_whitespace_at(self, ch, start);

	/* Store the generated integer. */
	if (negative)
		result = -result;

	/* Fast-path for signed integers that fits into a CPU pointer register. */
	return DeeInt_NEWS(result);
err_syntax:
	err_json_syntax();
	return NULL;
parse_float:
	self->jp_pos = orig_pos;
	return DeeJson_ParseFloat(self);
}


struct type_expression_parser {
	DeeTypeObject *tep_decl_type; /* [1..1] The type that is declaring the doc-string. */
	char const    *tep_doc;       /* [1..1] Parser position (points to the first character after the leading '?') */
	uint32_t       tep_flags;     /* Parser flags (set of `TYPE_EXPRESSION_FLAG_*') */
#define TYPE_EXPRESSION_FLAG_NORMAL                 0x0000 /* Normal flags */
#define TYPE_EXPRESSION_FLAG_GOT_OBJECT             0x0001 /* Encountered `?O', `?DObject', or `?Edeemon:Object' at one point */
#define TYPE_EXPRESSION_FLAG_NEED_DOC_ON_SUCCESS    0x0002 /* `tep_doc' must be advanced, even on success. */
#define TYPE_EXPRESSION_FLAG_NEED_DOC_ON_TYPE_ERROR 0x0004 /* `tep_doc' must be advanced, even on type error. */
};

struct type_expression_name {
	char const           *ten_start; /* [1..1] Start of name */
	char const           *ten_end;   /* [1..1] End of name */
	DREF DeeStringObject *ten_str;   /* [0..1] Name string (in case an extended name was used) */
};

#define type_expression_name_fini(self) Dee_XDecref((self)->ten_str)

PRIVATE WUNUSED NONNULL((1)) int DCALL
type_expression_name_unescape(struct type_expression_name *__restrict self) {
	struct Dee_unicode_printer printer = Dee_UNICODE_PRINTER_INIT;
	char const *iter, *end, *flush_start;

	/* Parse the string and unescape special symbols. */
	iter = self->ten_start;
	end  = self->ten_end;
	flush_start = iter;
	while (iter < end) {
		char ch = *iter++;
		if (ch == '\\') { /* Remove every first '\'-character */
			if unlikely(Dee_unicode_printer_print(&printer, flush_start,
			                                      (size_t)((iter - 1) - flush_start)) < 0)
				goto err_printer;
			flush_start = iter;
			if (iter < end)
				++iter; /* Don't remove the character following '\', even if it's another '\' */
		}
	}
	if (flush_start < end) {
		if unlikely(Dee_unicode_printer_print(&printer, flush_start,
		                                      (size_t)(end - flush_start)) < 0)
			goto err_printer;
	}

	/* Pack the unicode string */
	self->ten_str = (DREF DeeStringObject *)Dee_unicode_printer_pack(&printer);
	if unlikely(!self->ten_str)
		goto err;
	self->ten_start = DeeString_AsUtf8(Dee_AsObject(self->ten_str));
	if unlikely(!self->ten_start)
		goto err_ten_str;
	self->ten_end = self->ten_start + WSTR_LENGTH(self->ten_start);
	return 0;
err_ten_str:
	Dee_Decref(self->ten_str);
	goto err;
err_printer:
	Dee_unicode_printer_fini(&printer);
err:
	return -1;
}

/* Parse a type-expression `<NAME>' element
 * @return: 0 : Success (*result was initialized)
 * @return: -1: An error was thrown (*result is in an undefined state) */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
type_expression_parser_parsename(struct type_expression_parser *__restrict self,
                                 /*out*/ struct type_expression_name *__restrict result) {
	char const *doc = self->tep_doc;
	result->ten_str = NULL;
	if (*doc != '{') {
		result->ten_start = doc;
		while (DeeUni_IsSymCont(*doc))
			++doc;
		result->ten_end = doc;
		self->tep_doc   = doc;
		return 0;
	}
	++doc;
	result->ten_start = doc;
	doc = strchr(doc, '}');
	if unlikely(!doc)
		goto err_bad_doc_string;
	result->ten_end = doc;
	self->tep_doc   = doc + 1;

	/* Check if the string must be unescaped (i.e. contains any '\' characters) */
	if (memchr(result->ten_start, '\\',
	           (size_t)(result->ten_end - result->ten_start)) != NULL)
		return type_expression_name_unescape(result);
	return 0;
err_bad_doc_string:
	return DeeError_Throwf(&DeeError_ValueError,
	                       "Malformed type annotation: Missing '}' after '{' in %q",
	                       self->tep_doc);
}

/* Decode the referenced type, or return `ITER_DONE' if an extended type expression is used.
 * This function handles:
 * - ?.
 * - ?N
 * - ?O
 * - ?#<NAME>
 * - ?D<NAME>
 * - ?U<NAME>     (Treated identical to ?O)
 * - ?G<NAME>
 * - ?E<NAME>:<NAME>
 * - ?A<NAME><TYPE>
 * @return: * :        The referenced type
 * @return: NULL:      An error was thrown
 * @return: ITER_DONE: An extended type expression was used (in this case, `self->tep_doc' is unchanged) */
PRIVATE WUNUSED NONNULL((1)) DREF DeeTypeObject *DCALL
type_expression_parser_parsetype(struct type_expression_parser *__restrict self,
                                 bool accept_non_type_object) {
	DREF DeeTypeObject *result;
	char const *doc = self->tep_doc;
	struct type_expression_name name;
	switch (*doc++) {

	case '.':
		result = self->tep_decl_type;
		Dee_Incref(result);
		self->tep_doc = doc;
		break;

	case 'N':
		result = &DeeNone_Type;
		Dee_Incref(result);
		self->tep_doc = doc;
		break;

	case 'U':
		self->tep_doc = doc;
		if (type_expression_parser_parsename(self, &name))
			goto err;
		type_expression_name_fini(&name);
		doc = self->tep_doc;
		ATTR_FALLTHROUGH
	case 'O':
		result = &DeeObject_Type;
		Dee_Incref(result);
		self->tep_doc = doc;
		break;

	case '#':
	case 'D':
	case 'G':
	case 'E':
	case 'A': {
		DREF DeeObject *base;
		self->tep_doc = doc;
		if (type_expression_parser_parsename(self, &name))
			goto err;
		switch (doc[-1]) {

		case '#':
			base = Dee_AsObject(self->tep_decl_type);
			Dee_Incref(base);
			break;

		case 'D':
			base = Dee_AsObject(DeeModule_GetDeemon());
			Dee_Incref(base);
			break;

		case 'G':
			base = Dee_AsObject(DeeType_GetModule(self->tep_decl_type));
			if unlikely(!base) {
				DeeError_Throwf(&DeeError_TypeError,
				                "Unable to determine module of type %r",
				                self->tep_decl_type);
				goto err_name;
			}
			break;

		case 'E': {
			struct type_expression_name export_name;
			if (*self->tep_doc != ':') {
				DeeError_Throwf(&DeeError_ValueError,
				                "Malformed type annotation: Expected ':' after '?E' in %q",
				                doc);
				goto err_name;
			}
			++self->tep_doc;
			if (name.ten_str == NULL) {
				name.ten_str = (DREF DeeStringObject *)DeeString_NewUtf8(name.ten_start,
				                                                         (size_t)(name.ten_end -
				                                                                  name.ten_start),
				                                                         STRING_ERROR_FSTRICT);
				if unlikely(!name.ten_str)
					goto err_name;
			}
			if (type_expression_parser_parsename(self, &export_name))
				goto err_name;
			if (export_name.ten_str == NULL) {
				export_name.ten_str = (DREF DeeStringObject *)DeeString_NewUtf8(export_name.ten_start,
				                                                                (size_t)(export_name.ten_end -
				                                                                         export_name.ten_start),
				                                                                STRING_ERROR_FSTRICT);
				if unlikely(!export_name.ten_str)
					goto err_name;
			}
			result = (DREF DeeTypeObject *)DeeModule_GetExtern(Dee_AsObject(name.ten_str),
			                                                   Dee_AsObject(export_name.ten_str));
			type_expression_name_fini(&export_name);
			type_expression_name_fini(&name);
			if unlikely(!result)
				goto err;
			goto done_assert_result_is_type;
		}	break;

		case 'A': {
			/* Attribute of another object. */
			if (*self->tep_doc != '?') {
				DeeError_Throwf(&DeeError_ValueError,
				                "Malformed type annotation: Expected '?' after '?A' in %q",
				                doc);
				goto err_name;
			}
			++self->tep_doc;
			base = Dee_AsObject(type_expression_parser_parsetype(self, true));
			if (!ITER_ISOK(base)) {
				if (!base)
					goto err;
				DeeError_Throwf(&DeeError_ValueError,
				                "Malformed type annotation: Expected a basic type expression after '?A' in %q",
				                doc);
				goto err_name;
			}
		}	break;

		default: __builtin_unreachable();
		}
		if (name.ten_str) {
			result = (DREF DeeTypeObject *)DeeObject_GetAttr(base, Dee_AsObject(name.ten_str));
		} else {
			result = (DREF DeeTypeObject *)DeeObject_GetAttrStringLen(base, name.ten_start,
			                                                          (size_t)(name.ten_end - name.ten_start));
		}
		Dee_Decref(base);
		type_expression_name_fini(&name);
		if unlikely(!result)
			goto err;
done_assert_result_is_type:
		if (!accept_non_type_object) {
			if (DeeObject_AssertType(result, &DeeType_Type))
				goto err_r;
		}
	}	break;

	default:
		return (DREF DeeTypeObject *)ITER_DONE;
	}
/*done:*/
	return result;
err_name:
	type_expression_name_fini(&name);
	goto err;
err_r:
	Dee_Decref(result);
err:
	return NULL;
}


/* Skip a `<NAME>'-expression.
 * @return: true:  Success
 * @return: false: Malformed type expression */
PRIVATE NONNULL((1)) bool DCALL
type_expression_parser_skip_name(struct type_expression_parser *__restrict self) {
	char const *doc = self->tep_doc;
	if (*doc != '{') {
		while (DeeUni_IsSymCont(*doc))
			++doc;
	} else {
		++doc;
		doc = strchr(doc, '}');
		if unlikely(!doc)
			return false;
		++doc;
	}
	self->tep_doc = doc;
	return true;
}

/* Skip a whole type-expression.
 * This function expects the parser to be situated on '?', and will exit
 * with it placed *after* the last character of the associated expression,
 * which is either at the end of the type-expression, or on the '?' of the
 * next successor.
 *
 * @return: true:  Success
 * @return: false: Malformed type expression */
PRIVATE NONNULL((1)) bool DCALL
type_expression_parser_skip_expression(struct type_expression_parser *__restrict self) {
	char const *doc = self->tep_doc;
again:
	if unlikely(*doc++ != '?')
		goto err;
	switch (*doc++) {
	case '.':
	case 'N':
	case 'O':
		break;

	case 'E':
		self->tep_doc = doc;
		if unlikely(!type_expression_parser_skip_name(self))
			goto err;
		doc = self->tep_doc;
		if unlikely(*doc != ':')
			goto err;
		++doc;
		ATTR_FALLTHROUGH
	case '#':
	case 'D':
	case 'U':
	case 'G':
		self->tep_doc = doc;
		return type_expression_parser_skip_name(self);

	case 'A':
		self->tep_doc = doc;
		if unlikely(!type_expression_parser_skip_name(self))
			goto err;
		goto again;

	case 'C':
	case 'M':
		self->tep_doc = doc;
		if (!type_expression_parser_skip_expression(self))
			goto err;
		doc = self->tep_doc;
		goto again;

	case 'T':
	case 'X': {
		size_t n;
		if (!DeeUni_AsDigit(*doc, 10, &n))
			break;
		for (;;) {
			uint8_t temp;
			++doc;
			if (!DeeUni_AsDigit(*doc, 10, &temp))
				break;
			n *= 10;
			n += temp;
		}
		if likely(n > 0) {
			if (n > 1) {
				self->tep_doc = doc;
				do {
					if unlikely(!type_expression_parser_skip_expression(self))
						goto err;
				} while (--n > 1);
				doc = self->tep_doc;
			}
			goto again;
		}
	}	break;

	case 'S':
		goto again;

	default: goto err;
	}
	self->tep_doc = doc;
	return true;
err:
	return false;
}



/* Parse a JSON-component expression into an object whose typing is described by `tx_parser'
 * @return: * :        Success
 * @return: ITER_DONE: Encountered JSON cannot be decoded into the requested type
 *                     In this case, `tx_parser' is updated to point to the end of
 *                     the attempted type expression (such that if the caller is
 *                     currently evaluating an `?X<n>' expression, they should try
 *                     the next element next)
 * @return: NULL:      An error was thrown */
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeJsonObject_ParseWithTypeAnnotation(DeeJsonParser *__restrict self,
                                      struct type_expression_parser *__restrict tx_parser,
                                      bool throw_error_if_typing_fails);

/* Same as `DeeJsonObject_ParseWithTypeAnnotation()', but if the requested type is
 * `Object', set the `TYPE_EXPRESSION_FLAG_GOT_OBJECT' flag and return `ITER_DONE'. */
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeJsonObject_ParseWithTypeAnnotationWithObjectCheck(DeeJsonParser *__restrict self,
                                                     struct type_expression_parser *__restrict tx_parser,
                                                     bool throw_error_if_typing_fails);

/* Same as `DeeJsonObject_ParseWithTypeAnnotation()', but used to handle extended type annotations.
 * @return: * :        Success
 * @return: ITER_DONE: Encountered JSON cannot be decoded into the requested type
 * @return: NULL:      An error was thrown */
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeJsonObject_ParseWithTypeAnnotationEx(DeeJsonParser *__restrict self,
                                        struct type_expression_parser *__restrict tx_parser,
                                        bool throw_error_if_typing_fails);



/* Parse JSON into an objectlist `result' with custom typing. This function expects
 * the given parser `self' to be situated *after* the opening '[' token, and in
 * case of success, will exit with the parser situated *after* the closing ']'
 * @return: 0 : Success
 * @return: 1 : Typing error and `throw_error_if_typing_fails' is true
 *              In this case, the position of `self' is undefined.
 * @return: -1: An error was thrown */
PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
DeeJsonObject_ParseTypedObjectList(DeeJsonParser *__restrict self,
                                   struct type_expression_parser *__restrict seq_elem_tx_parser,
                                   struct Dee_objectlist *__restrict result,
                                   bool throw_error_if_typing_fails) {
	int tok;
	DREF DeeTypeObject *wanted_type;
	ASSERT(!(seq_elem_tx_parser->tep_flags & TYPE_EXPRESSION_FLAG_GOT_OBJECT));
	ASSERT(seq_elem_tx_parser->tep_doc[-1] == '?');

	/* Check if the sequence's element-type is a simple type-expression. */
	wanted_type = type_expression_parser_parsetype(seq_elem_tx_parser, false);
	if (ITER_ISOK(wanted_type)) {
		for (;;) {
			int error;
			DREF DeeObject *item;

			/* Check if end-of-array has been reached */
			if (libjson_parser_peeknext(&self->djp_parser) == JSON_PARSER_ENDARRAY) {
				if (libjson_parser_yield(&self->djp_parser) != JSON_PARSER_ENDARRAY)
					goto err_wanted_type_syntax;
				break;
			}
			item = DeeJson_ParseIntoType(self, wanted_type, true,
			                             throw_error_if_typing_fails);
			if (!ITER_ISOK(item)) {
				Dee_Decref(wanted_type);
				if (item == ITER_DONE)
					return 1;
				goto err;
			}
			error = Dee_objectlist_append(result, item);
			Dee_Decref(item);
			if unlikely(error)
				goto err_wanted_type;

			/* Consume the ',' token after the array element. */
			tok = libjson_parser_yield(&self->djp_parser);
			if (tok != JSON_PARSER_COMMA) {
				if (tok == JSON_PARSER_ENDARRAY)
					break;
				goto err_wanted_type_syntax;
			}
		}
		Dee_Decref(wanted_type);
	} else {
		char const *doc;
		if unlikely(!wanted_type)
			goto err;
		doc = seq_elem_tx_parser->tep_doc;
		for (;;) {
			int error;
			DREF DeeObject *item;

			/* Check if end-of-array has been reached */
			if (libjson_parser_peeknext(&self->djp_parser) == JSON_PARSER_ENDARRAY) {
				if (libjson_parser_yield(&self->djp_parser) != JSON_PARSER_ENDARRAY)
					goto err_wanted_type_syntax;
				break;
			}

			/* Parse each sequence element with an extended type-expression. */
			seq_elem_tx_parser->tep_doc = doc;
			item = DeeJsonObject_ParseWithTypeAnnotationEx(self, seq_elem_tx_parser,
			                                               throw_error_if_typing_fails);
			if (!ITER_ISOK(item)) {
				if (item == ITER_DONE)
					return 1;
				goto err;
			}
			error = Dee_objectlist_append(result, item);
			Dee_Decref(item);
			if unlikely(error)
				goto err_wanted_type;

			/* Consume the ',' token after the array element. */
			tok = libjson_parser_yield(&self->djp_parser);
			if (tok != JSON_PARSER_COMMA) {
				if (tok == JSON_PARSER_ENDARRAY)
					break;
				goto err_wanted_type_syntax;
			}
		}
	}
	return 0;
err_wanted_type_syntax:
	err_json_syntax();
err_wanted_type:
	Dee_Decref(wanted_type);
err:
	return -1;
}

PRIVATE ATTR_COLD NONNULL((1, 2)) int DCALL
err_json_cannot_decode_as_type_expression(struct json_parser *__restrict self,
                                          struct type_expression_parser *__restrict tx_parser,
                                          /*nullable*/ DeeStringObject *attr_name) {
	DeeTypeObject *json_type = DeeJson_PeekCanonicalObjectType(self);
	if (attr_name) {
		return DeeError_Throwf(&DeeError_TypeError,
		                       "Json-blob with type %k cannot be used to assign %k.%k "
		                       "which only accepts types %q", /* TODO: pretty-print accepted types */
		                       json_type, tx_parser->tep_decl_type, attr_name,
		                       tx_parser->tep_doc);
	} else {
		return DeeError_Throwf(&DeeError_TypeError,
		                       "Json-blob with type %k cannot be cast to any of %q", /* TODO: pretty-print accepted types */
		                       json_type, tx_parser->tep_doc);
	}
}

/* Same as `DeeJsonObject_ParseWithTypeAnnotation()', but used to handle extended type annotations.
 * @return: * :        Success
 * @return: ITER_DONE: Encountered JSON cannot be decoded into the requested type
 * @return: NULL:      An error was thrown */
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeJsonObject_ParseWithTypeAnnotationEx(DeeJsonParser *__restrict self,
                                        struct type_expression_parser *__restrict tx_parser,
                                        bool throw_error_if_typing_fails) {
	char const *doc_start = tx_parser->tep_doc;

	/* Handle all of the other types of known type annotations. */
	switch (*tx_parser->tep_doc++) {

	case 'M':
		/* TODO: Custom mapping types (these are only allowed when the key-type is `?Dstring') */
		break;

	case 'C':   /* Custom sequence with custom element types */
	case 'S': { /* Generic sequence with custom element types */
		int tok;
		char const *pos;
		uint32_t saved_flags;
		struct Dee_objectlist ol;
		DREF DeeTypeObject *seq_type;
		if (tx_parser->tep_doc[-1] == 'S') {
			/* Generic sequence */
			seq_type = &DeeSeq_Type;
			Dee_Incref(seq_type);
		} else {
			/* Sequences with custom typing. */
			if unlikely(*tx_parser->tep_doc != '?')
				break;
			++tx_parser->tep_doc;
			seq_type = type_expression_parser_parsetype(tx_parser, false);
			if unlikely(!ITER_ISOK(seq_type)) {
				if (!seq_type)
					goto err;
				break;
			}
		}

		/* Consume the leading '?' of the sequence-element type-expression. */
		if unlikely(*tx_parser->tep_doc != '?') {
			Dee_Decref(seq_type);
			break;
		}
		++tx_parser->tep_doc;

		pos = self->djp_parser.jp_pos;
		tok = libjson_parser_yield(&self->djp_parser);
		if unlikely(tok != JSON_PARSER_ARRAY) {
			if (tok == JSON_ERROR_SYNTAX) {
/*err_seq_type_syntax:*/
				err_json_syntax();
				goto err_seq_type;
			}
			self->djp_parser.jp_pos = pos;
			if (!throw_error_if_typing_fails) {
				Dee_Decref(seq_type);
				return ITER_DONE;
			}
			err_json_cannot_parse_type_as_type(DeeJson_PeekCanonicalObjectType(&self->djp_parser),
			                                   seq_type);
err_seq_type:
			Dee_Decref(seq_type);
			goto err;
		}

		/* Parse sequence elements. */
		saved_flags = tx_parser->tep_flags;
		tx_parser->tep_flags &= (TYPE_EXPRESSION_FLAG_NEED_DOC_ON_TYPE_ERROR |
		                         TYPE_EXPRESSION_FLAG_NEED_DOC_ON_SUCCESS);
		Dee_objectlist_init(&ol);
		tok = DeeJsonObject_ParseTypedObjectList(self, tx_parser, &ol,
		                                         throw_error_if_typing_fails);
		tx_parser->tep_flags = saved_flags;
		if unlikely(tok != 0) {
			Dee_objectlist_fini(&ol);
			Dee_Decref(seq_type);
			if unlikely(tok < 0)
				goto err;
			self->djp_parser.jp_pos = pos; /* Rewind to the start of the array. */
			return ITER_DONE;
		}

		/* Create a sequence of type `wanted_type' with the elements from `ol' */

		/* Check for special case: Pack everything into a list. */
		if (seq_type == &DeeList_Type || seq_type == &DeeSeq_Type) {
			DREF DeeListObject *result;
			Dee_DecrefNokill(seq_type);
			result = (DREF DeeListObject *)Dee_objectlist_packlist(&ol);
			if unlikely(!result) {
err_ol:
				Dee_objectlist_fini(&ol);
				goto err;
			}
			return Dee_AsObject(result);
		}

		/* Check for special case: Tuple */
		if (seq_type == &DeeTuple_Type) {
			DREF DeeTupleObject *result;
			Dee_DecrefNokill(seq_type);
			result = (DREF DeeTupleObject *)Dee_objectlist_packtuple(&ol);
			if unlikely(!result)
				goto err_ol;
			return Dee_AsObject(result);
		}

		/* Check for special case: Set (which we treat like `HashSet') */
		if (seq_type == &DeeSet_Type)
			seq_type = &DeeHashSet_Type; /* TODO: Dedicated optimization for sets */

		/* Fallback: create a shared vector which can then be casted to the target sequence type. */
		{
			DREF DeeObject *svec, *result = NULL;
			svec = DeeSharedVector_NewShared(ol.ol_elemc, ol.ol_elemv);
			if likely(svec) {
				result = DeeObject_New(seq_type, 1, &svec);
				DeeSharedVector_Decref(svec);
			}
			Dee_objectlist_fini(&ol);
			Dee_Decref(seq_type);
			return result;
		}
	}	break;

	case 'T': {
		/* Tuple with custom element types. */
		int tok;
		size_t i, n_items;
		DREF DeeTupleObject *result;
		uint32_t saved_flags;
		if (!DeeUni_AsDigit(*tx_parser->tep_doc, 10, &n_items))
			break;
		for (;;) {
			uint8_t temp;
			++tx_parser->tep_doc;
			if (!DeeUni_AsDigit(*tx_parser->tep_doc, 10, &temp))
				break;
			n_items *= 10;
			n_items += temp;
		}

		/* Consume leading '[' of the JSON-array */
		tok = libjson_parser_yield(&self->djp_parser);
		if unlikely(tok != JSON_PARSER_ARRAY) {
			if (tok == JSON_ERROR_SYNTAX)
				goto err_syntax;
			if (!throw_error_if_typing_fails)
				return ITER_DONE;
			err_json_cannot_parse_type_as_type(DeeJson_PeekCanonicalObjectType(&self->djp_parser),
			                                   &DeeTuple_Type);
			goto err;
		}

		/* Construct the tuple we intend to return. */
		result = DeeTuple_NewUninitialized(n_items);
		if unlikely(!result)
			goto err;

		/* Parse input JSON as elements of the tuple. */
		saved_flags = tx_parser->tep_flags;
		tx_parser->tep_flags &= TYPE_EXPRESSION_FLAG_NEED_DOC_ON_TYPE_ERROR;
		tx_parser->tep_flags |= TYPE_EXPRESSION_FLAG_NEED_DOC_ON_SUCCESS;
		for (i = 0; i < n_items;) {
			DREF DeeObject *tuple_item;
			if unlikely(*tx_parser->tep_doc != '?') {
				Dee_Decrefv(DeeTuple_ELEM(result), i);
				DeeTuple_FreeUninitialized(result);
				goto bad_annotation;
			}
			++tx_parser->tep_doc;

			/* Parse JSON array element as tuple item. */
			tuple_item = DeeJsonObject_ParseWithTypeAnnotation(self, tx_parser, throw_error_if_typing_fails);
			if unlikely(!ITER_ISOK(tuple_item)) { /* Error or type-failure */
				Dee_Decrefv(DeeTuple_ELEM(result), i);
				DeeTuple_FreeUninitialized(result);
				tx_parser->tep_flags = saved_flags;
				if (tuple_item == ITER_DONE && (saved_flags & TYPE_EXPRESSION_FLAG_NEED_DOC_ON_TYPE_ERROR)) {
					/* Skip extra type-expression */
					while (i < n_items) {
						++i;
						if (!type_expression_parser_skip_expression(tx_parser))
							goto bad_annotation;
					}
				}
				return tuple_item;
			}

			/* Put the parsed array element into the result tuple. */
			DeeTuple_SET(result, i, tuple_item);
			++i;

			/* Yield ',' or ']' following the array element. */
			tok = libjson_parser_yield(&self->djp_parser);
			if (tok != JSON_PARSER_COMMA) {
				if (tok != JSON_PARSER_ENDARRAY)
					goto err_r_tuple_i_syntax;
				if (i == n_items)
					goto done_tuple;
				Dee_Decrefv(DeeTuple_ELEM(result), i);
				DeeTuple_FreeUninitialized(result);
				tx_parser->tep_flags = saved_flags;
				if (!throw_error_if_typing_fails)
					return ITER_DONE;
				DeeError_Throwf(&DeeError_TypeError,
				                "JSON array length %" PRFuSIZ " is shorted than "
				                "expected length of %" PRFuSIZ " elements",
				                i, n_items);
				goto err;
			}
		}
		tx_parser->tep_flags = saved_flags;

		/* Consume the trailing ']' */
		tok = libjson_parser_yield(&self->djp_parser);
		if unlikely(tok != JSON_PARSER_ENDARRAY) {
			if (tok == JSON_ERROR_SYNTAX)
				goto err_r_tuple_i_syntax;
			/* Array length is incorrect. */
			Dee_Decrefv(DeeTuple_ELEM(result), i);
			DeeTuple_FreeUninitialized(result);
			if (!throw_error_if_typing_fails)
				return ITER_DONE;
			DeeError_Throwf(&DeeError_TypeError,
			                "JSON array length is longer than expected length of %" PRFuSIZ " elements",
			                n_items);
			goto err;
		}

		/* Given the caller the tuple we just parsed. */
done_tuple:
		return Dee_AsObject(result);
err_r_tuple_i_syntax:
		err_json_syntax();
/*err_r_tuple_i:*/
		Dee_Decrefv(DeeTuple_ELEM(result), i);
		DeeTuple_FreeUninitialized(result);
		goto err;
	}	break;

	case 'X': {
		/* Multiple-choice */
		size_t n_choices;
		uint32_t saved_flags;
		char const *pos;
		DREF DeeObject *result;
		if (!DeeUni_AsDigit(*tx_parser->tep_doc, 10, &n_choices))
			break;
		for (;;) {
			uint8_t temp;
			++tx_parser->tep_doc;
			if (!DeeUni_AsDigit(*tx_parser->tep_doc, 10, &temp))
				break;
			n_choices *= 10;
			n_choices += temp;
		}
		if unlikely(!n_choices)
			break;
		saved_flags = tx_parser->tep_flags;
		tx_parser->tep_flags &= TYPE_EXPRESSION_FLAG_NEED_DOC_ON_SUCCESS;
		tx_parser->tep_flags |= TYPE_EXPRESSION_FLAG_NEED_DOC_ON_TYPE_ERROR;
		pos = self->djp_parser.jp_pos;
		do {
			bool local_throw_error_if_typing_fails;
			if (*tx_parser->tep_doc != '?')
				goto bad_annotation;
			++tx_parser->tep_doc;
			self->djp_parser.jp_pos = pos;
			local_throw_error_if_typing_fails = throw_error_if_typing_fails;
			if (n_choices > 1)
				local_throw_error_if_typing_fails = false;
			result = DeeJsonObject_ParseWithTypeAnnotationWithObjectCheck(self, tx_parser,
			                                                              local_throw_error_if_typing_fails);
			if (result != ITER_DONE) { /* Check if we got something other than a type-error. */
				tx_parser->tep_flags = saved_flags;
				if (result != NULL && (saved_flags & TYPE_EXPRESSION_FLAG_NEED_DOC_ON_SUCCESS)) {
					/* Skip extra type-expression */
					--n_choices;
					while (n_choices) {
						--n_choices;
						if (!type_expression_parser_skip_expression(tx_parser)) {
							goto bad_annotation;
						}
					}
				}
				return result;
			}
		} while (--n_choices);

		/* Special case: Is `Object' is one of the accepted choices? */
		if (tx_parser->tep_flags & TYPE_EXPRESSION_FLAG_GOT_OBJECT) {
			tx_parser->tep_flags = saved_flags;
			self->djp_parser.jp_pos = pos;
			return DeeJson_ParseObject(self, true);
		}

		/* Throw an error or return ITER_DONE */
		tx_parser->tep_flags = saved_flags;
		if (throw_error_if_typing_fails) {
			self->djp_parser.jp_pos = pos;
			tx_parser->tep_doc = doc_start;
			err_json_cannot_decode_as_type_expression(&self->djp_parser, tx_parser, NULL);
			goto err;
		}
		return ITER_DONE;
	}	break;

	case 'R':
	case 'Q':
		/* Not implemented: evaluated type-expressions. */
		break;

	default:
		break;
	}
bad_annotation:
	DeeError_Throwf(&DeeError_ValueError,
	                "Malformed type annotation: %q",
	                doc_start - 1);
err:
	return NULL;
err_syntax:
	err_json_syntax();
	goto err;
}

/* Parse a JSON-component expression into an object whose typing is described by `tx_parser'
 * @return: * :        Success
 * @return: ITER_DONE: Encountered JSON cannot be decoded into the requested type
 *                     In this case, `tx_parser' is updated to point to the end of
 *                     the attempted type expression (such that if the caller is
 *                     currently evaluating an `?X<n>' expression, they should try
 *                     the next element next)
 * @return: NULL:      An error was thrown */
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeJsonObject_ParseWithTypeAnnotation(DeeJsonParser *__restrict self,
                                      struct type_expression_parser *__restrict tx_parser,
                                      bool throw_error_if_typing_fails) {
	DREF DeeTypeObject *wanted_type;
	ASSERT(tx_parser->tep_doc[-1] == '?');

	/* Try to parse a simple type expression. */
	wanted_type = type_expression_parser_parsetype(tx_parser, false);
	if (ITER_ISOK(wanted_type)) {
		DREF DeeObject *result;

		/* Caller wants us to produce an object with specific typing. */
		result = DeeJson_ParseIntoType(self, wanted_type, true,
		                               throw_error_if_typing_fails);
		Dee_Decref(wanted_type);
		return result;
	}
	if unlikely(!wanted_type)
		goto err;

	/* Parse an extended type annotation. */
	return DeeJsonObject_ParseWithTypeAnnotationEx(self, tx_parser, throw_error_if_typing_fails);
err:
	return NULL;
}

/* Same as `DeeJsonObject_ParseWithTypeAnnotation()', but if the requested type is
 * `Object', set the `TYPE_EXPRESSION_FLAG_GOT_OBJECT' flag and return `ITER_DONE'. */
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeJsonObject_ParseWithTypeAnnotationWithObjectCheck(DeeJsonParser *__restrict self,
                                                     struct type_expression_parser *__restrict tx_parser,
                                                     bool throw_error_if_typing_fails) {
	DREF DeeTypeObject *wanted_type;
	ASSERT(tx_parser->tep_doc[-1] == '?');

	/* Try to parse a simple type expression. */
	wanted_type = type_expression_parser_parsetype(tx_parser, false);
	if (ITER_ISOK(wanted_type)) {
		DREF DeeObject *result;

		/* Special case for `Object' (which is handled differently) */
		if unlikely(wanted_type == &DeeObject_Type) {
			tx_parser->tep_flags |= TYPE_EXPRESSION_FLAG_GOT_OBJECT;
			Dee_DecrefNokill(wanted_type);
			return ITER_DONE;
		}

		/* Caller wants us to produce an object with specific typing. */
		result = DeeJson_ParseIntoType(self, wanted_type, true,
		                               throw_error_if_typing_fails);
		Dee_Decref(wanted_type);
		return result;
	}
	if unlikely(!wanted_type)
		goto err;

	/* Parse an extended type annotation. */
	return DeeJsonObject_ParseWithTypeAnnotationEx(self, tx_parser, throw_error_if_typing_fails);
err:
	return NULL;
}

/* Parse a JSON-component expression and assign it to the specified class-attribute
 * @return: 1 : `throw_error_if_typing_fails' is `false' and typing failed
 * @return: 0 : Success
 * @return: -1: An error was thrown */
PRIVATE WUNUSED NONNULL((1, 2, 3, 4, 5)) int DCALL
DeeJsonObject_ParseIntoClassAttribute(DeeJsonParser *__restrict self,
                                      DeeObject *into,
                                      DeeTypeObject *into_attr_type,
                                      struct Dee_class_desc *into_attr_class,
                                      struct Dee_class_attribute *into_attr,
                                      bool throw_error_if_typing_fails) {
	int result;
	struct Dee_instance_desc *into_instance;
	DREF DeeObject *value;
	if (into_attr->ca_doc) {
		struct type_expression_parser parser;
		parser.tep_doc = DeeString_STR(into_attr->ca_doc);
		if (bcmpc(parser.tep_doc, "->?", 3, sizeof(char)) != 0)
			goto fallback;
		parser.tep_decl_type = into_attr_type;
		parser.tep_flags     = TYPE_EXPRESSION_FLAG_NORMAL;
		parser.tep_doc += 3;
		value = DeeJsonObject_ParseWithTypeAnnotation(self, &parser, throw_error_if_typing_fails);
		if (!ITER_ISOK(value)) {
			if (!value)
				goto err;

			/* Check for special case: if we ever encountered
			 * `Object' as a candidate, parse as generic JSON. */
			if (parser.tep_flags & TYPE_EXPRESSION_FLAG_GOT_OBJECT)
				goto fallback;

			/* Throw an error explaining that the given JSON-expression
			 * cannot be stored in a field that requires the given types. */
			if (!throw_error_if_typing_fails)
				return 1; /* Typing failed... */
			parser.tep_doc = DeeString_STR(into_attr->ca_doc) + 3;
			err_json_cannot_decode_as_type_expression(&self->djp_parser, &parser, into_attr->ca_name);
			goto err;
		}
	} else {
		/* Fallback: parse a generic JSON object. */
fallback:
		value = DeeJson_ParseObject(self, true);
		if unlikely(!value)
			goto err;
	}
	into_instance = DeeInstance_DESC(into_attr_class, into);
	result = DeeInstance_SetAttribute(into_attr_class, into_instance, into, into_attr, value);
	Dee_Decref(value);
	return result;
err:
	return -1;
}

/* Parse a JSON-component expression and assign it to `<into>.operator . (<attr_name>)'
 * @return: 1 : `throw_error_if_typing_fails' is `false' and typing failed
 * @return: 0 : Success
 * @return: -1: An error was thrown */
PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
DeeJsonObject_ParseIntoAttribute(DeeJsonParser *__restrict self,
                                 DeeObject *into,
                                 DeeStringObject *attr_name,
                                 bool throw_error_if_typing_fails) {
	int error;
	DREF DeeObject *value;
	DeeTypeObject *into_type = Dee_TYPE(into);
	if (DeeType_IsClass(into_type)) {
		struct Dee_class_desc *attr_class;
		struct Dee_class_attribute *attr;
		attr_class = DeeClass_DESC(into_type);
		attr       = DeeClassDesc_QueryInstanceAttribute(attr_class, (DeeObject *)attr_name);
		if (!attr) {
			/* Check if the attribute exists in a base-class */
			DeeTypeMRO mro;
			into_type = DeeTypeMRO_Init(&mro, into_type);
			for (;;) {
				into_type = DeeTypeMRO_Next(&mro, into_type);
				if (into_type == NULL)
					goto fallback;
				if (!DeeType_IsClass(into_type))
					goto fallback;
				attr_class = DeeClass_DESC(into_type);
				attr       = DeeClassDesc_QueryInstanceAttribute(attr_class, (DeeObject *)attr_name);
				if (attr)
					break;
			}
		}

		/* Parse into a specific attribute. */
		return DeeJsonObject_ParseIntoClassAttribute(self, into, into_type, attr_class,
		                                             attr, throw_error_if_typing_fails);
	}

fallback:
	/* Parse as generic JSON and do a regular attribute assignment. */
	value = DeeJson_ParseObject(self, true);
	if unlikely(!value)
		goto err;
	error = DeeObject_SetAttr(into, (DeeObject *)attr_name, value);
	Dee_Decref(value);
	return error;
err:
	return -1;
}

/* Same as `DeeJson_ParseInto()', but the leading `{' has already been parsed.
 * @return: 1 : `throw_error_if_typing_fails' is `false' and typing failed
 * @return: 0 : Success
 * @return: -1: An error was thrown */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
DeeJsonObject_ParseInto(DeeJsonParser *__restrict self, DeeObject *into,
                        bool must_advance_parser,
                        bool throw_error_if_typing_fails) {
	DREF DeeStringObject *attr_name;
	for (;;) {
		int error;
		if (libjson_parser_peeknext(&self->djp_parser) == JSON_PARSER_ENDOBJECT)
			break;

		/* Parse the attribute name. */
		attr_name = (DREF DeeStringObject *)DeeJson_ParseString(&self->djp_parser);
		if unlikely(!attr_name)
			goto err;
		if (libjson_parser_yield(&self->djp_parser) != JSON_PARSER_COLON)
			goto err_syntax_attr_name;

		/* Parse the value to-be assigned to the attribute. */
		error = DeeJsonObject_ParseIntoAttribute(self, into, attr_name,
		                                         throw_error_if_typing_fails);
		Dee_Decref(attr_name);
		if unlikely(error != 0)
			return error; /* Error or type-error */

		/* Parse the ',' following the attribute, or the trailing '}'-token. */
		error = libjson_parser_yield(&self->djp_parser);
		if (error != JSON_PARSER_COMMA) {
			if (error == JSON_PARSER_ENDOBJECT)
				return 0;
			goto err_syntax;
		}
	}
	if (must_advance_parser) {
		if (libjson_parser_yield(&self->djp_parser) != JSON_PARSER_ENDOBJECT)
			goto err_syntax;
	}
	return 0;
err_syntax_attr_name:
	Dee_Decref(attr_name);
err_syntax:
	return err_json_syntax();
err:
	return -1;
}

/* Implement the functionality of parsing JSON *into* the attributes
 * of a given object `into'. Note that for this purpose, it is OK if
 * the object specifies more fields than are provided by `self', but
 * it is not OK if `self' tries to set a non-existent field, or (when
 * the field being set has type annotation) tries to set a field with
 * a type annotation that disallows the type of object given by `self'
 *
 * Additionally, when type annotation specifies a type other than the
 * standard set of literals supported by JSON (bool, none, string, int,
 * float, Sequence or Mapping), such as a user-defined class, then an
 * instance of that class will be created and recursively parsed into.
 *
 * NOTE: This function also supports user-defined struct types defined
 *       by the `ctypes' dex.
 *
 * @return: 1 : `throw_error_if_typing_fails' is `false' and typing failed
 * @return: 0 : Success
 * @return: -1: An error was thrown */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
DeeJson_ParseInto(DeeJsonParser *__restrict self, DeeObject *into,
                  bool must_advance_parser,
                  bool throw_error_if_typing_fails) {
	int tok = libjson_parser_yield(&self->djp_parser);
	if (tok == JSON_PARSER_OBJECT) {
		return DeeJsonObject_ParseInto(self, into,
		                               must_advance_parser,
		                               throw_error_if_typing_fails);
	}

	/* Check for special type: `null' can be parsed into `none' */
	if (tok == JSON_PARSER_NULL && DeeNone_Check(into))
		return 0;

	/* Only JSON-objects can be parsed into deemon objects,
	 * so any token other than '{' is a syntax error. */
	return err_json_syntax();
}


/* Similar to `DeeJson_ParseInto()', but construct an instance of `into_type' and populate it.
 * @return: *   : Success
 * @return: NULL: An error was thrown
 * @return: ITER_DONE: `throw_error_if_typing_fails' is `false' and typing failed */
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeJson_ParseIntoType(DeeJsonParser *__restrict self,
                      DeeTypeObject *into_type,
                      bool must_advance_parser,
                      bool throw_error_if_typing_fails) {
	int error;
	DREF DeeObject *result;

	/* Handling for special core types */

	/* None (aka. `null') */
	if (into_type == &DeeNone_Type) {
		int tok = libjson_parser_peeknext(&self->djp_parser);
		if (tok == JSON_PARSER_NULL) {
			if (libjson_parser_yield(&self->djp_parser) != JSON_PARSER_NULL)
				goto err_syntax;
			return_none;
		}
		goto err_cannot_parse_into;
	}

	/* Bool */
	if (into_type == &DeeBool_Type) {
		int tok = libjson_parser_peeknext(&self->djp_parser);
		if (tok == JSON_PARSER_TRUE) {
			if (libjson_parser_yield(&self->djp_parser) != JSON_PARSER_TRUE)
				goto err_syntax;
			return_true;
		}
		if (tok == JSON_PARSER_FALSE) {
			if (libjson_parser_yield(&self->djp_parser) != JSON_PARSER_FALSE)
				goto err_syntax;
			return_false;
		}
		goto err_cannot_parse_into;
	}

	/* String */
	if (into_type == &DeeString_Type) {
		int tok = libjson_parser_peeknext(&self->djp_parser);
		if (tok == JSON_PARSER_STRING)
			return DeeJson_ParseString(&self->djp_parser);
		goto err_cannot_parse_into;
	}

	/* Number types */
	if (into_type == &DeeNumeric_Type || /* TODO: Support for numeric types from ctypes */
	    into_type == &DeeInt_Type ||
	    into_type == &DeeFloat_Type) {
		int tok = libjson_parser_peeknext(&self->djp_parser);
		if (tok == JSON_PARSER_NUMBER) {
			char const *pos;
			pos    = self->djp_parser.jp_pos;
			result = DeeJson_ParseNumber(&self->djp_parser);
			if unlikely(!result)
				goto err;
			if (DeeObject_InstanceOf(result, into_type))
				return result;
			self->djp_parser.jp_pos = pos;
			Dee_Decref_likely(result);
		}
		goto err_cannot_parse_into;
	}

	/* Mapping types */
	if (DeeType_Implements(into_type, &DeeMapping_Type)) {
		int tok;
		tok = libjson_parser_peeknext(&self->djp_parser);
		if (tok == JSON_PARSER_OBJECT) {
			result = Dee_AsObject(DeeJsonMapping_New(self, must_advance_parser));
check_result_and_maybe_cast_to_into_type:
			if unlikely(!result)
				goto err;
			if (!DeeObject_InstanceOf(result, into_type)) {
				DREF DeeObject *new_result;
				new_result = DeeObject_New(into_type, 1, &result);
				Dee_Decref(result);
				result = new_result;
			}
			return result;
		}
		goto err_cannot_parse_into;
	}

	/* Sequence types */
	if (DeeType_Implements(into_type, &DeeSeq_Type)) {
		int tok;
		tok = libjson_parser_peeknext(&self->djp_parser);
		if (tok == JSON_PARSER_ARRAY) {
			result = Dee_AsObject(DeeJsonSequence_New(self, must_advance_parser));
			goto check_result_and_maybe_cast_to_into_type;
		}
		goto err_cannot_parse_into;
	}

	/* Special case: `Object' means anything goes */
	if (into_type == &DeeObject_Type)
		return DeeJson_ParseObject(self, must_advance_parser);

	/* Default: construct an instance of the given type and parse into it. */
	result = DeeObject_NewDefault(into_type);
	if unlikely(!result)
		goto err;
	error = DeeJson_ParseInto(self, result,
	                          must_advance_parser,
	                          throw_error_if_typing_fails);
	if (error == 0)
		return result;
	Dee_Decref_likely(result);
	if (error > 0)
		return ITER_DONE;
err:
	return NULL;
err_syntax:
	err_json_syntax();
	goto err;
err_cannot_parse_into:
	if (throw_error_if_typing_fails) {
		err_json_cannot_parse_type_as_type(DeeJson_PeekCanonicalObjectType(&self->djp_parser),
		                                   into_type);
		return NULL;
	}
	return ITER_DONE;
}



PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
DeeJsonMapping_IntoObject(DeeJsonMappingObject *self,
                          DeeObject *obj) {
	DeeJsonParser parser;
	DeeJsonMapping_LockRead(self);
	parser.djp_parser = self->jm_parser;
	DeeJsonMapping_LockEndRead(self);
	parser.djp_owner = self->jm_owner;
	if (libjson_parser_rewind(&parser.djp_parser) != JSON_PARSER_OBJECT)
		goto err_syntax;
	return DeeJsonObject_ParseInto(&parser, obj, false, true);
err_syntax:
	return err_json_syntax();
}

/* Same as `DeeJson_ParseInto()', but don't actually do any JSON-parsing,
 * but simply read out the elements of `mapping' and assign them to the
 * attributes of `self', whilst doing the same special handling that is
 * also done by `DeeJson_ParseInto()' for type annotations.
 *
 * @return: 0 : Success
 * @return: -1: An error was thrown */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
DeeObject_PopulateFromMapping(DeeObject *self, DeeObject *mapping) {
	/* Special handling for when the given `mapping' is a JSON-mapping-wrapper.
	 * In this case, we can use the dedicated JSON-into-object code-path. */
	if (DeeObject_InstanceOfExact(mapping, &DeeJsonMapping_Type))
		return DeeJsonMapping_IntoObject((DeeJsonMappingObject *)mapping, self);

	/* TODO: just like with the parse-from-JSON code above:
	 * >> import json;
	 * >> local jsonBlob = json.write(mapping);
	 * >> json.parse(jsonBlob, into: self); // Only that types that can't be represented as JSON also work here!
	 */
	(void)self;
	(void)mapping;
	return DeeError_NOTIMPLEMENTED();
}


PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
json_foreach_write_item(void *arg, DeeObject *elem) {
	DeeJsonWriter *me = (DeeJsonWriter *)arg;
	return DeeJson_WriteObject(me, elem);
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
json_foreach_write_pair(void *arg, DeeObject *key, DeeObject *value) {
	char const *key_utf8;
	DeeJsonWriter *me = (DeeJsonWriter *)arg;
	if (DeeObject_AssertTypeExact(key, &DeeString_Type))
		goto err;
	key_utf8 = DeeString_AsUtf8(key);
	if unlikely(!key_utf8)
		goto err;
	if unlikely(libjson_writer_addfield(&me->djw_writer, key_utf8, WSTR_LENGTH(key_utf8)))
		goto err;
	return DeeJson_WriteObject(me, value);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
libjson_writer_writeattr(void *arg, struct Dee_attrdesc *__restrict attr) {
	DeeJsonWriter *me = (DeeJsonWriter *)arg;
	DREF DeeObject *attr_value;
	int error;

	/* Skip attributes from low-level base classes. */
	if (attr->ad_info.ai_decl == Dee_AsObject(&DeeObject_Type) ||
	    attr->ad_info.ai_decl == Dee_AsObject(&DeeType_Type)) {
		if (attr->ad_info.ai_decl != me->djw_obj)
			return 0;
	}

	/* Check if we should include the attribute. For this purpose:
	 * - The attribute must not be private
	 * - The attribute must not be a property
	 * - The attribute must not be a function
	 * - The attribute must not be part of the class definition (i.e. must not be static)
	 * - The attribute must be readable */
	if ((attr->ad_perm & (Dee_ATTRPERM_F_PRIVATE | Dee_ATTRPERM_F_PROPERTY | Dee_ATTRPERM_F_CMEMBER |
	                      Dee_ATTRPERM_F_CANGET | Dee_ATTRPERM_F_CANCALL)) != (Dee_ATTRPERM_F_CANGET))
		return 0;
	if (attr->ad_perm & Dee_ATTRPERM_F_NAMEOBJ) {
		DeeStringObject *str_name = Dee_attrdesc_nameobj(attr);
		attr_value = DeeObject_GetAttr(me->djw_obj, (DeeObject *)str_name);
	} else {
		attr_value = DeeObject_GetAttrString(me->djw_obj, attr->ad_name);
	}
	if unlikely(!attr_value) {
		/* Unbound attributes simply aren't included in JSON blobs. */
		if (DeeError_Catch(&DeeError_UnboundAttribute))
			return 0;
		goto err;
	}

	/* All right! Let's write this attribute. */
	if (attr->ad_perm & Dee_ATTRPERM_F_NAMEOBJ) {
		DeeStringObject *str_name = Dee_attrdesc_nameobj(attr);
		char const *name_utf8 = DeeString_AsUtf8((DeeObject *)str_name);
		if unlikely(!name_utf8)
			goto err_attr_value;
		error = libjson_writer_addfield(&me->djw_writer, name_utf8, WSTR_LENGTH(name_utf8));
	} else {
		error = libjson_writer_addfield(&me->djw_writer, attr->ad_name, strlen(attr->ad_name));
	}
	if unlikely(error)
		goto err_attr_value;

	/* Write the attribute value */
	error = DeeJson_WriteObject(me, attr_value);
	Dee_Decref(attr_value);
	return error;
err_attr_value:
	Dee_Decref(attr_value);
err:
	return -1;
}


/* Convert an object `obj' to JSON and write said JSON to `self'.
 * This function supports the same set of object types as are supported
 * by the parsing set of functions above, and you can configure `self'
 * to either produce JSON in a compact (`JSON_WRITER_FORMAT_COMPACT')
 * or pretty (`JSON_WRITER_FORMAT_PRETTY') format.
 *
 * @return: 0 : Success
 * @return: -1: An error was thrown */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
DeeJson_WriteObject(DeeJsonWriter *__restrict self,
                    DeeObject *__restrict obj) {
	DeeTypeObject *type = Dee_TYPE(obj);
	if (type == &DeeString_Type) {
		char const *utf8 = DeeString_AsUtf8(obj);
		if unlikely(!utf8)
			goto err;
		if unlikely(libjson_writer_putstring(&self->djw_writer, utf8, WSTR_LENGTH(utf8)))
			goto err;
	} else if (type == &DeeNone_Type) {
		if unlikely(libjson_writer_putvalue(&self->djw_writer))
			goto err;
		if unlikely(json_print(&self->djw_writer, "null", 4))
			goto err;
	} else if (type == &DeeFloat_Type || /* TODO: Support for numeric types from `ctypes' */
	           type == &DeeInt_Type ||
	           type == &DeeBool_Type) {
		/* Can just print the object representation as-is */
		Dee_ssize_t temp;
		if unlikely(libjson_writer_putvalue(&self->djw_writer))
			goto err;
		temp = DeeObject_PrintRepr(obj, self->djw_writer.jw_printer, self->djw_writer.jw_arg);
		if unlikely(temp < 0) {
			self->djw_writer.jw_result = temp;
			goto err;
		}
		self->djw_writer.jw_result += temp;
	} else {
		/* TODO: Types need to be able to define some magic function whose
		 *       presence indicates that some custom behavior should be
		 *       used in order to facilitate JSON encode/decode.
		 * Idea:
		 * >> class MyClass {
		 * >>     public function __into_literal__(): int | float | bool | none | string | Sequence | Mapping;
		 * >>     public static function __from_literal__(lit: int | float | bool | none | string | Sequence | Mapping): MyClass;
		 * >> }
		 * This pair of functions (if present) would then be used to convert
		 * an object to/from JSON-compatible literals. For example, these functions
		 * could be implemented by `Time from time' to have __into_literal__ return
		 * strings like "2023-05-29T20:39Z", and `__from_literal__' to parse those
		 * strings once again (in addition to also supporting other formats, which
		 * would then depend on the actual value of the given `lit')
		 *
		 * NOTE: But before implementing something like this, check if (and how)
		 *       python might already do something similar to this in its json module.
		 */

		/* Make sure that we haven't already written this object. */
		int already_handled_status;
		already_handled_status = DeeJsonWriter_InsertActive(self, obj);
		if (already_handled_status <= 0) {
			if unlikely(already_handled_status < 0)
				goto err;

			/* Support for user-defined recursion handlers. */
			if (!DeeNone_Check(self->djw_recursion)) {
				obj = DeeObject_Call(self->djw_recursion, 1, (DeeObject **)&obj);
				if unlikely(!obj)
					goto err;

				/* Check if the produced object is already being printed. */
				already_handled_status = DeeJsonWriter_IsActive(self, obj);
				if (already_handled_status == 0) {
					int result = DeeJson_WriteObject(self, obj);
					Dee_Decref(obj);
					return result;
				}
				Dee_Decref(obj);
				if unlikely(already_handled_status < 0)
					goto err;
				/* Fallthru to the error-throw below. */
			}

			/* Object was already written */
			DeeError_Throwf(&DeeError_ValueError,
			                "Cannot write recursive object %r as json",
			                obj);
			goto err;
		}
		if (DeeType_Implements(type, &DeeMapping_Type)) {
			if unlikely(libjson_writer_beginobject(&self->djw_writer))
				goto err;
			if unlikely(DeeObject_ForeachPair(obj, &json_foreach_write_pair, self) < 0)
				goto err;
			if unlikely(libjson_writer_endobject(&self->djw_writer))
				goto err;
		} else if (DeeType_Implements(type, &DeeSeq_Type)) {
			if unlikely(libjson_writer_beginarray(&self->djw_writer))
				goto err;
			if unlikely(DeeObject_Foreach(obj, &json_foreach_write_item, self) < 0)
				goto err;
			if unlikely(libjson_writer_endarray(&self->djw_writer))
				goto err;
		} else {
			DeeObject *old_obj;
			/* Generic object (write attributes as an object) */
			if unlikely(libjson_writer_beginobject(&self->djw_writer))
				goto err;
			old_obj = self->djw_obj;
			self->djw_obj = obj;
			{
				struct Dee_attrhint filter;
				filter.ah_decl = NULL;

				/* Check if we should include the attribute. For this purpose:
				 * - The attribute must not be private
				 * - The attribute must not be a property
				 * - The attribute must not be a function
				 * - The attribute must not be part of the class definition (i.e. must not be static)
				 * - The attribute must be readable */
				filter.ah_perm_mask = Dee_ATTRPERM_F_PRIVATE | Dee_ATTRPERM_F_PROPERTY | Dee_ATTRPERM_F_CMEMBER |
				                      Dee_ATTRPERM_F_CANGET | Dee_ATTRPERM_F_CANCALL;
				filter.ah_perm_value = Dee_ATTRPERM_F_CANGET;
				if unlikely(DeeObject_EnumAttr(Dee_TYPE(obj), obj, &filter,
				                               &libjson_writer_writeattr, self) < 0)
					goto err;
			}
			self->djw_obj = old_obj;
			if unlikely(libjson_writer_endobject(&self->djw_writer))
				goto err;
		}
		if unlikely(DeeJsonWriter_RemoveActive(self, obj) < 0)
			goto err;
	}
	return 0;
err:
	return -1;
}



PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
json_parser_parse_maybe_into(DeeJsonParser *__restrict self,
                             /*nullable*/ DeeObject *into) {
	/* Deal with special case of parsing JSON *into* an object. */
	if (into) {
		if (DeeType_Check(into))
			return DeeJson_ParseIntoType(self, (DeeTypeObject *)into, false, true);
		if unlikely(DeeJson_ParseInto(self, into, false, true))
			goto err;
		return_reference_(into);
	}

	/* Default case: parse JSON and convert to its canonical deemon form. */
	return DeeJson_ParseObject(self, false);
err:
	return NULL;
}

/*[[[deemon (print_KwCMethod from rt.gen.unpack)("parse", """
	DeeObject *data:?X4?DFile?DBytes?Dstring?DMapping;
	DeeObject *into = NULL;
""", methodFlags: "METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES");]]]*/
#define libjson_parse_params "data:?X4?DFile?DBytes?Dstring?DMapping,into?"
FORCELOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL libjson_parse_f_impl(DeeObject *data, DeeObject *into);
#ifndef DEFINED_kwlist__data_into
#define DEFINED_kwlist__data_into
PRIVATE DEFINE_KWLIST(kwlist__data_into, { KEX("data", 0x3af4b6d3, 0xb0164401a9853128), KEX("into", 0xc932469b, 0x46e544c708586600), KEND });
#endif /* !DEFINED_kwlist__data_into */
PRIVATE WUNUSED DREF DeeObject *DCALL libjson_parse_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *data;
		DeeObject *into;
	} args;
	args.into = NULL;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__data_into, "o|o:parse", &args))
		goto err;
	return libjson_parse_f_impl(args.data, args.into);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libjson_parse, &libjson_parse_f, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES);
FORCELOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL libjson_parse_f_impl(DeeObject *data, DeeObject *into)
/*[[[end]]]*/
{
	DREF DeeObject *result;
	DeeJsonParser parser;
	if (DeeBytes_Check(data)) {
		/* Parse raw bytes as JSON. */
		void *start, *end;
		DeeBytes_IncUse(data);
		start = DeeBytes_DATA(data);
		end   = DeeBytes_END(data);
		libjson_parser_init(&parser.djp_parser, start, end);
		parser.djp_owner = data;
		result = json_parser_parse_maybe_into(&parser, into);
		json_parser_fini(&parser.djp_parser);
		DeeBytes_DecUse(data);
	} else if (DeeString_Check(data)) {
		/* Parse a given string as JSON. */
		void const *start, *end;
		DeeStringObject *input = (DeeStringObject *)data;
		if (input->s_data) {
			struct Dee_string_utf *utf = input->s_data;
			/* JSON is smart enough to automatically detect larger encodings,
			 * so in the case of larger-than-utf-8, we can just pass with
			 * width-representation of the string.
			 * 
			 * Still though, if the string already has a utf-8 cache, then we
			 * always use that one (simply because it's more compact, as well
			 * as because libjson originating from KOS, is primarily optimized
			 * for processing UTF-8 data) */
			if (utf->u_utf8) {
				start = utf->u_utf8;
				end   = (char const *)start + WSTR_LENGTH(start);
			} else {
				size_t length;
				start  = utf->u_data[utf->u_width];
				length = WSTR_LENGTH(start);
				length = Dee_STRING_MUL_SIZEOF_WIDTH(length, utf->u_width);
				end    = (byte_t const *)start + length;
			}
		} else {
			char const *utf8 = DeeString_AsUtf8((DeeObject *)input);
			if unlikely(!utf8)
				goto err;
			start = utf8;
			end   = utf8 + WSTR_LENGTH(utf8);
		}
		libjson_parser_init(&parser.djp_parser, start, end);
		parser.djp_owner = data;
		result = json_parser_parse_maybe_into(&parser, into);
		json_parser_fini(&parser.djp_parser);
	} else if (DeeFile_Check(data)) {
		void *start, *end;
		data = DeeFile_ReadBytes(data, (size_t)-1, true);
		if unlikely(!data)
			goto err;
		DeeBytes_IncUse(data);
		start = DeeBytes_DATA(data);
		end   = DeeBytes_END(data);
		libjson_parser_init(&parser.djp_parser, start, end);
		parser.djp_owner = data;
		result = json_parser_parse_maybe_into(&parser, into);
		json_parser_fini(&parser.djp_parser);
		DeeBytes_DecUse(data);
		Dee_Decref_likely(data);
	} else {
		/* Convert mapping to DTO-like object. */
		int error;
		if (!into) {
			if (DeeObject_AssertType(data, &DeeMapping_Type))
				goto err;
			return_reference_(data);
		}
		if (DeeType_Check(into)) {
			result = DeeObject_NewDefault((DeeTypeObject *)into);
		} else {
			result = into;
			Dee_Incref(result);
		}
		error = DeeObject_PopulateFromMapping(data, result);
		if unlikely(error != 0) {
			Dee_Decref(result);
			goto err;
		}
	}
	return result;
err:
	return NULL;
}

/*[[[deemon (print_KwCMethod from rt.gen.unpack)("write", """
	DeeObject *data:?X8?O?Dfloat?Dint?Dstring?Dbool?N?DSequence?DMapping;
	DeeObject *into:?DFile = NULL;
	bool pretty = false;
	DeeObject *recursion:?X2?DCallable?N = Dee_None;
""", methodFlags: "METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES");]]]*/
#define libjson_write_params "data:?X8?O?Dfloat?Dint?Dstring?Dbool?N?DSequence?DMapping,into?:?DFile,pretty=!f,recursion:?X2?DCallable?N=!N"
FORCELOCAL WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL libjson_write_f_impl(DeeObject *data, DeeObject *into, bool pretty, DeeObject *recursion);
#ifndef DEFINED_kwlist__data_into_pretty_recursion
#define DEFINED_kwlist__data_into_pretty_recursion
PRIVATE DEFINE_KWLIST(kwlist__data_into_pretty_recursion, { KEX("data", 0x3af4b6d3, 0xb0164401a9853128), KEX("into", 0xc932469b, 0x46e544c708586600), KEX("pretty", 0x86358140, 0xafda632567fd2329), KEX("recursion", 0xf4d902ea, 0xf6191b6563337389), KEND });
#endif /* !DEFINED_kwlist__data_into_pretty_recursion */
PRIVATE WUNUSED DREF DeeObject *DCALL libjson_write_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *data;
		DeeObject *into;
		bool pretty;
		DeeObject *recursion;
	} args;
	args.into = NULL;
	args.pretty = false;
	args.recursion = Dee_None;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__data_into_pretty_recursion, "o|obo:write", &args))
		goto err;
	return libjson_write_f_impl(args.data, args.into, args.pretty, args.recursion);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libjson_write, &libjson_write_f, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES);
FORCELOCAL WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL libjson_write_f_impl(DeeObject *data, DeeObject *into, bool pretty, DeeObject *recursion)
/*[[[end]]]*/
{
	int error;
	DeeJsonWriter writer;
	unsigned int format;
	writer.djw_recursion = recursion;
#if JSON_WRITER_FORMAT_COMPACT == 0 && JSON_WRITER_FORMAT_PRETTY == 1
	format = (unsigned int)pretty;
#else /* JSON_WRITER_FORMAT_COMPACT == 0 && JSON_WRITER_FORMAT_PRETTY == 1 */
	format = JSON_WRITER_FORMAT_COMPACT;
	if (pretty)
		format = JSON_WRITER_FORMAT_PRETTY;
#endif /* JSON_WRITER_FORMAT_COMPACT != 0 || JSON_WRITER_FORMAT_PRETTY != 1 */

	/* Produce JSON data, either writing it to a file, or to a string. */
	if (into) {
		if (DeeJsonWriter_Init(&writer, (Dee_formatprinter_t)&DeeFile_WriteAll, into, format))
			goto err;
		error = DeeJson_WriteObject(&writer, data);
		DeeJsonWriter_Fini(&writer);
		if unlikely(error != 0)
			goto err;
		return_reference_(into);
	} else {
		struct Dee_ascii_printer printer = Dee_ASCII_PRINTER_INIT;
		if (DeeJsonWriter_Init(&writer, &Dee_ascii_printer_print, &printer, format))
			goto err_ascii_printer;
		error = DeeJson_WriteObject(&writer, data);
		DeeJsonWriter_Fini(&writer);
		if unlikely(error != 0)
			goto err_ascii_printer;
		return Dee_ascii_printer_pack(&printer);
err_ascii_printer:
		Dee_ascii_printer_fini(&printer);
		/* fallthru to `err' */
	}
err:
	return NULL;
}


DEX_BEGIN

/* TODO: Add another argument `path' that allows you to only parse certain sub-
 *       components of a larger JSON-blob. For this, it's probably best to implement
 *       a sub-set of JsonPath: https://github.com/json-path/JsonPath */
DEX_MEMBER_F("parse", &libjson_parse, Dee_DEXSYM_READONLY,
             "(data:?X4?DFile?DBytes?Dstring?DMapping)->?X7?Dfloat?Dint?Dstring?Dbool?N?GSequence?GMapping\n"
             "(data:?X4?DFile?DBytes?Dstring?DMapping,into)->\n"
             "(data:?X4?DFile?DBytes?Dstring?DMapping,into:?DType)->\n"
             "Parse JSON @data and convert it either into its native deemon representation, or "
             /**/ "use it to populate the fields of an object @into, or by creating a new instance "
             /**/ "of a ?DType @into (by calling its construct without any arguments), filling its "
             /**/ "fields, and then returning said instance.\n"
             "Additionally, a ?DMapping object can be given as data which then represents already-"
             /**/ "process JSON data. Using this, you can fill in DTO objects from sources other "
             /**/ "than JSON blobs\n"
             "This function may either ignore extra data that comes after the parsed JSON-construct, "
             /**/ "or it may throw an error should such data exist. The exact behavior is implementation-"
             /**/ "specific. Though if no exception is thrown, such trailing data is ignored and has no "
             /**/ "effect."),
DEX_MEMBER_F("write", &libjson_write, Dee_DEXSYM_READONLY,
             "(data:?X8?O?Dfloat?Dint?Dstring?Dbool?N?DSequence?DMapping,pretty=!f,recursion:?X2?DCallable?N=!N)->?Dstring\n"
             "(data:?X8?O?Dfloat?Dint?Dstring?Dbool?N?DSequence?DMapping,into:?DFile,pretty=!f,recursion:?X2?DCallable?N=!N)->?DFile\n"
             "#precursion{An optional callback that is invoked to replace inner instances of objects referencing "
             /*       */ "themselves via some attribute. When set to ?N, or if the object returned by the callback, "
             /*       */ "is also currently being written, a :ValueError is thrown instead.}"
             "Convert a native deemon object @data into JSON and write said JSON to @into, or pack it "
             /**/ "into a string which is then returned. In either case, you can use @pretty to specify "
             /**/ "if a pretty representation (using newlines, and indentation), or a compact one should "
             /**/ "be used in generated JSON. The default is to generate compact JSON."),
DEX_MEMBER_F_NODOC("Sequence", &DeeJsonSequence_Type, Dee_DEXSYM_READONLY),
DEX_MEMBER_F_NODOC("Mapping", &DeeJsonMapping_Type, Dee_DEXSYM_READONLY),

DEX_END(NULL, NULL, NULL);

DECL_END

#endif /* !GUARD_DEX_JSON_LIBJSON_C */
