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
#ifndef GUARD_DEX_SQLITE3_QUERY_C
#define GUARD_DEX_SQLITE3_QUERY_C 1
#define CONFIG_BUILDING_LIBSQLITE3
#define DEE_SOURCE

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/system-features.h>
#include <deemon/util/atomic.h>
#include <deemon/util/lock.h>

#include <stdint.h>

/**/
#include "sqlite3-external.h"
#include "libsqlite3.h"

DECL_BEGIN

/************************************************************************/
/************************************************************************/
/*                                                                      */
/* Query Iterator                                                       */
/*                                                                      */
/************************************************************************/
/************************************************************************/

typedef struct {
	OBJECT_HEAD
	DREF Query *qi_query; /* [1..1][const] The query being iterated */
} QueryIterator;

PRIVATE WUNUSED NONNULL((1)) int DCALL
qiter_init(QueryIterator *__restrict self,
           size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("QueryIterator", params: "
	Query *query:?GQuery
", docStringPrefix: "qiter");]]]*/
#define qiter_QueryIterator_params "query:?GQuery"
	struct {
		Query *query;
	} args;
	_DeeArg_Unpack1(err, argc, argv, "QueryIterator", &args.query);
/*[[[end]]]*/
	if (DeeObject_AssertTypeExact(args.query, &Query_Type))
		goto err;
	self->qi_query = args.query;
	Dee_Incref(args.query);
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
qiter_fini(QueryIterator *__restrict self) {
	Dee_Decref(self->qi_query);
}

PRIVATE NONNULL((1, 2)) void DCALL
qiter_visit(QueryIterator *__restrict self, Dee_visit_t proc, void *arg) {
	Dee_Visit(self->qi_query);
}

PRIVATE WUNUSED NONNULL((1)) DREF Row *DCALL
qiter_next(QueryIterator *__restrict self) {
#ifdef QUERY_STEP_DONE_IS_ITER_DONE
	return Query_Step(self->qi_query);
#else /* !QUERY_STEP_DONE_IS_ITER_DONE */
	DREF Row *result = Query_Step(self->qi_query);
	if (result == QUERY_STEP_DONE_IS_ITER_DONE)
		result = (DREF Row *)ITER_DONE;
	return result;
#endif /* QUERY_STEP_DONE_IS_ITER_DONE */
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
qiter_advance(QueryIterator *__restrict self, size_t step) {
	return (size_t)Query_Skip(self->qi_query, (uint64_t)step);
}

PRIVATE struct Dee_type_iterator qiter_iterator = {
	/* .tp_nextpair  = */ NULL,
	/* .tp_nextkey   = */ NULL,
	/* .tp_nextvalue = */ NULL,
	/* .tp_advance   = */ (size_t (DCALL *)(DeeObject *__restrict, size_t))&qiter_advance,
};

PRIVATE struct type_member tpconst qiter_members[] = {
	TYPE_MEMBER_FIELD_DOC("seq", STRUCT_OBJECT, offsetof(QueryIterator, qi_query), "->?GQuery"),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject QueryIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "QueryIterator",
	/* .tp_doc      = */ DOC("(query:?GQuery)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ NULL,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ (Dee_funptr_t)&qiter_init,
				TYPE_FIXED_ALLOCATOR(QueryIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&qiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL,
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&qiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&qiter_next,
	/* .tp_iterator      = */ &qiter_iterator,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ qiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ NULL,
	/* .tp_callable      = */ NULL,
};



/************************************************************************/
/************************************************************************/
/*                                                                      */
/* Query                                                                */
/*                                                                      */
/************************************************************************/
/************************************************************************/

PRIVATE NONNULL((1)) void DCALL
query_destroy(Query *__restrict self) {
	DeeStringObject *sql;
	DB *db = self->q_db;

	/* At this point, our query is no longer in-use, and it is our task to mark it as
	 * unused, as well as potentially truncate some older queries from the unused tailq. */
	ASSERT(!Query_InUse(self));
	/*ASSERT(!Query_IsUnused(self));*/ /* Can't be asserted without lock to query-cache of DB */

	/* Common finalization */
	Query_FiniCommon(self);

	/* First things first: check if we can destroy the SQL source code string.
	 * If we can, then we know that the query was just a one-time-only thing,
	 * and we can let the string finalization hook do all the work. */
	sql = self->q_sql;
	if (atomic_cmpxch(&sql->ob_refcnt, 1, 0)) {
		Dee_Decref(db);
		DeeObject_Destroy((DeeObject *)sql);
		return;
	}

	/* Reset query and bindings */
	(void)sqlite3_reset(self->q_stmt);
	(void)sqlite3_clear_bindings(self->q_stmt);

	/* Mark query as unused */
	DB_QueryCache_LockWrite(db);

	/* Add the query to the unused list of `q_db' */
	ASSERT(!Query_IsUnused(self));
	DB_querycache_unused_insert(db, self);
	ASSERT(Query_IsUnused(self));

	/* If the unused query list has grown too large, remove the oldest query */
	if (db->db_querycache_unused_count > db->db_querycache_unused_limit) {
		db_free_oldest_unused_query_and_unlock(db);
	} else {
		DB_QueryCache_LockEndWrite(db);
	}

	/* Drop references */
	Dee_Decref_unlikely(db);
	Dee_Decref_unlikely(sql);
}


PRIVATE WUNUSED NONNULL((1)) char *DCALL
query_get_expanded_sql(Query *__restrict self) {
	char *result;
again:
	if unlikely(Query_Acquire(self))
		goto err;
	result = sqlite3_expanded_sql(self->q_stmt);
	Query_Release(self);
	if unlikely(!result) {
		if (Dee_CollectMemory(DeeString_SIZE(self->q_sql)))
			goto again;
		goto err;
	}
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
query_print(Query *__restrict self, Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t result;
	char *sql_repr = query_get_expanded_sql(self);
	if unlikely(!sql_repr)
		goto err;
	result = (*printer)(arg, sql_repr, strlen(sql_repr));
	sqlite3_free(sql_repr);
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
query_getsql(Query *__restrict self) {
	DREF DeeObject *result;
	char *sql_repr = query_get_expanded_sql(self);
	if unlikely(!sql_repr)
		goto err;
	result = DeeString_NewUtf8(sql_repr, strlen(sql_repr), STRING_ERROR_FIGNORE);
	sqlite3_free(sql_repr);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
query_printrepr(Query *__restrict self, Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t result;
	char *sql_repr = query_get_expanded_sql(self);
	if unlikely(!sql_repr)
		goto err;
	result = DeeFormat_Printf(printer, arg, "%r.query(%q)", self->q_db, sql_repr);
	sqlite3_free(sql_repr);
	return result;
err:
	return -1;
}

/* Advance the query by 1 step and return the resulting row.
 * Returns "QUERY_STEP_DONE" when there are no more rows. */
INTERN WUNUSED NONNULL((1)) DREF Row *DCALL
Query_Step(Query *__restrict self) {
	int rc;

	/* Allocate the row */
	DREF Row *result = Row_Alloc();
	if unlikely(!result)
		goto err;

	/* Lock query and detach old row */
again_with_row:
	if unlikely(Query_AcquireAndDetachRow(self))
		goto err_r;
	ASSERT(Dee_weakref_getaddr(&self->q_row) == NULL);

	/* Lock DB */
	if unlikely(DB_Lock(self->q_db))
		goto err_r_lock;

	/* Perform the SQL step */
	rc = sqlite3_step(self->q_stmt);
	if unlikely(rc != SQLITE_ROW) {
		Query_Release(self);
		if (rc == SQLITE_DONE) {
			Row_Free(result);
			DB_Unlock(self->q_db);
			return QUERY_STEP_DONE;
		}
		rc = err_sql_throwerror_and_maybe_unlock(self->q_db, NULL, rc, NULL,
		                                         ERR_SQL_THROWERROR_F_ALLOW_RESTART |
		                                         ERR_SQL_THROWERROR_F_UNLOCK_DB);
		if (rc)
			goto err_r;
		goto again_with_row;
	}

	/* Unlock DB */
	DB_Unlock(self->q_db);

	/* Initialize the new row and remember it */
	DeeObject_Init(result, &Row_Type);
	weakref_support_init(result);
	Dee_atomic_rwlock_init(&result->r_lock);
	result->r_query = self;
	Dee_Incref(self);
	result->r_rowfmt = NULL; /* Lazily allocated */
	result->r_cells  = NULL; /* Lazily allocated */
	Dee_weakref_set(&self->q_row, (DeeObject *)result);

	Query_Release(self);
	return result;
err_r_lock:
	Query_Release(self);
err_r:
	Row_Free(result);
err:
	return NULL;
}




/* Execute `self' until there is no more data present.
 * @return: (uint64_t)-1: Error
 * @return: * : The # of affected rows */
INTERN WUNUSED NONNULL((1)) uint64_t DCALL
Query_Exec(Query *__restrict self) {
	uint64_t result;
	if unlikely(Query_AcquireAndDetachRow(self))
		goto err;
	result = db_exec_stmt(self->q_db, self->q_stmt);
	Query_Release(self);
	return result;
err:
	return (uint64_t)-1;
}


/* Skip at most `count' rows, returning the actual # of skipped rows.
 * @return: (uint64_t)-1: Error
 * @return: * : The # of skipped rows */
INTERN WUNUSED NONNULL((1)) uint64_t DCALL
Query_Skip(Query *__restrict self, uint64_t count) {
	uint64_t result;
	if unlikely(Query_AcquireAndDetachRow(self))
		goto err;
	result = db_skip_stmt(self->q_db, self->q_stmt, count);
	Query_Release(self);
	return result;
err:
	return (uint64_t)-1;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
query_exec(Query *__restrict self, size_t argc, DeeObject *const *argv) {
	uint64_t rows;
	_DeeArg_Unpack0(err, argc, argv, "exec");
	rows = Query_Exec(self);
	if unlikely(rows == (uint64_t)-1)
		goto err;
	return DeeInt_NewUInt64(rows);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
query_step(Query *__restrict self, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result;
	_DeeArg_Unpack0(err, argc, argv, "step");
	result = (DREF DeeObject *)Query_Step(self);
	if (result == (DREF DeeObject *)QUERY_STEP_DONE)
		result = DeeNone_NewRef();
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
query_skip(Query *__restrict self, size_t argc, DeeObject *const *argv) {
	uint64_t result;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("skip", params: "
	uint64_t count
", docStringPrefix: "query");]]]*/
#define query_skip_params "count:?Dint"
	struct {
		uint64_t count;
	} args;
	if (DeeArg_UnpackStruct(argc, argv, UNPu64 ":skip", &args))
		goto err;
/*[[[end]]]*/
	result = Query_Skip(self, args.count);
	if unlikely(result == (uint64_t)-1)
		goto err;
	return DeeInt_NewUInt64(result);
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF RowFmt *DCALL
query_rowfmt_get(Query *__restrict self) {
	RowFmt *result = Query_GetRowFmt(self);
	Dee_XIncref(result);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF QueryIterator *DCALL
query_iter(Query *__restrict self) {
	DREF QueryIterator *result = DeeObject_MALLOC(QueryIterator);
	if likely(result) {
		DeeObject_Init(result, &QueryIterator_Type);
		result->qi_query = self;
		Dee_Incref(self);
	}
	return result;
}


PRIVATE struct type_seq query_seq = {
	/* .tp_iter = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&query_iter,
};

PRIVATE struct type_method tpconst query_methods[] = {
	TYPE_METHOD("exec", &query_exec, "()->?Dint\nExecute query until there are no more rows, returning the number of affected rows"),
	TYPE_METHOD("step", &query_step, "()->?X2?GRow?N\nReturn the next row that hasn't already been read (returns ?N when there are no more rows)"),
	TYPE_METHOD("skip", &query_skip, "(count:?Dint)->?Dint\nSkip at most @count rows, returning how many rows were actually skipped"),
	/* TODO: fetchone()->?GRecord              (read 1 row, assert that there are no other rows, then return that row as a record)  */
	/* TODO: fetchall()->?S?GRecord            (same as ".frozen.each.asrecord") */
	/* TODO: fetch(limit:?Dint)->?S?GRecord    (same as "[:limit].frozen.each.asrecord") */

	/* TODO: Directly expose sqlite3_stmt_explain() */
	/* TODO: Directly expose sqlite3_stmt_isexplain() */
	/* TODO: Directly expose sqlite3_stmt_readonly() */
	/* TODO: Directly expose sqlite3_stmt_status() */
	/* TODO: Directly expose sqlite3_column_count() */
	/* TODO: Directly expose sqlite3_column_database_name() */
	/* TODO: Directly expose sqlite3_column_origin_name() */
	/* TODO: Directly expose sqlite3_column_table_name() */
	/* TODO: Directly expose sqlite3_column_decltype() */
	/* TODO: Directly expose sqlite3_column_name() */
	/* TODO: Directly expose sqlite3_column_blob()    (get a specific column as Bytes) */
	/* TODO: Directly expose sqlite3_column_double()  (get a specific column as float) */
	/* TODO: Directly expose sqlite3_column_int64()   (get a specific column as int) */
	/* TODO: Directly expose sqlite3_column_text()    (get a specific column as string) */
	/* TODO: Directly expose sqlite3_column_type()    (get type of a specific column) */
	/* TODO: Directly expose sqlite3_data_count() */
	TYPE_METHOD_END
};

PRIVATE struct type_getset tpconst query_getsets[] = {
	TYPE_GETTER_AB("row", &Query_GetRow,
	               "->?GRow\n"
	               "Descriptor for the most-recent row"),
	TYPE_GETTER_AB("_rowfmt", &query_rowfmt_get,
	               "->?G_RowFmt\n"
	               "Descriptor for the format of rows"),
	TYPE_GETTER_AB("sql", &query_getsql,
	               "->?Dstring\n"
	               "The effective SQL being queried, after "
	               /**/ "parameters were inserted (s.a. ?#rawsql)"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst query_members[] = {
	TYPE_MEMBER_FIELD_DOC("db", STRUCT_OBJECT, offsetof(Query, q_db), "->?GDB"),
	TYPE_MEMBER_FIELD_DOC("rawsql", STRUCT_OBJECT, offsetof(Query, q_sql),
	                      "->?Dstring\n"
	                      "The original SQL before parameters were inserted (s.a. ?#sql)"),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject Query_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "Query",
	/* .tp_doc      = */ DOC("TODO"),
	/* NOTE: "TP_FVARIABLE" because of our custom destroy function
	 *       (so must prevent _hostasm from using stack allocation) */
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL | TP_FVARIABLE,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_var = */ {
				/* .tp_ctor        = */ (Dee_funptr_t)NULL,
				/* .tp_copy_ctor   = */ (Dee_funptr_t)NULL,
				/* .tp_deep_ctor   = */ (Dee_funptr_t)NULL,
				/* .tp_any_ctor    = */ (Dee_funptr_t)NULL,
				/* .tp_free        = */ (Dee_funptr_t)NULL,
				/* .tp_pad         = */ { (Dee_funptr_t)NULL },
				/* .tp_any_ctor_kw = */ (Dee_funptr_t)NULL, // TODO: &query_init_kw,
			}
		},
		/* .tp_dtor        = */ NULL, /* Nope: this one has a custom tp_destroy (needed to dead queries can be re-used) */
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ NULL,
		/* .tp_destroy     = */ (void (DCALL *)(DeeObject *__restrict))&query_destroy,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ NULL,
		/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&query_print,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&query_printrepr,
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &query_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ query_methods,
	/* .tp_getsets       = */ query_getsets,
	/* .tp_members       = */ query_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

DECL_END

#endif /* !GUARD_DEX_SQLITE3_QUERY_C */
