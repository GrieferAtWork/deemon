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
#ifndef GUARD_DEX_SQLITE3_ERROR_C
#define GUARD_DEX_SQLITE3_ERROR_C 1
#define CONFIG_BUILDING_LIBSQLITE3
#define DEE_SOURCE

#include <deemon/api.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/object.h>
#include <deemon/string.h>
/**/

#include "libsqlite3.h"
#include "sqlite3-external.h"

DECL_BEGIN

INTERN ATTR_COLD NONNULL((1)) int
(DCALL err_index_out_of_bounds)(DeeObject *__restrict self,
                                size_t index, size_t size) {
	ASSERT_OBJECT(self);
	return DeeError_Throwf(&DeeError_IndexError,
	                       "Index `%" PRFuSIZ "' lies outside the valid bounds "
	                       "`0...%" PRFuSIZ "' of sequence of type `%k'",
	                       index, size, Dee_TYPE(self));
}

INTERN ATTR_COLD NONNULL((1, 2)) int
(DCALL err_unknown_key_str)(DeeObject *__restrict map, char const *__restrict key) {
	ASSERT_OBJECT(map);
	return DeeError_Throwf(&DeeError_KeyError,
	                       "Could not find key `%s' in %k `%k'",
	                       key, Dee_TYPE(map), map);
}

INTERN ATTR_COLD NONNULL((1, 2)) int
(DCALL err_unknown_key_str_len)(DeeObject *__restrict map, char const *__restrict key, size_t keylen) {
	ASSERT_OBJECT(map);
	return DeeError_Throwf(&DeeError_KeyError,
	                       "Could not find key `%$s' in %k `%k'",
	                       keylen, key, Dee_TYPE(map), map);
}

INTERN ATTR_COLD NONNULL((1, 2)) int
(DCALL err_unbound_attribute_string)(DeeTypeObject *__restrict tp,
                                     char const *__restrict name) {
	ASSERT_OBJECT(tp);
	return DeeError_Throwf(&DeeError_UnboundAttribute,
	                       "Unbound attribute `%r.%s'",
	                       tp, name);
}

INTERN ATTR_COLD NONNULL((1)) int DCALL
err_multiple_statements(DeeStringObject *__restrict sql) {
	return DeeError_Throwf(&DeeError_SyntaxError,
	                       "Given query contains multiple SQL statements: %r",
	                       sql);
}


/* Throw an error and possibly unlock the DB (if it was given as non-NULL)
 * @return: 0 : try again (only when `ERR_SQL_THROWERROR_F_ALLOW_RESTART' was set)
 * @return: -1: Error was thrown */
INTERN ATTR_COLD int DCALL
err_sql_throwerror_and_maybe_unlock(DB *db, char const *sql, int errcode,
                                    char const *errmsg, unsigned int flags) {
	int result;
	DeeTypeObject *type;
	int erroffs = -1;
	if (db != NULL) {
		int db_errcode = sqlite3_errcode(db->db_db);
		if (db_errcode != 0) {
			char const *db_errmsg;
			errcode   = db_errcode;
			db_errmsg = sqlite3_errmsg(db->db_db);
			if (db_errmsg != NULL)
				errmsg = db_errmsg;
		}
		if (sql != NULL)
			erroffs = sqlite3_error_offset(db->db_db);
	}
	if (errmsg == NULL) {
		if (errmsg == NULL)
			errmsg = sqlite3_errstr(errcode);
		if (errmsg == NULL)
			errmsg = "Unknown error";
	}

	/* Use different errors for specific codes */
	type = &DeeError_Error; /* TODO: SQLError (extends DeeError_Error) */
	switch (errcode & 0xff) {
	case SQLITE_PERM:
handle_eacces:
		type = &DeeError_FileAccessError;
		break;
	case SQLITE_NOMEM:
handle_nomem:
		if (flags & ERR_SQL_THROWERROR_F_ALLOW_RESTART) {
			if (flags & ERR_SQL_THROWERROR_F_UNLOCK_DB)
				DB_Unlock(db);
			return Dee_CollectMemory(4096) ? 0 : -1;
		}
		type = &DeeError_NoMemory;
		break;
	case SQLITE_READONLY:
		if (errcode == SQLITE_READONLY_DIRECTORY)
			type = &DeeError_ReadOnlyFile;
		break;
	case SQLITE_INTERRUPT:
		if (flags & ERR_SQL_THROWERROR_F_ALLOW_RESTART) {
			if (flags & ERR_SQL_THROWERROR_F_UNLOCK_DB)
				DB_Unlock(db);
			return DeeThread_CheckInterrupt();
		}
		type = &DeeError_Interrupt;
		break;
	case SQLITE_IOERR:
		type = &DeeError_FSError;
		if (errcode == SQLITE_IOERR_NOMEM)
			goto handle_nomem;
		if (errcode == SQLITE_IOERR_ACCESS)
			goto handle_eacces;
		break;
	case SQLITE_CANTOPEN:
		type = &DeeError_FileNotFound;
		if (errcode == SQLITE_CANTOPEN_ISDIR)
			type = &DeeError_IsDirectory;
		break;
	case SQLITE_MISUSE:
		type = &DeeError_UnsupportedAPI;
		break;
	case SQLITE_RANGE:
		type = &DeeError_IndexError;
		break;
	default: break;
	}
	if (erroffs >= 0) {
		result = DeeError_Throwf(type, "SQL error %d: %s [at offset %d in %q]",
		                         errcode, errmsg, erroffs, sql);
	} else {
		result = DeeError_Throwf(type, "SQL error %d: %s",
		                         errcode, errmsg);
	}
	if (flags & ERR_SQL_THROWERROR_F_UNLOCK_DB)
		DB_Unlock(db);
	return result;
}

INTERN ATTR_COLD int DCALL
err_sql_throwerror(int errcode, unsigned int flags) {
	return err_sql_throwerror_and_maybe_unlock(NULL, NULL, errcode, NULL,
	                                           flags | ERR_SQL_THROWERROR_F_NORMAL);
}




/* These functions are called when a "DB_Type" object is created/destroyed.
 * Internally, these keep a running counter such that:
 * - The first call does `sqlite3_initialize()' and throws an error if something went wrong
 * - The last call does `sqlite3_shutdown()'
 * @return: 0 : Success
 * @return: -1: An error was thrown */
PRIVATE Dee_refcnt_t libsqlite3_initialized = 0;

PRIVATE ATTR_NOINLINE WUNUSED int DCALL
libsqlite3_init_impl(void) {
	int rc;
again:
	rc = sqlite3_initialize();
	if likely(rc == SQLITE_OK)
		return 0;
	rc = err_sql_throwerror(rc, ERR_SQL_THROWERROR_F_ALLOW_RESTART);
	if (rc == 0)
		goto again;
	return -1;
}

INTERN WUNUSED int DCALL libsqlite3_init(void) {
	if (_DeeRefcnt_FetchInc(&libsqlite3_initialized) == 0)
		return libsqlite3_init_impl();
	return 0;
}

INTERN void DCALL libsqlite3_fini(void) {
	if (_DeeRefcnt_DecFetch(&libsqlite3_initialized) == 0)
		(void)sqlite3_shutdown();
}


DECL_END

#endif /* !GUARD_DEX_SQLITE3_ERROR_C */
