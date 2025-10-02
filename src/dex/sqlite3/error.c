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

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/system.h>
#include <deemon/thread.h>
/**/

#include "libsqlite3.h"
#include "sqlite3-external.h"

DECL_BEGIN

#ifndef CONFIG_HAVE_memcasecmp
#define CONFIG_HAVE_memcasecmp
#define memcasecmp dee_memcasecmp
DeeSystem_DEFINE_memcasecmp(dee_memcasecmp)
#endif /* !CONFIG_HAVE_memcasecmp */

#ifndef CONFIG_HAVE_memcasemem
#define CONFIG_HAVE_memcasemem
#undef memcasemem
#define memcasemem dee_memcasemem
DeeSystem_DEFINE_memcasemem(dee_memcasemem)
#endif /* !CONFIG_HAVE_memcasemem */



/************************************************************************/
/* GENERIC DEEMON ERROR THROWING                                        */
/************************************************************************/

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




/************************************************************************/
/* SQL ERROR TYPE                                                       */
/************************************************************************/

PRIVATE WUNUSED NONNULL((1)) int DCALL
sqlerror_init_kw(SQLError *__restrict self, size_t argc,
                 DeeObject *const *argv, DeeObject *kw) {
	/* TODO */
	(void)self;
	(void)argc;
	(void)argv;
	(void)kw;
	return DeeError_NOTIMPLEMENTED();
}

PRIVATE NONNULL((1)) void DCALL
sqlerror_fini(SQLError *__restrict self) {
	Dee_XDecref(self->sqe_sql);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
sqlerror_print(SQLError *__restrict self,
               Dee_formatprinter_t printer, void *arg) {
	if (self->sqe_sql && self->sqe_system.e_message) {
		return DeeFormat_Printf(printer, arg, "%k [error %#x] [at offset %d in %r]",
		                        self->sqe_system.e_message, self->sqe_ecode,
		                        self->sqe_sqloffutf8, self->sqe_sql);
	} else if (self->sqe_sql) {
		return DeeFormat_Printf(printer, arg, "error %#x [at offset %d in %r]",
		                        self->sqe_ecode, self->sqe_sqloffutf8, self->sqe_sql);
	} else if (self->sqe_system.e_message) {
		return DeeFormat_Printf(printer, arg, "%k [error %#x]",
		                        self->sqe_system.e_message, self->sqe_ecode);
	} else {
		return DeeFormat_Printf(printer, arg, "error %#x",
		                        self->sqe_ecode);
	}
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
sqlerror_printrepr(SQLError *__restrict self,
                   Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t result, temp;
	result = DeeFormat_Printf(printer, arg, "%r(ecode: %d",
	                          Dee_TYPE(&self->sqe_system), self->sqe_ecode);
	if unlikely(result < 0)
		goto done;
	if (self->sqe_sql) {
		temp = DeeFormat_Printf(printer, arg, ", sql: %r, sqloff: %d",
		                        self->sqe_sql, self->sqe_sqloffutf8);
		if unlikely(temp < 0)
			goto err;
		result += temp;
	}
	if (self->sqe_system.e_message) {
		temp = DeeFormat_Printf(printer, arg, ", message: %r",
		                        self->sqe_system.e_message);
		if unlikely(temp < 0)
			goto err;
		result += temp;
	}
	if (self->sqe_system.se_errno != Dee_SYSTEM_ERROR_UNKNOWN) {
		temp = DeeFormat_Printf(printer, arg, ", errno: %d",
		                        self->sqe_system.se_errno);
		if unlikely(temp < 0)
			goto err;
		result += temp;
	}
	if (self->sqe_system.e_inner) {
		temp = DeeFormat_Printf(printer, arg, ", inner: %r",
		                        self->sqe_system.e_inner);
		if unlikely(temp < 0)
			goto err;
		result += temp;
	}

	temp = DeeFormat_PRINT(printer, arg, ")");
	if unlikely (temp < 0)
		goto err;
	result += temp;
done:
	return result;
err:
	return temp;
}

PRIVATE struct type_member tpconst sqlerror_members[] = {
	TYPE_MEMBER_FIELD_DOC("sql", STRUCT_OBJECT, offsetof(SQLError, sqe_sql), "->?Dstring"),
	TYPE_MEMBER_FIELD("sqloff", STRUCT_INT | STRUCT_CONST, offsetof(SQLError, sqe_sqloffutf8)),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject SQLError_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "SQLError",
	/* .tp_doc      = */ DOC("(ecode:?Dint,sql?:?Dstring,sqloff?:?Dint,message?:?Dstring,errno?:?Dint,inner?:?DError)\n"
	                         "#pecode{SQLite error code}"
	                         "#psql{SQL code causing the exception (if there is any)}"
	                         "#psqloff{Offset into @sql where error happened}"
	                         "#pmessage{SQL error message}"
	                         "#perrno{System error code (if relevant)}"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeError_SystemError,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor        = */ (Dee_funptr_t)NULL,
				/* .tp_copy_ctor   = */ (Dee_funptr_t)NULL,
				/* .tp_deep_ctor   = */ (Dee_funptr_t)NULL,
				/* .tp_any_ctor    = */ (Dee_funptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(SQLError),
				/* .tp_any_ctor_kw = */ (Dee_funptr_t)&sqlerror_init_kw
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&sqlerror_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ NULL,
		/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&sqlerror_print,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&sqlerror_printrepr,
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ sqlerror_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ NULL,
	/* .tp_callable      = */ NULL,
};

#define INIT_SQLERROR_SUBCLASS(name, base)                      \
	{                                                           \
		OBJECT_HEAD_INIT(&DeeType_Type),                        \
		/* .tp_name     = */ name,                              \
		/* .tp_doc      = */ NULL,                              \
		/* .tp_flags    = */ TP_FINHERITCTOR,                   \
		/* .tp_weakrefs = */ 0,                                 \
		/* .tp_features = */ TF_NONE,                           \
		/* .tp_base     = */ base,                              \
		/* .tp_init = */ {                                      \
			{                                                   \
				/* .tp_alloc = */ {                             \
					/* .tp_ctor        = */ (Dee_funptr_t)NULL, \
					/* .tp_copy_ctor   = */ (Dee_funptr_t)NULL, \
					/* .tp_deep_ctor   = */ (Dee_funptr_t)NULL, \
					/* .tp_any_ctor    = */ (Dee_funptr_t)NULL, \
					TYPE_FIXED_ALLOCATOR(SQLError),             \
				}                                               \
			},                                                  \
			/* .tp_dtor        = */ NULL,                       \
			/* .tp_assign      = */ NULL,                       \
			/* .tp_move_assign = */ NULL,                       \
		},                                                      \
		/* .tp_cast = */ {                                      \
			/* .tp_str       = */ NULL,                         \
			/* .tp_repr      = */ NULL,                         \
			/* .tp_bool      = */ NULL,                         \
			/* .tp_print     = */ NULL,                         \
			/* .tp_printrepr = */ NULL,                         \
		},                                                      \
		/* .tp_visit         = */ NULL,                         \
		/* .tp_gc            = */ NULL,                         \
		/* .tp_math          = */ NULL,                         \
		/* .tp_cmp           = */ NULL,                         \
		/* .tp_seq           = */ NULL,                         \
		/* .tp_iter_next     = */ NULL,                         \
		/* .tp_iterator      = */ NULL,                         \
		/* .tp_attr          = */ NULL,                         \
		/* .tp_with          = */ NULL,                         \
		/* .tp_buffer        = */ NULL,                         \
		/* .tp_methods       = */ NULL,                         \
		/* .tp_getsets       = */ NULL,                         \
		/* .tp_members       = */ NULL,                         \
		/* .tp_class_methods = */ NULL,                         \
		/* .tp_class_getsets = */ NULL,                         \
		/* .tp_class_members = */ NULL,                         \
		/* .tp_method_hints  = */ NULL,                         \
		/* .tp_call          = */ NULL,                         \
		/* .tp_callable      = */ NULL,                         \
	}
INTERN DeeTypeObject SQLSyntaxError_Type =
INIT_SQLERROR_SUBCLASS("SQLSyntaxError", &SQLError_Type);
INTERN DeeTypeObject SQLConstraintError_Type =
INIT_SQLERROR_SUBCLASS("SQLConstraintError", &SQLError_Type);










/************************************************************************/
/* SQL ERROR TRANSLATION                                                */
/************************************************************************/

PRIVATE bool DCALL
is_syntax_errmsg(char const *msg, size_t len) {
#define CONTAINS(text)   (len >= COMPILER_STRLEN(text) && memcasemem(msg, len, text, COMPILER_STRLEN(text) * sizeof(char)) == 0)
#define ENDSWITH(text)   (len >= COMPILER_STRLEN(text) && memcasecmp(msg + len - COMPILER_STRLEN(text), text, COMPILER_STRLEN(text) * sizeof(char)) == 0)
#define STARTSWITH(text) (len >= COMPILER_STRLEN(text) && memcasecmp(msg, text, COMPILER_STRLEN(text) * sizeof(char)) == 0)
#define EQUALS(text)     (len == COMPILER_STRLEN(text) && memcasecmp(msg, text, COMPILER_STRLEN(text) * sizeof(char)) == 0)
	if (EQUALS("syntax error"))
		return true;
	if (STARTSWITH("syntax error "))
		return true;
	if (ENDSWITH(": syntax error"))
		return true;
	if (CONTAINS(": syntax error "))
		return true;
	return false;
#undef ENDSWITH
#undef STARTSWITH
#undef EQUALS
}

/* Throw an error and possibly unlock the DB
 * @return: 0 : try again
 * @return: -1: Error was thrown */
INTERN ATTR_COLD WUNUSED int DCALL
err_sql_throwerror(int errcode, unsigned int flags,
                   DB *db, DeeStringObject *sql) {
#define LOCAL_UNLOCK()                              \
	do {                                            \
		if (flags & ERR_SQL_THROWERROR_F_UNLOCK_DB) \
			DB_Unlock(db);                          \
	}	__WHILE0
	int result;
	DeeTypeObject *type;
	int erroffs = -1;
	char const *errmsg = NULL;
	DREF DeeStringObject *errmsg_ob;
	int system_errno = sqlite3_system_errno(db->db_db);
	if (db != NULL) {
		int db_errcode = sqlite3_errcode(db->db_db);
		if (db_errcode != 0) {
			errcode = db_errcode;
			errmsg  = sqlite3_errmsg(db->db_db);
		}
		if (sql != NULL) {
			erroffs = sqlite3_error_offset(db->db_db);
			if (erroffs == -1)
				sql = NULL;
		}
	}
	if (errmsg == NULL)
		errmsg = sqlite3_errstr(errcode);
	if (errmsg == NULL)
		errmsg = "Unknown error";

	/* Use different errors for specific codes */
	switch (errcode & 0xff) {
	case SQLITE_ERROR:
		/* Check (based on the message) if this may be is a syntax error */
		type = &SQLError_Type;
		if (errmsg && is_syntax_errmsg(errmsg, strlen(errmsg)))
			type = &SQLSyntaxError_Type;
		break;
	case SQLITE_PERM:
handle_eacces:
		type = &DeeError_FileAccessError;
		break;
	case SQLITE_NOMEM:
handle_nomem:
		LOCAL_UNLOCK();
		return Dee_CollectMemory(4096) ? 0 : -1;
		break;
	case SQLITE_READONLY:
		type = &SQLError_Type;
		if (errcode == SQLITE_READONLY_DIRECTORY)
			type = &DeeError_ReadOnlyFile;
		break;
	case SQLITE_INTERRUPT:
		LOCAL_UNLOCK();
		return DeeThread_CheckInterrupt();
	case SQLITE_IOERR: /* Some kind of disk I/O error occurred */
		type = &DeeError_FSError;
		if (errcode == SQLITE_IOERR_NOMEM)
			goto handle_nomem;
		if (errcode == SQLITE_IOERR_ACCESS)
			goto handle_eacces;
		break;
	case SQLITE_CANTOPEN: /* Unable to open the database file */
		type = &DeeError_FileNotFound;
		if (errcode == SQLITE_CANTOPEN_ISDIR)
			type = &DeeError_IsDirectory;
		break;
	case SQLITE_INTERNAL: /* Internal logic error in SQLite */
	case SQLITE_MISUSE:   /* Library used incorrectly */
	case SQLITE_NOLFS:    /* Uses OS features not supported on host */
		type = &DeeError_UnsupportedAPI;
		break;
	case SQLITE_RANGE: /* 2nd parameter to sqlite3_bind out of range */
		type = &DeeError_IndexError;
		break;
	case SQLITE_NOTFOUND: /* Unknown opcode in sqlite3_file_control() */
		type = &DeeError_ValueError;
		break;
	case SQLITE_TOOBIG:     /* String or BLOB exceeds size limit (consider the size a constraint, so use "SQLConstraintError_Type") */
	case SQLITE_CONSTRAINT: /* Abort due to constraint violation */
		type = &SQLConstraintError_Type;
		if (errcode == SQLITE_CONSTRAINT_DATATYPE) {
	case SQLITE_MISMATCH:
			/* Wrong argument type used in API/constraint -> throw native TypeError */
			type = &DeeError_TypeError;
		}
		break;
	default:
		type = &SQLError_Type;
		break;
	}
	errmsg_ob = NULL;
	if (errmsg) {
		size_t msglen = strlen(errmsg);
		errmsg_ob = (DREF DeeStringObject *)DeeString_TryNewUtf8(errmsg, msglen,
		                                                         STRING_ERROR_FIGNORE);
		if unlikely(!errmsg_ob) {
			LOCAL_UNLOCK();
			return Dee_CollectMemory(msglen) ? 0 : -1;
		}
	}
	LOCAL_UNLOCK();

	/* Special case for SQL-error-derived types */
	if (DeeType_Extends(type, &SQLError_Type)) {
		DREF SQLError *error = DeeObject_MALLOC(SQLError);
		if unlikely(!error)
			goto err;
		DeeObject_Init(&error->sqe_system, type);
		error->sqe_system.e_inner   = NULL;
		error->sqe_system.e_message = errmsg_ob; /* Inherit reference */
#ifdef CONFIG_HOST_WINDOWS
		error->sqe_system.se_lasterror = system_errno;
		error->sqe_system.se_errno = DeeNTSystem_TranslateErrno(system_errno);
#else /* CONFIG_HOST_WINDOWS */
		error->sqe_system.se_errno = system_errno;
#endif /* !CONFIG_HOST_WINDOWS */
		error->sqe_sql = sql;
		Dee_XIncref(sql);
		error->sqe_sqloffutf8 = erroffs;
		error->sqe_ecode      = errcode;
		/* Throw SQL error */
		result = DeeError_ThrowInherited((DREF DeeObject *)error);
	} else if (erroffs >= 0) {
		result = DeeError_Throwf(type, "SQL error %d: %K [at offset %d in %r]",
		                         errcode, errmsg_ob, erroffs, sql);
	} else {
		result = DeeError_Throwf(type, "SQL error %d: %K",
		                         errcode, errmsg_ob);
	}
	return result;
err:
	return -1;
}

DECL_END

#endif /* !GUARD_DEX_SQLITE3_ERROR_C */
