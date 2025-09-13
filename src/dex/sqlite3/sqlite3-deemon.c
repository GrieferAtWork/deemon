/* Copyright (c) 2018-2025 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2025 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEX_SQLITE3_SQLITE3_DEEMON_H
#define GUARD_DEX_SQLITE3_SQLITE3_DEEMON_H 1
#define CONFIG_BUILDING_LIBSQLITE3
#define DEE_SOURCE

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/bool.h>
#include <deemon/bytes.h>
#include <deemon/error.h>
#include <deemon/float.h>
#include <deemon/int.h>
#include <deemon/method-hints.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/super.h>
#include <deemon/system-features.h>

#include <hybrid/typecore.h>

#include <stdint.h>

/**/
#include "sqlite3-external.h"
#include "libsqlite3.h"

DECL_BEGIN

PRIVATE void _dee_sqlite3_bind_string_decref(void *str) {
	DREF DeeStringObject *string;
	string = COMPILER_CONTAINER_OF((char *)str, DeeStringObject, s_str);
	Dee_Decref(string);
}

INTERN WUNUSED NONNULL((3)) int DCALL
dee_sqlite3_bind_string(sqlite3_stmt *stmt, int index, DeeStringObject *self) {
	int rc;
again:
	if (DeeString_STR_ISUTF8(self)) {
		/* Simple case: can directly pass `DeeString_STR()' */
		size_t length = DeeString_SIZE(self);
#if __SIZEOF_SIZE_T__ > 4
		rc = sqlite3_bind_text64(stmt, index, DeeString_STR(self), (sqlite3_uint64)length,
		                         &_dee_sqlite3_bind_string_decref, SQLITE_UTF8);
#else /* __SIZEOF_SIZE_T__ > 4 */
		rc = sqlite3_bind_text(stmt, index, DeeString_STR(self), (int)length,
		                       &_dee_sqlite3_bind_string_decref);
#endif /* __SIZEOF_SIZE_T__ <= 4 */
	} else {
		size_t length;
		char const *utf8 = DeeString_AsUtf8((DeeObject *)self);
		if unlikely(!utf8)
			goto err;
		length = WSTR_LENGTH(utf8);
#if __SIZEOF_SIZE_T__ > 4
		rc = sqlite3_bind_text64(stmt, index, utf8, (sqlite3_uint64)length, NULL, SQLITE_UTF8);
#else /* __SIZEOF_SIZE_T__ > 4 */
		rc = sqlite3_bind_text(stmt, index, utf8, (int)length, NULL);
#endif /* __SIZEOF_SIZE_T__ <= 4 */
	}
	if unlikely(rc != SQLITE_OK)
		goto err_rc;
	return 0;
err_rc:
	rc = err_sql_throwerror(rc, ERR_SQL_THROWERROR_F_ALLOW_RESTART);
	if (rc == 0)
		goto again;
	return rc;
err:
	return -1;
}

PRIVATE void _dee_sqlite3_bind_bytes_decref(void *str) {
	DREF DeeBytesObject *bytes;
	bytes = COMPILER_CONTAINER_OF((char *)str, DeeBytesObject, b_data);
	Dee_Decref(bytes);
}

INTERN WUNUSED NONNULL((3)) int DCALL
dee_sqlite3_bind_bytes(sqlite3_stmt *stmt, int index, DeeBytesObject *self) {
	int rc;
	__BYTE_TYPE__ *data;
	size_t size;
again:
	data = DeeBytes_DATA(self);
	size = DeeBytes_SIZE(self);
	if (data == self->b_data) {
		/* Simple case: can directly pass `data' */
#if __SIZEOF_SIZE_T__ > 4
		rc = sqlite3_bind_blob64(stmt, index, data, (sqlite3_uint64)size,
		                         &_dee_sqlite3_bind_bytes_decref);
#else /* __SIZEOF_SIZE_T__ > 4 */
		rc = sqlite3_bind_blob(stmt, index, data, (int)size,
		                       &_dee_sqlite3_bind_bytes_decref);
#endif /* __SIZEOF_SIZE_T__ <= 4 */
	} else {
#if __SIZEOF_SIZE_T__ > 4
		rc = sqlite3_bind_blob64(stmt, index, data, (sqlite3_uint64)size, NULL);
#else /* __SIZEOF_SIZE_T__ > 4 */
		rc = sqlite3_bind_blob(stmt, index, data, (int)size, NULL);
#endif /* __SIZEOF_SIZE_T__ <= 4 */
	}
	if unlikely(rc != SQLITE_OK)
		goto err_rc;
	return 0;
err_rc:
	rc = err_sql_throwerror(rc, ERR_SQL_THROWERROR_F_ALLOW_RESTART);
	if (rc == 0)
		goto again;
	return rc;
}


/* Bind "ob" to query parameter "index" of "stmt"
 * @return: 0 : Success
 * @return: -1: An error was thrown */
INTERN WUNUSED NONNULL((3)) int DCALL
dee_sqlite3_bind_object(sqlite3_stmt *stmt, int index, DeeObject *self) {
	int rc;
	DeeTypeObject *type = Dee_TYPE(self);
	if (type == &DeeSuper_Type) {
		type = DeeSuper_TYPE(self);
		self = DeeSuper_SELF(self);
	}
again:
	if (type == &DeeString_Type) {
		return dee_sqlite3_bind_string(stmt, index, (DeeStringObject *)self);
	} else if (type == &DeeInt_Type) {
		int64_t value;
		if unlikely(DeeObject_AsInt64(self, &value))
			goto err;
		rc = sqlite3_bind_int64(stmt, index, (sqlite3_int64)value);
	} else if (type == &DeeBool_Type) {
		rc = sqlite3_bind_int(stmt, index, DeeBool_IsTrue(self) ? 1 : 0);
	} else if (type == &DeeNone_Type) {
		rc = sqlite3_bind_null(stmt, index);
#ifdef CONFIG_HAVE_FPU
	} else if (type == &DeeFloat_Type) {
		rc = sqlite3_bind_double(stmt, index, DeeFloat_VALUE(self));
#endif /* CONFIG_HAVE_FPU */
	} else if (type == &DeeBytes_Type) {
		return dee_sqlite3_bind_bytes(stmt, index, (DeeBytesObject *)self);
	} else {
		/* TODO: Support for generic numeric types:
		 * >> if (self is Numeric) {
		 * >>     if (Numeric.isfloat(self)) {
		 * >>         return BIND((float)self);
		 * >>     } else {
		 * >>         return BIND((int)self);
		 * >>     }
		 * >> } */
		char const *param_name = sqlite3_bind_parameter_name(stmt, index);
		if (param_name) {
			return DeeError_Throwf(&DeeError_TypeError,
			                       "Cannot bind object of type %k as SQL query parameter %q",
			                       type, param_name);
		}
		return DeeError_Throwf(&DeeError_TypeError,
		                       "Cannot bind object of type %k as %d SQL query parameter",
		                       type, index);
	}
	if unlikely(rc != SQLITE_OK)
		goto err_rc;
	return 0;
err_rc:
	rc = err_sql_throwerror(rc, ERR_SQL_THROWERROR_F_ALLOW_RESTART);
	if (rc == 0)
		goto again;
	return rc;
err:
	return -1;
}

struct dee_sqlite3_bind_params_indexed_data {
	sqlite3_stmt *sbpid_stmt; /* Statement to populate */
	ptrdiff_t     sbpid_off;  /* Offset to convert sequence "index" to SQL parameter index. */
};

PRIVATE WUNUSED Dee_ssize_t DCALL
dee_sqlite3_bind_params_indexed(void *arg, size_t index, /*nullable*/ DeeObject *value) {
	if likely(value) {
		Dee_ssize_t result;
		struct dee_sqlite3_bind_params_indexed_data *data;
		data = (struct dee_sqlite3_bind_params_indexed_data *)arg;

		/* Params are 1-based, so have to add +1 to the (0-based) deemon sequence index. */
		result = dee_sqlite3_bind_object(data->sbpid_stmt, (int)(index + data->sbpid_off), value);
		if likely(result == 0)
			result = 1;
		return result;
	}
	return 0;
}


/* Custom function: like `sqlite3_bind_parameter_index', but "zName"
 * doesn't have to include the leading `:', `$' or `@'. */
SQLITE_API int sqlite3_bind_parameter_index__without_prefix(sqlite3_stmt*, const char *zName);

PRIVATE WUNUSED NONNULL((2, 3)) Dee_ssize_t DCALL
dee_sqlite3_bind_params_named(void *arg, DeeObject *key, DeeObject *value) {
	char const *utf8_name;
	int param_index;
	if (DeeObject_AssertTypeExact(key, &DeeString_Type))
		goto err;
	utf8_name = DeeString_AsUtf8(key);
	if unlikely(!utf8_name)
		goto err;
	param_index = sqlite3_bind_parameter_index__without_prefix((sqlite3_stmt *)arg, utf8_name);
	if unlikely(param_index == 0) {
		return DeeError_Throwf(&DeeError_ValueError,
		                       "SQL query param %q not found",
		                       utf8_name);
	}
	return dee_sqlite3_bind_object((sqlite3_stmt *)arg, param_index, value);
err:
	return -1;
}


/* Bind parameters from `params' to `stmt'. Depending on `stmt' using "?"
 * or ":foo" for referencing parameters, this function either requires
 * `params' to be `{Object...}' or `{string: Object}'. As such, deemon's
 * sqlite interface requires either all parameters to be named, or all
 * parameters to be unnamed (when there are no parameters, we simply
 * assert that "params" is an empty sequence)
 *
 * @param: unnamed_start: Starting offset for unnamed parameters
 * @return: -1: An error was thrown
 * @return: * : When ?-parameters were used, the # parameters consumed
 *              (future calls should increment "unnamed_start" by this value)
 * @return: * : When ?-parameters were not used, ">= 0" indicates success */
INTERN WUNUSED NONNULL((2)) Dee_ssize_t DCALL
dee_sqlite3_bind_params(sqlite3_stmt *stmt, DeeObject *params, size_t unnamed_start) {
	Dee_ssize_t fe_status;
	char const *greatest_name;
	int greatest_index = sqlite3_bind_parameter_count(stmt);
	if (greatest_index == 0)
		return 0; /* No params to consume */

	/* Figure out the name of the last SQL parameter */
	greatest_name = sqlite3_bind_parameter_name(stmt, greatest_index);
	if (greatest_name == NULL) {
		/* Assume (read: require) that params are unnamed/indexed
		 * -> enumerate+bind using `seq_enumerate_index()' */
		struct dee_sqlite3_bind_params_indexed_data data;
do_enumerate_params:
		data.sbpid_stmt = stmt;
		data.sbpid_off  = (ptrdiff_t)1 - (ptrdiff_t)unnamed_start;
		fe_status = DeeObject_InvokeMethodHint(seq_enumerate_index, params,
		                                       &dee_sqlite3_bind_params_indexed, &data,
		                                       unnamed_start,
		                                       unnamed_start + greatest_index);
	} else if (*greatest_name == '?') {
		/* Parameters are explicitly indexed, though possibly with gaps */
		unnamed_start = 0;
		goto do_enumerate_params;
	} else {
		/* Assume (read: require) that all parameters are named
		 * -> enumerate+bind using `map_operator_foreach_pair()' */
		fe_status = DeeObject_InvokeMethodHint(map_operator_foreach_pair, params,
		                                       &dee_sqlite3_bind_params_named,
		                                       stmt);
	}
	/* NOTE: We don't really assert that all query params
	 *       are bound, since there's no easy way to do that. */
	return fe_status;
}


/* Return column "col" as a deemon string object */
INTERN WUNUSED DREF DeeStringObject *DCALL
dee_sqlite3_column_string(sqlite3_stmt *stmt, int col) {
	DREF DeeStringObject *result;
	unsigned char const *text;
	int textlen;
again:
	text = sqlite3_column_text(stmt, col);
	if unlikely(!text)
		goto err_nomem;
	textlen = sqlite3_column_bytes(stmt, col);
	result = (DREF DeeStringObject *)DeeString_NewUtf8((char const *)text, (size_t)textlen,
	                                          STRING_ERROR_FIGNORE);
	return result;
err_nomem:
	if (Dee_CollectMemory(1))
		goto again;
/*err:*/
	return NULL;
}


/* Return column "col" as a deemon DeeBytesObject object */
INTERN WUNUSED DREF DeeBytesObject *DCALL
dee_sqlite3_column_bytes(sqlite3_stmt *stmt, int col) {
	DREF DeeBytesObject *result;
	void const *bytes;
	int bloblen;
again:
	bytes   = sqlite3_column_blob(stmt, col);
	bloblen = sqlite3_column_bytes(stmt, col);
	if unlikely(!bytes && bloblen)
		goto err_nomem;
	result = (DREF DeeBytesObject *)DeeBytes_NewBufferData(bytes, (size_t)bloblen);
	return result;
err_nomem:
	if (Dee_CollectMemory(1))
		goto again;
/*err:*/
	return NULL;
}

DECL_END

#endif /* !GUARD_DEX_SQLITE3_SQLITE3_DEEMON_H */
