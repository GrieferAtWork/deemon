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
#ifndef GUARD_DEX_SQLITE3_SQLITE3_DEEMON_H
#define GUARD_DEX_SQLITE3_SQLITE3_DEEMON_H 1
#define CONFIG_BUILDING_LIBSQLITE3
#define DEE_SOURCE

#include "libsqlite3.h"
/**/

#include <deemon/api.h>

#include <deemon/bool.h>            /* DeeBool_IsTrue, DeeBool_Type */
#include <deemon/bytes.h>           /* DeeBytes* */
#include <deemon/error.h>           /* DeeError_Throwf, DeeError_TypeError */
#include <deemon/float.h>           /* CONFIG_HAVE_FPU, DeeFloat_Type, DeeFloat_VALUE */
#include <deemon/int.h>             /* DeeInt_Type */
#include <deemon/method-hints.h>    /* DeeObject_InvokeMethodHint */
#include <deemon/none.h>            /* DeeNone_Type */
#include <deemon/object.h>
#include <deemon/string.h>          /* DeeString*, WSTR_LENGTH */
#include <deemon/super.h>           /* DeeSuper* */
#include <deemon/system-features.h> /* CONFIG_HAVE_FPU */

#include <hybrid/typecore.h> /* __BYTE_TYPE__, __SIZEOF_SIZE_T__ */

#include "sqlite3-external.h"

#include <stddef.h> /* NULL, ptrdiff_t, size_t */
#include <stdint.h> /* int64_t */

#undef byte_t
#define byte_t __BYTE_TYPE__

DECL_BEGIN

#if __SIZEOF_SIZE_T__ > 4
#define sqlite3_bind_textSIZ(stmt, i, text, length, dtor) sqlite3_bind_text64(stmt, i, text, (sqlite3_uint64)(length), dtor, SQLITE_UTF8)
#define sqlite3_bind_blobSIZ(stmt, i, blob, length, dtor) sqlite3_bind_blob64(stmt, i, blob, (sqlite3_uint64)(length), dtor)
#else /* __SIZEOF_SIZE_T__ > 4 */
#define sqlite3_bind_textSIZ(stmt, i, text, length, dtor) sqlite3_bind_text(stmt, i, text, (int)(unsigned int)(length), dtor)
#define sqlite3_bind_blobSIZ(stmt, i, blob, length, dtor) sqlite3_bind_blob(stmt, i, blob, (int)(unsigned int)(length), dtor)
#endif /* __SIZEOF_SIZE_T__ <= 4 */


PRIVATE void _dee_sqlite3_bind_string_decref(void *str) {
	DREF DeeStringObject *string;
	string = COMPILER_CONTAINER_OF((char *)str, DeeStringObject, s_str);
	Dee_Decref(string);
}

INTERN WUNUSED NONNULL((1, 4)) int DCALL
dee_sqlite3_bind_string(DB *__restrict db, sqlite3_stmt *stmt,
                        int index, DeeStringObject *self) {
	int rc;
again:
	if (DeeString_STR_ISUTF8(self)) {
		/* Simple case: can directly pass `DeeString_STR()' */
		size_t length = DeeString_SIZE(self);
		if unlikely(DB_Lock(db))
			goto err;
		Dee_Incref(self); /* Inherited by `sqlite3_bind_textSIZ()' */
		rc = sqlite3_bind_textSIZ(stmt, index, DeeString_STR(self), length,
		                          &_dee_sqlite3_bind_string_decref);
	} else {
		size_t length;
		char const *utf8 = DeeString_AsUtf8(Dee_AsObject(self));
		if unlikely(!utf8)
			goto err;
		length = WSTR_LENGTH(utf8);
		if unlikely(DB_Lock(db))
			goto err;
		rc = sqlite3_bind_textSIZ(stmt, index, utf8, length, SQLITE_TRANSIENT);
	}
	if unlikely(rc != SQLITE_OK)
		goto err_rc;
	DB_Unlock(db);
	return 0;
err_rc:
	rc = err_sql_throwerror(rc, ERR_SQL_THROWERROR_F_UNLOCK_DB, db, NULL);
	if (rc == 0)
		goto again;
	return rc;
err:
	return -1;
}

PRIVATE void _dee_sqlite3_bind_bytes_decref(void *blob) {
	DREF DeeBytesObject *bytes;
	bytes = COMPILER_CONTAINER_OF((byte_t *)blob, DeeBytesObject, b_buffer);
	Dee_Decref(bytes);
}

INTERN WUNUSED NONNULL((1, 4)) int DCALL
dee_sqlite3_bind_bytes(DB *__restrict db, sqlite3_stmt *stmt,
                       int index, DeeBytesObject *self) {
	int rc;
	byte_t *data;
	size_t size;
again:
	if unlikely(DB_Lock(db))
		goto err;
	data = DeeBytes_DATA(self);
	size = DeeBytes_SIZE(self);
	if (data == self->b_buffer) {
		/* Simple case: can directly pass `data' */
		Dee_Incref(self);
		rc = sqlite3_bind_blobSIZ(stmt, index, data, size,
		                          &_dee_sqlite3_bind_bytes_decref);
	} else {
		rc = sqlite3_bind_blobSIZ(stmt, index, data, size, SQLITE_TRANSIENT);
	}
	if unlikely(rc != SQLITE_OK)
		goto err_rc;
	DB_Unlock(db);
	return 0;
err_rc:
	rc = err_sql_throwerror(rc, ERR_SQL_THROWERROR_F_UNLOCK_DB, db, NULL);
	if (rc == 0)
		goto again;
	return rc;
err:
	return -1;
}


/* Bind "ob" to query parameter "index" of "stmt"
 * @return: 0 : Success
 * @return: -1: An error was thrown */
INTERN WUNUSED NONNULL((1, 4)) int DCALL
dee_sqlite3_bind_object(DB *__restrict db, sqlite3_stmt *stmt, int index, DeeObject *self) {
	int rc;
	DeeTypeObject *type = Dee_TYPE(self);
	if (type == &DeeSuper_Type) {
		type = DeeSuper_TYPE(self);
		self = DeeSuper_SELF(self);
	}
again:
	if (type == &DeeString_Type) {
		return dee_sqlite3_bind_string(db, stmt, index, (DeeStringObject *)self);
	} else if (type == &DeeInt_Type) {
		int64_t value;
		if unlikely(DeeObject_AsInt64(self, &value))
			goto err;
		if unlikely(DB_Lock(db))
			goto err;
		rc = sqlite3_bind_int64(stmt, index, (sqlite3_int64)value);
	} else if (type == &DeeBool_Type) {
		if unlikely(DB_Lock(db))
			goto err;
		rc = sqlite3_bind_int(stmt, index, DeeBool_IsTrue(self) ? 1 : 0);
	} else if (type == &DeeNone_Type) {
		if unlikely(DB_Lock(db))
			goto err;
		rc = sqlite3_bind_null(stmt, index);
#ifdef CONFIG_HAVE_FPU
	} else if (type == &DeeFloat_Type) {
		if unlikely(DB_Lock(db))
			goto err;
		rc = sqlite3_bind_double(stmt, index, DeeFloat_VALUE(self));
#endif /* CONFIG_HAVE_FPU */
	} else if (type == &DeeBytes_Type) {
		return dee_sqlite3_bind_bytes(db, stmt, index, (DeeBytesObject *)self);
	} else {
		/* TODO: Support for generic numeric types:
		 * >> if (self is Numeric) {
		 * >>     if (Numeric.isfloat(self)) {
		 * >>         return BIND((float)self);
		 * >>     } else {
		 * >>         return BIND((int)self);
		 * >>     }
		 * >> } */
		char const *param_name;

		/* HINT: Looking at the impl, `sqlite3_bind_parameter_name()' actually
		 *       doesn't need to lock the database, so we can call it would the
		 *       need of locking the DB, and then duplicating the string! */
		param_name = sqlite3_bind_parameter_name(stmt, index);
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
	DB_Unlock(db);
	return 0;
err_rc:
	rc = err_sql_throwerror(rc, ERR_SQL_THROWERROR_F_UNLOCK_DB, db, NULL);
	if (rc == 0)
		goto again;
	return rc;
err:
	return -1;
}

struct dee_sqlite3_bind_params_indexed_data {
	DB           *sbpid_db;   /* Linked DB */
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
		result = dee_sqlite3_bind_object(data->sbpid_db, data->sbpid_stmt,
		                                 (int)(index + data->sbpid_off), value);
		if likely(result == 0)
			result = 1;
		return result;
	}
	return 0;
}


/* Custom function: like `sqlite3_bind_parameter_index', but "zName"
 * doesn't have to include the leading `:', `$' or `@'. */
SQLITE_API int sqlite3_bind_parameter_index__without_prefix(sqlite3_stmt*, const char *zName);

struct dee_sqlite3_bind_params_named_data {
	DB           *sbpnd_db;   /* Linked DB */
	sqlite3_stmt *sbpnd_stmt; /* Statement to populate */
};

PRIVATE WUNUSED NONNULL((2, 3)) Dee_ssize_t DCALL
dee_sqlite3_bind_params_named(void *arg, DeeObject *key, DeeObject *value) {
	struct dee_sqlite3_bind_params_named_data *data;
	char const *utf8_name;
	int param_index;
	if (DeeObject_AssertTypeExact(key, &DeeString_Type))
		goto err;
	utf8_name = DeeString_AsUtf8(key);
	if unlikely(!utf8_name)
		goto err;
	data = (struct dee_sqlite3_bind_params_named_data *)arg;
	/* HINT: Looking at the impl, `sqlite3_bind_parameter_index()' actually
	 *       doesn't need to lock the database, so we can call it would the
	 *       need of locking the DB, and then duplicating the string! */
	param_index = sqlite3_bind_parameter_index__without_prefix(data->sbpnd_stmt, utf8_name);
	if unlikely(param_index == 0)
		return 0; /* Ignore parameter (may be used by a subsequent query) */
	return dee_sqlite3_bind_object(data->sbpnd_db, data->sbpnd_stmt,
	                               param_index, value);
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
INTERN WUNUSED NONNULL((1, 3)) Dee_ssize_t DCALL
dee_sqlite3_bind_params(DB *__restrict db, sqlite3_stmt *stmt,
                        DeeObject *params, size_t unnamed_start) {
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
		data.sbpid_db   = db;
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
		struct dee_sqlite3_bind_params_named_data data;
		data.sbpnd_db   = db;
		data.sbpnd_stmt = stmt;
		fe_status = DeeObject_InvokeMethodHint(map_operator_foreach_pair, params,
		                                       &dee_sqlite3_bind_params_named, &data);
	}
	/* NOTE: We don't really assert that all query params
	 *       are bound, since there's no easy way to do that. */
	return fe_status;
}

DECL_END

#endif /* !GUARD_DEX_SQLITE3_SQLITE3_DEEMON_H */
