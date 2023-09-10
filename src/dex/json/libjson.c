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
#ifndef GUARD_DEX_JSON_LIBJSON_C
#define GUARD_DEX_JSON_LIBJSON_C 1
#define CONFIG_BUILDING_LIBWIN32
#define DEE_SOURCE

#include "libjson.h"
/**/

#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/bytes.h>
#include <deemon/class.h>
#include <deemon/error.h>
#include <deemon/file.h>
#include <deemon/float.h>
#include <deemon/hashset.h>
#include <deemon/list.h>
#include <deemon/map.h>
#include <deemon/mapfile.h>
#include <deemon/none.h>
#include <deemon/numeric.h>
#include <deemon/objmethod.h>
#include <deemon/seq.h>
#include <deemon/set.h>
#include <deemon/string.h>
#include <deemon/stringutils.h>
#include <deemon/system-features.h>
#include <deemon/tuple.h>
#include <deemon/util/objectlist.h>

#include <hybrid/byteswap.h>

#include <stdint.h>

/* Enable KOS compatibility emulation */
#include <deemon/util/kos-compat.h>

/* Define everything with PRIVATE scoping. */
#undef INTDEF
#undef INTERN
#undef DEFINE_PUBLIC_ALIAS
#define INTDEF PRIVATE
#define INTERN PRIVATE
#define DEFINE_PUBLIC_ALIAS(new, old) /* Disable exports */

/* clang-format off */
#include "../../libjson/parser.c" /* 1 */
#include "../../libjson/writer.c" /* 2 */
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

PRIVATE WUNUSED NONNULL((1)) int DCALL
jseqiter_init(DeeJsonIteratorObject *__restrict self,
              size_t argc, DeeObject *const *argv) {
	DeeJsonSequenceObject *seq;
	if (DeeArg_Unpack(argc, argv, "o:_JsonSequenceIterator", &seq))
		goto err;
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
	if (DeeArg_Unpack(argc, argv, "o:_JsonMappingIterator", &seq))
		goto err;
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

	/* Try to write-back the new parser position. */
	if (!atomic_cmpxch_or_write(&self->ji_parser.jp_pos, orig_pos,
	                            parser.djp_parser.jp_pos)) {
		Dee_Decref(result);
		goto again;
	}
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
jmapiter_next(DeeJsonIteratorObject *__restrict self) {
	int tok;
	char const *orig_pos, *temp;
	DREF DeeObject *key_and_value[2], *result;
	DeeJsonParser parser;
again:
	DeeJsonIterator_GetParserEx(self, &parser);
	orig_pos = parser.djp_parser.jp_pos;
	if (libjson_parser_peeknext(&parser.djp_parser) == JSON_PARSER_ENDOBJECT)
		return ITER_DONE;
	key_and_value[0] = DeeJson_ParseString(&parser.djp_parser);
	if unlikely(!key_and_value[0])
		goto err;
	if (libjson_parser_yield(&parser.djp_parser) != JSON_PARSER_COLON)
		goto err_key_syntax;
	key_and_value[1] = DeeJson_ParseObject(&parser, true);
	if unlikely(!key_and_value[1])
		goto err_key;
	result = DeeTuple_NewVectorSymbolic(2, key_and_value);
	if unlikely(!result)
		goto err_key_value;

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
		Dee_Decref(result);
		goto again;
	}
	return result;
err_key_value_syntax:
	err_json_syntax();
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

#define DEFINE_JSON_ITERATOR_COMPARE(name, cmp)                       \
	PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL             \
	name(DeeJsonIteratorObject *self, DeeJsonIteratorObject *other) { \
		char const *lhs_pos, *rhs_pos;                                \
		if (DeeObject_AssertTypeExact(other, Dee_TYPE(self)))         \
			goto err;                                                 \
		lhs_pos = atomic_read(&self->ji_parser.jp_pos);               \
		rhs_pos = atomic_read(&other->ji_parser.jp_pos);              \
		return_bool(lhs_pos cmp rhs_pos);                             \
	err:                                                              \
		return NULL;                                                  \
	}
DEFINE_JSON_ITERATOR_COMPARE(jiter_eq, ==)
DEFINE_JSON_ITERATOR_COMPARE(jiter_ne, !=)
DEFINE_JSON_ITERATOR_COMPARE(jiter_lo, <)
DEFINE_JSON_ITERATOR_COMPARE(jiter_le, <=)
DEFINE_JSON_ITERATOR_COMPARE(jiter_gr, >)
DEFINE_JSON_ITERATOR_COMPARE(jiter_ge, >=)
#undef DEFINE_JSON_ITERATOR_COMPARE

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
	/* .nii_class = */ TYPE_ITERX_CLASS_BIDIRECTIONAL,
	/* .nii_flags = */ TYPE_ITERX_FNORMAL,
	{
		/* .nii_common = */ {
			/* .nii_getseq   = */ (dfunptr_t)&jseqiter_getseq,
			/* .nii_getindex = */ (dfunptr_t)NULL,
			/* .nii_setindex = */ (dfunptr_t)NULL,
			/* .nii_rewind   = */ (dfunptr_t)&jseqiter_nii_rewind,
			/* .nii_revert   = */ (dfunptr_t)NULL,
			/* .nii_advance  = */ (dfunptr_t)NULL,
			/* .nii_prev     = */ (dfunptr_t)&jiter_nii_prev,
			/* .nii_next     = */ (dfunptr_t)&jiter_nii_next,
			/* .nii_hasprev  = */ (dfunptr_t)&jseqiter_nii_hasprev,
			/* .nii_peek     = */ (dfunptr_t)&jseqiter_nii_peek
		}
	}
};

PRIVATE struct type_nii tpconst jmapiter_nii = {
	/* .nii_class = */ TYPE_ITERX_CLASS_BIDIRECTIONAL,
	/* .nii_flags = */ TYPE_ITERX_FNORMAL,
	{
		/* .nii_common = */ {
			/* .nii_getseq   = */ (dfunptr_t)&jmapiter_getseq,
			/* .nii_getindex = */ (dfunptr_t)NULL,
			/* .nii_setindex = */ (dfunptr_t)NULL,
			/* .nii_rewind   = */ (dfunptr_t)&jmapiter_nii_rewind,
			/* .nii_revert   = */ (dfunptr_t)NULL,
			/* .nii_advance  = */ (dfunptr_t)NULL,
			/* .nii_prev     = */ (dfunptr_t)&jiter_nii_prev,
			/* .nii_next     = */ (dfunptr_t)&jiter_nii_next,
			/* .nii_hasprev  = */ (dfunptr_t)&jmapiter_nii_hasprev,
			/* .nii_peek     = */ (dfunptr_t)&jmapiter_nii_peek
		}
	}
};

PRIVATE struct type_cmp jseqiter_cmp = {
	/* .tp_hash = */ NULL,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&jiter_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&jiter_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&jiter_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&jiter_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&jiter_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&jiter_ge,
	/* .tp_nii  = */ &jseqiter_nii,
};

PRIVATE struct type_cmp jmapiter_cmp = {
	/* .tp_hash = */ NULL,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&jiter_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&jiter_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&jiter_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&jiter_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&jiter_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&jiter_ge,
	/* .tp_nii  = */ &jmapiter_nii,
};

PRIVATE struct type_getset tpconst jseqiter_getsets[] = {
	TYPE_GETTER("seq", &jseqiter_getseq, "->?GSequence"),
	TYPE_GETSET_END
};

PRIVATE struct type_getset tpconst jmapiter_getsets[] = {
	TYPE_GETTER("seq", &jmapiter_getseq, "->?GMapping"),
	TYPE_GETSET_END
};


PRIVATE NONNULL((1)) int DCALL
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

PRIVATE NONNULL((1, 2)) int DCALL
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

PRIVATE NONNULL((1)) int DCALL
jmap_ctor(DeeJsonMappingObject *__restrict self) {
	self->jm_owner              = DeeNone_NewRef();
	self->jm_parser.jp_encoding = JSON_ENCODING_UTF8;
	self->jm_parser.jp_start    = empty_json_object;
	self->jm_parser.jp_pos      = empty_json_object + 1;
	self->jm_parser.jp_end      = empty_json_object + 2;
	Dee_atomic_rwlock_init(&self->jm_lock);
	return 0;
}

PRIVATE NONNULL((1, 2)) int DCALL
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
PRIVATE NONNULL((1, 2)) int DCALL
jseq_or_map_init_parser(DeeJsonSequenceObject *__restrict self,
                        DeeObject *__restrict data) {
	if (DeeBytes_Check(data)) {
		/* Parse raw bytes as JSON. */
		void *start = DeeBytes_DATA(data);
		void *end   = DeeBytes_TERM(data);
		libjson_parser_init(&self->js_parser, start, end);
		Dee_Incref(data);
	} else if (DeeFile_Check(data)) {
		void *start, *end;
		data = DeeFile_ReadBytes(data, (size_t)-1, true);
		if unlikely(!data)
			goto err;
		start = DeeBytes_DATA(data);
		end   = DeeBytes_TERM(data);
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
				end   = (char *)start + WSTR_LENGTH(start);
			} else {
				size_t length;
				start  = utf->u_data[utf->u_width];
				length = WSTR_LENGTH(start);
				length = Dee_STRING_MUL_SIZEOF_WIDTH(length, utf->u_width);
				end    = (byte_t *)start + length;
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

PRIVATE NONNULL((1)) int DCALL
jseq_init(DeeJsonSequenceObject *__restrict self,
          size_t argc, DeeObject *const *argv) {
	DeeObject *data;
	if (DeeArg_Unpack(argc, argv, "o:_JsonSequence", &data))
		goto err;
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

PRIVATE NONNULL((1)) int DCALL
jmap_init(DeeJsonMappingObject *__restrict self,
          size_t argc, DeeObject *const *argv) {
	DeeObject *data;
	if (DeeArg_Unpack(argc, argv, "o:_JsonMapping", &data))
		goto err;
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


PRIVATE NONNULL((1)) int DCALL
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

PRIVATE NONNULL((1)) int DCALL
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
jseq_nsi_getsize(DeeJsonSequenceObject *__restrict self) {
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
	while ((tok = libjson_parser_next(&parser)) == JSON_ERROR_OK)
		++result;
	if unlikely(tok == JSON_ERROR_SYNTAX)
		goto err_syntax;
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
jseq_nsi_getitem(DeeJsonSequenceObject *__restrict self, size_t index) {
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
	DeeError_Throwf(&DeeError_IndexError,
	                "Index `%" PRFuSIZ "' lies outside the valid bounds "
	                "`0...%" PRFuSIZ "' of sequence of type `%k'",
	                index, self->js_size, Dee_TYPE(self));
	goto err;
err_syntax_r:
	Dee_Decref(result);
err_syntax:
	err_json_syntax();
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
jmap_nsi_getsize(DeeJsonMappingObject *__restrict self) {
	int tok;
	size_t result;
	struct json_parser parser;
	DeeJsonMapping_LockRead(self);
	parser = self->jm_parser;
	DeeJsonMapping_LockEndRead(self);
	if unlikely(libjson_parser_rewind(&parser) != JSON_PARSER_OBJECT)
		goto err_syntax;
	result = 0;
	while ((tok = libjson_parser_next(&parser)) == JSON_ERROR_OK)
		++result;
	if unlikely(tok == JSON_ERROR_SYNTAX)
		goto err_syntax;
	return result;
err_syntax:
	return (size_t)err_json_syntax();
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
jmap_nsi_getdefault(DeeJsonMappingObject *self,
                    DeeObject *key, DeeObject *defl) {
	int error;
	DeeJsonParser parser;
	DREF DeeObject *result;
	char const *keystr;
	if (DeeObject_AssertTypeExact(key, &DeeString_Type))
		goto err;
	keystr = DeeString_AsUtf8(key);
	DeeJsonMapping_LockRead(self);
	parser.djp_parser = self->jm_parser;
	DeeJsonMapping_LockEndRead(self);
	error = libjson_parser_findkey(&parser.djp_parser, keystr, WSTR_LENGTH(keystr));
	if unlikely(error == JSON_ERROR_SYNTAX)
		goto err_syntax;
	if (error == JSON_ERROR_NOOBJ) {
		if (defl != ITER_DONE)
			Dee_Incref(defl);
		return defl;
	}

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


PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
jmap_get(DeeJsonMappingObject *self, DeeObject *key) {
	DREF DeeObject *result = jmap_nsi_getdefault(self, key, ITER_DONE);
	if (result == ITER_DONE) {
		DeeError_Throwf(&DeeError_KeyError,
		                "Could not find key `%k' in %k `%k'",
		                key, Dee_TYPE(self), self);
		result = NULL;
	}
	return result;
}


PRIVATE struct type_nsi tpconst jseq_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_SEQ,
	/* .nsi_flags   = */ TYPE_SEQX_FNORMAL,
	{
		/* .nsi_seqlike = */ {
			/* .nsi_getsize      = */ (dfunptr_t)&jseq_nsi_getsize,
			/* .nsi_getsize_fast = */ (dfunptr_t)NULL,
			/* .nsi_getitem      = */ (dfunptr_t)&jseq_nsi_getitem,
			/* .nsi_delitem      = */ (dfunptr_t)NULL,
			/* .nsi_setitem      = */ (dfunptr_t)NULL,
			/* .nsi_getitem_fast = */ (dfunptr_t)NULL,
			/* .nsi_getrange     = */ (dfunptr_t)NULL,
			/* .nsi_getrange_n   = */ (dfunptr_t)NULL,
			/* .nsi_setrange     = */ (dfunptr_t)NULL,
			/* .nsi_setrange_n   = */ (dfunptr_t)NULL,
			/* .nsi_find         = */ (dfunptr_t)NULL,
			/* .nsi_rfind        = */ (dfunptr_t)NULL,
			/* .nsi_xch          = */ (dfunptr_t)NULL,
			/* .nsi_insert       = */ (dfunptr_t)NULL,
			/* .nsi_insertall    = */ (dfunptr_t)NULL,
			/* .nsi_insertvec    = */ (dfunptr_t)NULL,
			/* .nsi_pop          = */ (dfunptr_t)NULL,
			/* .nsi_erase        = */ (dfunptr_t)NULL,
			/* .nsi_remove       = */ (dfunptr_t)NULL,
			/* .nsi_rremove      = */ (dfunptr_t)NULL,
			/* .nsi_removeall    = */ (dfunptr_t)NULL,
			/* .nsi_removeif     = */ (dfunptr_t)NULL
		}
	}
};

PRIVATE struct type_nsi tpconst jmap_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_MAP,
	/* .nsi_flags   = */ TYPE_SEQX_FNORMAL,
	{
		/* .nsi_maplike = */ {
			/* .nsi_getsize    = */ (dfunptr_t)&jmap_nsi_getsize,
			/* .nsi_nextkey    = */ (dfunptr_t)&jmapiter_nextkey,
			/* .nsi_nextvalue  = */ (dfunptr_t)&jmapiter_nextvalue,
			/* .nsi_getdefault = */ (dfunptr_t)&jmap_nsi_getdefault
		}
	}
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
jseq_size(DeeJsonSequenceObject *__restrict self) {
	size_t result = jseq_nsi_getsize(self);
	if unlikely(result == (size_t)-1)
		goto err;
	return DeeInt_NewSize(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
jmap_size(DeeJsonMappingObject *__restrict self) {
	size_t result = jmap_nsi_getsize(self);
	if unlikely(result == (size_t)-1)
		goto err;
	return DeeInt_NewSize(result);
err:
	return NULL;
}

PRIVATE struct type_seq jseq_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&jseq_iter,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&jseq_size,
	/* .tp_contains  = */ NULL,
	/* .tp_get       = */ NULL,
	/* .tp_del       = */ NULL,
	/* .tp_set       = */ NULL,
	/* .tp_range_get = */ NULL,
	/* .tp_range_del = */ NULL,
	/* .tp_range_set = */ NULL,
	/* .tp_nsi       = */ &jseq_nsi
};

PRIVATE struct type_seq jmap_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&jmap_iter,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&jmap_size,
	/* .tp_contains  = */ NULL,
	/* .tp_get       = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&jmap_get,
	/* .tp_del       = */ NULL,
	/* .tp_set       = */ NULL,
	/* .tp_range_get = */ NULL,
	/* .tp_range_del = */ NULL,
	/* .tp_range_set = */ NULL,
	/* .tp_nsi       = */ &jmap_nsi
};

PRIVATE NONNULL((1)) void DCALL
jseq_fini(DeeJsonSequenceObject *__restrict self) {
	Dee_Decref(self->js_owner);
}

PRIVATE NONNULL((1, 2)) void DCALL
jseq_visit(DeeJsonSequenceObject *__restrict self,
           dvisit_t proc, void *arg) {
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
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&jseqiter_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&jseqiter_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&jseqiter_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&jseqiter_init,
				TYPE_FIXED_ALLOCATOR(DeeJsonIteratorObject)
			}
		},
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
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&jseqiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &jseqiter_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&jseqiter_next,
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
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&jmapiter_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&jmapiter_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&jmapiter_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&jmapiter_init,
				TYPE_FIXED_ALLOCATOR(DeeJsonIteratorObject)
			}
		},
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
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&jmapiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &jmapiter_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&jmapiter_next,
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
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&jseq_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&jseq_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&jseq_copy,
				/* .tp_any_ctor  = */ (dfunptr_t)&jseq_init,
				TYPE_FIXED_ALLOCATOR(DeeJsonSequenceObject)
			}
		},
		/* .tp_dtor = */ (void (DCALL *)(DeeObject *__restrict))&jseq_fini
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&jseq_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&jseq_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &jseq_seq,
	/* .tp_iter_next     = */ NULL,
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
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&jmap_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&jmap_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&jmap_copy,
				/* .tp_any_ctor  = */ (dfunptr_t)&jmap_init,
				TYPE_FIXED_ALLOCATOR(DeeJsonMappingObject)
			}
		},
		/* .tp_dtor = */ (void (DCALL *)(DeeObject *__restrict))&jmap_fini
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&jmap_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&jmap_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &jmap_seq,
	/* .tp_iter_next     = */ NULL,
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
		result = (DREF DeeObject *)DeeJsonMapping_New(self, must_advance_parser);
		break;

	case JSON_PARSER_ARRAY:
		result = (DREF DeeObject *)DeeJsonSequence_New(self, must_advance_parser);
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
	dssize_t error;
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	/* Print the JSON string into a unicode-printer to convert it into a deemon string. */
	status = libjson_parser_printstring(self, &unicode_printer_print, &printer, &error);
	if (status != JSON_ERROR_OK) {
		unicode_printer_fini(&printer);
		if (status == JSON_ERROR_SYSERR)
			goto err;    /* `unicode_printer_print()' returned a negative value. */
		goto err_syntax; /* Either a *true* syntax error, or current token isn't a string. */
	}
	return unicode_printer_pack(&printer);
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
		                           DEEINT_STRING(0, DEEINT_STRING_FNOSEPS));
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
		                           DEEINT_STRING(0, DEEINT_STRING_FNOSEPS));
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
			return_reference_(DeeInt_Zero);
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
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	char const *iter, *end, *flush_start;

	/* Parse the string and unescape special symbols. */
	iter = self->ten_start;
	end  = self->ten_end;
	flush_start = iter;
	while (iter < end) {
		char ch = *iter++;
		if (ch == '\\') { /* Remove every first '\'-character */
			if unlikely(unicode_printer_print(&printer, flush_start,
			                                  (size_t)((iter - 1) - flush_start)) < 0)
				goto err_printer;
			flush_start = iter;
			if (iter < end)
				++iter; /* Don't remove the character following '\', even if it's another '\' */
		}
	}
	if (flush_start < end) {
		if unlikely(unicode_printer_print(&printer, flush_start,
		                                  (size_t)(end - flush_start)) < 0)
			goto err_printer;
	}

	/* Pack the unicode string */
	self->ten_str = (DREF DeeStringObject *)unicode_printer_pack(&printer);
	if unlikely(!self->ten_str)
		goto err;
	self->ten_start = DeeString_AsUtf8((DeeObject *)self->ten_str);
	if unlikely(!self->ten_start)
		goto err_ten_str;
	self->ten_end = self->ten_start + WSTR_LENGTH(self->ten_start);
	return 0;
err_ten_str:
	Dee_Decref(self->ten_str);
	goto err;
err_printer:
	unicode_printer_fini(&printer);
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
			base = (DREF DeeObject *)self->tep_decl_type;
			Dee_Incref(base);
			break;

		case 'D':
			base = (DREF DeeObject *)DeeModule_GetDeemon();
			Dee_Incref(base);
			break;

		case 'G':
			base = DeeType_GetModule(self->tep_decl_type);
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
			result = (DREF DeeTypeObject *)DeeModule_GetExtern((DeeObject *)name.ten_str,
			                                                   (DeeObject *)export_name.ten_str);
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
			base = (DREF DeeObject *)type_expression_parser_parsetype(self, true);
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
			result = (DREF DeeTypeObject *)DeeObject_GetAttr(base, (DeeObject *)name.ten_str);
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
                                   struct objectlist *__restrict result,
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
			error = objectlist_append(result, item);
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
			error = objectlist_append(result, item);
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
		struct objectlist ol;
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
		objectlist_init(&ol);
		tok = DeeJsonObject_ParseTypedObjectList(self, tx_parser, &ol,
		                                         throw_error_if_typing_fails);
		tx_parser->tep_flags = saved_flags;
		if unlikely(tok != 0) {
			objectlist_fini(&ol);
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
			result = (DREF DeeListObject *)objectlist_packlist(&ol);
			if unlikely(!result) {
err_ol:
				objectlist_fini(&ol);
				goto err;
			}
			return (DREF DeeObject *)result;
		}

		/* Check for special case: Tuple */
		if (seq_type == &DeeTuple_Type) {
			DREF DeeTupleObject *result;
			Dee_DecrefNokill(seq_type);
			result = (DREF DeeTupleObject *)objectlist_packtuple(&ol);
			if unlikely(!result)
				goto err_ol;
			return (DREF DeeObject *)result;
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
			objectlist_fini(&ol);
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
		return (DREF DeeObject *)result;
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
                                      struct class_desc *into_attr_class,
                                      struct class_attribute *into_attr,
                                      bool throw_error_if_typing_fails) {
	int result;
	struct instance_desc *into_instance;
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
		struct class_desc *attr_class;
		struct class_attribute *attr;
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
			result = (DREF DeeObject *)DeeJsonMapping_New(self, must_advance_parser);
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
			result = (DREF DeeObject *)DeeJsonSequence_New(self, must_advance_parser);
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


PRIVATE WUNUSED NONNULL((2)) dssize_t DCALL
json_foreach_write_item(void *arg, DeeObject *elem) {
	DeeJsonWriter *me = (DeeJsonWriter *)arg;
	return DeeJson_WriteObject(me, elem);
}

PRIVATE WUNUSED NONNULL((2)) dssize_t DCALL
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

PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
json_foreach_write_attribute(DeeObject *declarator,
                             char const *attr_name, char const *attr_doc,
                             uint16_t perm, DeeTypeObject *attr_type, void *arg) {
	int error;
	DREF DeeObject *attr_value;
	DeeJsonWriter *me = (DeeJsonWriter *)arg;

	/* Unused arguments. */
	(void)declarator;
	(void)attr_doc;
	(void)attr_type;

	/* Skip attributes from low-level base classes. */
	if (declarator == (DeeObject *)&DeeObject_Type ||
	    declarator == (DeeObject *)&DeeType_Type) {
		if (declarator != me->djw_obj)
			return 0;
	}

	/* Check if we should include the attribute. For this purpose:
	 * - The attribute must not be private
	 * - The attribute must not be a property
	 * - The attribute must not be a function
	 * - The attribute must not be part of the class definition (i.e. must not be static)
	 * - The attribute must be readable */
	if ((perm & (Dee_ATTR_PRIVATE | Dee_ATTR_PROPERTY | Dee_ATTR_CMEMBER |
	             Dee_ATTR_PERMGET | Dee_ATTR_PERMCALL)) != (Dee_ATTR_PERMGET))
		return 0;
	if (perm & Dee_ATTR_NAMEOBJ) {
		DeeStringObject *str_name;
		str_name   = COMPILER_CONTAINER_OF(attr_name, DeeStringObject, s_str);
		attr_value = DeeObject_GetAttr(me->djw_obj, (DeeObject *)str_name);
	} else {
		attr_value = DeeObject_GetAttrString(me->djw_obj, attr_name);
	}
	if unlikely(!attr_value) {
		/* Unbound attributes simply aren't included in JSON blobs. */
		if (DeeError_Catch(&DeeError_UnboundAttribute))
			return 0;
		goto err;
	}

	/* All right! Let's write this attribute. */
	if (perm & Dee_ATTR_NAMEOBJ) {
		char const *name_utf8;
		DeeStringObject *str_name;
		str_name  = COMPILER_CONTAINER_OF(attr_name, DeeStringObject, s_str);
		name_utf8 = DeeString_AsUtf8((DeeObject *)str_name);
		if unlikely(!name_utf8)
			goto err_attr_value;
		error = libjson_writer_addfield(&me->djw_writer, name_utf8, WSTR_LENGTH(name_utf8));
	} else {
		error = libjson_writer_addfield(&me->djw_writer, attr_name, strlen(attr_name));
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
PRIVATE NONNULL((1, 2)) int DCALL
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
		dssize_t temp;
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
			if unlikely(DeeObject_EnumAttr(Dee_TYPE(obj), obj, &json_foreach_write_attribute, self) < 0)
				goto err;
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

PRIVATE struct keyword parse_kwlist[] = { K(data), K(into), KEND };
PRIVATE WUNUSED DREF DeeObject *DCALL
f_libjson_parse(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *result;
	DeeJsonParser parser;
	DeeObject *data, *into = NULL;
	if (DeeArg_UnpackKw(argc, argv, kw, parse_kwlist, "o|o:parse", &data, &into))
		goto err;
	if (DeeBytes_Check(data)) {
		/* Parse raw bytes as JSON. */
		void *start = DeeBytes_DATA(data);
		void *end   = DeeBytes_TERM(data);
		libjson_parser_init(&parser.djp_parser, start, end);
		parser.djp_owner = data;
		result = json_parser_parse_maybe_into(&parser, into);
		json_parser_fini(&parser.djp_parser);
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
				end   = (char *)start + WSTR_LENGTH(start);
			} else {
				size_t length;
				start  = utf->u_data[utf->u_width];
				length = WSTR_LENGTH(start);
				length = Dee_STRING_MUL_SIZEOF_WIDTH(length, utf->u_width);
				end    = (byte_t *)start + length;
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
		start = DeeBytes_DATA(data);
		end   = DeeBytes_TERM(data);
		libjson_parser_init(&parser.djp_parser, start, end);
		parser.djp_owner = data;
		result = json_parser_parse_maybe_into(&parser, into);
		json_parser_fini(&parser.djp_parser);
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

PRIVATE struct keyword write_kwlist[] = { K(data), K(into), K(pretty), K(recursion), KEND };
PRIVATE WUNUSED DREF DeeObject *DCALL
f_libjson_write(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	int error;
	DeeJsonWriter writer;
	bool pretty = false;
	unsigned int format;
	DeeObject *data, *into = NULL;
	writer.djw_recursion = Dee_None;
	if (DeeArg_UnpackKw(argc, argv, kw, write_kwlist, "o|obo:write",
	                    &data, &into, &pretty, &writer.djw_recursion))
		goto err;
#if JSON_WRITER_FORMAT_COMPACT == 0 && JSON_WRITER_FORMAT_PRETTY == 1
	format = (unsigned int)pretty;
#else /* JSON_WRITER_FORMAT_COMPACT == 0 && JSON_WRITER_FORMAT_PRETTY == 1 */
	format = JSON_WRITER_FORMAT_COMPACT;
	if (pretty)
		format = JSON_WRITER_FORMAT_PRETTY;
#endif /* JSON_WRITER_FORMAT_COMPACT != 0 || JSON_WRITER_FORMAT_PRETTY != 1 */

	/* Produce JSON data, either writing it to a file, or to a string. */
	if (into) {
		if (DeeJsonWriter_Init(&writer, (dformatprinter)&DeeFile_WriteAll, into, format))
			goto err;
		error = DeeJson_WriteObject(&writer, data);
		DeeJsonWriter_Fini(&writer);
		if unlikely(error != 0)
			goto err;
		return_reference_(into);
	} else {
		struct ascii_printer printer = ASCII_PRINTER_INIT;
		if (DeeJsonWriter_Init(&writer, &ascii_printer_print, &printer, format))
			goto err_ascii_printer;
		error = DeeJson_WriteObject(&writer, data);
		DeeJsonWriter_Fini(&writer);
		if unlikely(error != 0)
			goto err_ascii_printer;
		return ascii_printer_pack(&printer);
err_ascii_printer:
		ascii_printer_fini(&printer);
		/* fallthru to `err' */
	}
err:
	return NULL;
}

PRIVATE DEFINE_KWCMETHOD(libjson_parse, &f_libjson_parse);
PRIVATE DEFINE_KWCMETHOD(libjson_write, &f_libjson_write);


PRIVATE struct dex_symbol symbols[] = {
	{ "parse", (DeeObject *)&libjson_parse, MODSYM_FNORMAL,
	  /* TODO: Add another argument `path' that allows you to only parse certain sub-
	   *       components of a larger JSON-blob. For this, it's probably best to implement
	   *       a sub-set of JsonPath: https://github.com/json-path/JsonPath
	   */
	  DOC("(data:?X4?DFile?DBytes?Dstring?DMapping)->?X7?Dfloat?Dint?Dstring?Dbool?N?GSequence?GMapping\n"
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
	      /**/ "effect.") },
	{ "write", (DeeObject *)&libjson_write, MODSYM_FNORMAL,
	  DOC("(data:?X8?O?Dfloat?Dint?Dstring?Dbool?N?DSequence?DMapping,pretty=!f,recursion:?X2?DCallable?N=!N)->?Dstring\n"
	      "(data:?X8?O?Dfloat?Dint?Dstring?Dbool?N?DSequence?DMapping,into:?DFile,pretty=!f,recursion:?X2?DCallable?N=!N)->?DFile\n"
	      "#precursion{An optional callback that is invoked to replace inner instances of objects referencing "
	      /*       */ "themselves via some attribute. When set to ?N, or if the object returned by the callback, "
	      /*       */ "is also currently being written, a :ValueError is thrown instead.}"
	      "Convert a native deemon object @data into JSON and write said JSON to @into, or pack it "
	      /**/ "into a string which is then returned. In either case, you can use @pretty to specify "
	      /**/ "if a pretty representation (using newlines, and indentation), or a compact one should "
	      /**/ "be used in generated JSON. The default is to generate compact JSON.") },
	{ "Sequence", (DeeObject *)&DeeJsonSequence_Type },
	{ "Mapping", (DeeObject *)&DeeJsonMapping_Type },
	{ NULL }
};

PUBLIC struct dex DEX = {
	/* .d_symbols = */ symbols
};

DECL_END

#endif /* !GUARD_DEX_JSON_LIBJSON_C */
