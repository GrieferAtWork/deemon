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
#ifndef GUARD_DEX_SQLITE3_QUERY_C
#define GUARD_DEX_SQLITE3_QUERY_C 1
#define CONFIG_BUILDING_LIBSQLITE3
#define DEE_SOURCE

#include "libsqlite3.h"
/**/

#include <deemon/api.h>

#include <deemon/alloc.h>           /* DeeObject_MALLOC, Dee_CollectMemory, Dee_CollectMemoryc, Dee_Free, Dee_TYPE_CONSTRUCTOR_INIT_FIXED, Dee_TryMallocc */
#include <deemon/arg.h>             /* DeeArg_Unpack*, UNPu64 */
#include <deemon/bytes.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/system-features.h>
#include <deemon/util/atomic.h>     /* atomic_cmpxch, atomic_read */
#include <deemon/util/lock.h>       /* Dee_atomic_rwlock_init */

#include "sqlite3-external.h"

#include <stdbool.h> /* bool */
#include <stddef.h>  /* NULL, offsetof, size_t */
#include <stdint.h>  /* uint64_t */

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
	DeeArg_Unpack1(err, argc, argv, "QueryIterator", &args.query);
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
	return Query_Step(self->qi_query);
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
	/* .tp_doc      = */ DOC("(query:?GQuery)\n"
	                         "\n"
	                         "next->?GRow"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ QueryIterator,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ NULL,
			/* tp_deep_ctor:   */ NULL,
			/* tp_any_ctor:    */ &qiter_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL /* Can't be serialized because queries can't be serialized */
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&qiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL,
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&qiter_visit,
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

/* Returns ITER_DONE if lock was released and you should try again */
PRIVATE WUNUSED NONNULL((1)) DREF RowFmt *DCALL
_Query_TryNewRowFmtOrUnlock(Query *__restrict self) {
	DREF RowFmt *result;
	unsigned int i, ncol;
	size_t badalloc_memsize;
	ncol = (unsigned int)sqlite3_column_count(self->q_stmt);
	result = RowFmt_TryAlloc(ncol);
	if unlikely(!result) {
		Query_UnlockDB(self);
		if (Dee_CollectMemory(RowFmt_Sizeof(ncol)))
			return (DREF RowFmt *)ITER_DONE;
		return NULL;
	}
	DeeObject_Init(result, &RowFmt_Type);
	result->rf_ncol = ncol;
	for (i = 0; i < ncol; ++i) {
		DREF DeeStringObject *nameob, *decltypeob;
		struct cellfmt *fmt = &result->rf_cols[i];
		char const *name, *decltype_;

		/* Allocate name */
		name = sqlite3_column_name(self->q_stmt, (int)i);
		/* >> If sqlite3_malloc() fails [...] then a NULL pointer is returned */
		if unlikely(!name) {
			bool collect_ok;
			badalloc_memsize = 32; /* Guess required size */
			nameob = NULL;
err_nomem__unlock_r_i_nameopt:
			Query_UnlockDB(self);
			collect_ok = Dee_CollectMemory(badalloc_memsize);
			while (i--) {
				fmt = &result->rf_cols[i];
				cellfmt_fini(fmt);
			}
			RowFmt_Free(result);
			Dee_XDecref_unlikely(nameob);
			return collect_ok ? (DREF RowFmt *)ITER_DONE : NULL;
		}
		badalloc_memsize = strlen(name);
		nameob = (DREF DeeStringObject *)DeeString_TryNewUtf8(name, badalloc_memsize,
		                                                      STRING_ERROR_FIGNORE);
		if unlikely(!nameob)
			goto err_nomem__unlock_r_i_nameopt;
		fmt->cfmt_name = nameob; /* Inherit reference */

		/* Allocate decltype */
		decltype_ = sqlite3_column_decltype(self->q_stmt, (int)i);
		/* >> If the Nth column of the result set is an expression or subquery, then a NULL pointer is returned */
		decltypeob = NULL;
		if likely(decltype_) {
			badalloc_memsize = strlen(decltype_);
			decltypeob = (DREF DeeStringObject *)DeeString_TryNewUtf8(decltype_, badalloc_memsize,
			                                                          STRING_ERROR_FIGNORE);
			if unlikely(!decltypeob)
				goto err_nomem__unlock_r_i_nameopt;
		}
		fmt->cfmt_decltype = decltypeob; /* Inherit reference */
	}
	return result;
}

/* Return (and lazily allocate on first use) the RowFmt descriptor of this query. */
INTERN WUNUSED NONNULL((1)) RowFmt *DCALL
Query_GetRowFmt(Query *__restrict self) {
	RowFmt *result = atomic_read(&self->q_rowfmt);
	if likely(result)
		return result;
again:
	if unlikely(Query_LockDB(self))
		goto err;
	result = self->q_rowfmt;
	if unlikely(result) {
		Query_UnlockDB(self);
		return result;
	}
	ASSERT(!self->q_rowfmt);
	result = _Query_TryNewRowFmtOrUnlock(self);
	ASSERT(!self->q_rowfmt);
	if unlikely(!ITER_ISOK(result)) {
		if unlikely(!result)
			goto err;
		goto again;
	}
	ASSERT(!self->q_rowfmt);
	self->q_rowfmt = result; /* Inherit reference */
	Query_UnlockDB(self);
	return result;
err:
	return NULL;
}



/* Ensure that `self->q_row' is either dead or NULL.
 * If it isn't, try to copy row data into "q_row", then clear the weakref. */
#define QUERY_DETACHROWORUNLOCK_OK       0    /* Success, and locks were never lost */
#define QUERY_DETACHROWORUNLOCK_UNLOCKED 1    /* Success, but Query+DB lock was released at one point */
#define QUERY_DETACHROWORUNLOCK_ERR      (-1) /* Error, and Query+DB lock was released */
PRIVATE WUNUSED NONNULL((1)) int DCALL
Query_DetachRowOrUnlock(Query *__restrict self) {
	int result = QUERY_DETACHROWORUNLOCK_OK;
	DREF Row *row;
	RowFmt *rowfmt;
again:
	row = (DREF Row *)Dee_weakref_lock(&self->q_row);
	if (!row) /* Row isn't cached -> nothing to detach! */
		return result;

	/* XXX: Given regular deemon user-code like this:
	 * >> for (local row: db.query("SELECT * FROM foo"))
	 * >>     print repr row;
	 *
	 * Without optimizations, this code will keep the
	 * old "row" alive until **AFTER** it is overwritten
	 * by the next row, meaning that we get here during
	 * every iteration, having to detach the old row.
	 *
	 * Is there some way to prevent that?
	 *
	 * This doesn't happen when writing like this:
	 * >> for (__stack local row: db.query("SELECT * FROM foo"))
	 * >>     print repr row;
	 *
	 * Similarly, when executed using _hostasm, we don't
	 * get here either (even in user-code without __stack)
	 * Reason is that _hostasm actively tries to minimize
	 * the effective life-time of all variables */
	rowfmt = self->q_rowfmt;
	if unlikely(rowfmt == NULL) {
		/* Must allocate new row format descriptor */
		rowfmt = _Query_TryNewRowFmtOrUnlock(self);
		if unlikely(!ITER_ISOK(rowfmt)) {
			if unlikely(!rowfmt)
				goto err;
			result = QUERY_DETACHROWORUNLOCK_UNLOCKED;
			goto again;
		}
		self->q_rowfmt = rowfmt; /* Inherit reference */
	}

	if unlikely(!Row_LockTryWrite(row)) {
		Query_UnlockDB(self);
		Row_LockWaitRead(row);
decref_row_and_start_again:
		result = QUERY_DETACHROWORUNLOCK_UNLOCKED;
		Dee_Decref(row);
		if unlikely(Query_LockDB(self))
			goto err;
		goto again;
	}

	/* Check if "row" has already been detached. */
	if likely(row->r_query != NULL) {
		struct cell *rowdata;
		size_t i, ncol;
		sqlite3_stmt *stmt;
		ASSERT(row->r_query == self);
		ASSERT(row->r_rowfmt == NULL);
		ASSERT(row->r_cells == NULL);

		/* Allocate data to detach row. */
		ncol = rowfmt->rf_ncol;
		rowdata = (struct cell *)Dee_TryMallocc(ncol, sizeof(struct cell));
		if unlikely(!rowdata) {
			Row_LockEndWrite(row);
			Query_UnlockDB(self);
			if (!Dee_CollectMemoryc(ncol, sizeof(struct cell)))
				goto err_row;
			goto decref_row_and_start_again;
		}

		/* Initialize "rowdata" */
		stmt = self->q_stmt;
		for (i = 0; i < ncol; ++i) {
			unsigned int length;
			int type = sqlite3_column_type(stmt, (int)i);
			struct cell *c = &rowdata[i];
			switch (type) {

			case SQLITE_INTEGER:
				c->c_type = CELLTYPE_INT;
				c->c_data.d_int = sqlite3_column_int64(stmt, (int)i);
				break;

			case SQLITE_FLOAT:
				c->c_type = CELLTYPE_FLOAT;
				c->c_data.d_float = sqlite3_column_double(stmt, (int)i);
				break;

			case SQLITE_TEXT: {
				DREF DeeObject *string;
				unsigned char const *text;
				length = (unsigned int)sqlite3_column_bytes(stmt, (int)i);
				text = sqlite3_column_text(stmt, (int)i);
				if unlikely(!text) {
					bool ok;
unlock_and_collect_length_memory:
					Row_LockEndWrite(row);
					Query_UnlockDB(self);
					ok = Dee_CollectMemoryc((length + 1), sizeof(char));
					while (i--)
						cell_fini(&rowdata[i]);
					Dee_Free(rowdata);
					if (!ok)
						goto err_row;
					goto decref_row_and_start_again;
				}
				string = DeeString_TryNewUtf8((char const *)text, length,
				                              STRING_ERROR_FIGNORE);
				if unlikely(!string)
					goto unlock_and_collect_length_memory;
				c->c_type = CELLTYPE_OBJECT;
				c->c_data.d_obj = string; /* Inherit reference */
			}	break;

			case SQLITE_BLOB: {
				DREF DeeObject *bytes;
				void const *blob = NULL;
				length = (unsigned int)sqlite3_column_bytes(stmt, (int)i);
				if (length) {
					blob = sqlite3_column_blob(stmt, (int)i);
					if unlikely(!blob)
						goto unlock_and_collect_length_memory;
				}
				bytes = DeeBytes_TryNewBufferData(blob, length);
				if unlikely(!bytes)
					goto unlock_and_collect_length_memory;
				c->c_type = CELLTYPE_OBJECT;
				c->c_data.d_obj = bytes; /* Inherit reference */
			}	break;

			default:
				c->c_type = CELLTYPE_NONE;
				break;
			}
		}

		/* Detach the row by gifting it "rowdata" */
		ASSERT(row->r_query == self);
		ASSERT(row->r_rowfmt == NULL);
		ASSERT(row->r_cells == NULL);
		row->r_query = NULL;    /* Steal reference */
		Dee_DecrefNokill(self); /* Old reference from "row->r_query" */
		row->r_rowfmt = rowfmt;
		Dee_Incref(rowfmt);
		row->r_cells = rowdata; /* Gift data */
	}
	Row_LockEndWrite(row);

	/* Clear our weakref to the row -> we're done now!
	 * HINT: Clearing a weakref never invokes callbacks, so this is safe */
	Dee_weakref_clear(&self->q_row);

	/* Drop our reference to "row". Note that if this one ends up being
	 * destroyed, then we have to do so without holding any locks, since
	 * those locks might be re-acquired by string-fini-hooks that may
	 * have been enabled on cached strings stored within "row". */
	if unlikely(!Dee_DecrefIfNotOne(row)) {
		Query_UnlockDB(self);
		Dee_Decref(row);
		if unlikely(Query_LockDB(self))
			goto err;
		result = QUERY_DETACHROWORUNLOCK_UNLOCKED;
		goto again;
	}

	return result;
err_row:
	Dee_Decref(row);
err:
	return QUERY_DETACHROWORUNLOCK_ERR;
}

/* Same as `Query_LockDB()', but ensure that `q_row' is unbound,
 * and any potential old row has been detached (given its own copy
 * of cell data)
 * @return: 0 : Success
 * @return: -1: Error */
INTERN WUNUSED NONNULL((1)) int DCALL
Query_LockDBAndDetachRow(Query *__restrict self) {
	int result = Query_LockDB(self);
	if likely(result == 0) {
		STATIC_ASSERT(QUERY_DETACHROWORUNLOCK_ERR == -1);
		result = Query_DetachRowOrUnlock(self);
		if (result != QUERY_DETACHROWORUNLOCK_ERR)
			result = 0;
	}
	return result;
}


/* Return a reference to the Row descriptor of a given Query.
 * When the first step hasn't been executed yet, the returned
 * row will not contain any valid data,  */
INTERN WUNUSED NONNULL((1)) DREF Row *DCALL
Query_GetRow(Query *__restrict self) {
	DREF Row *existing_row;
	DREF Row *result = (DREF Row *)Dee_weakref_lock(&self->q_row);
	if (result)
		return result;
	/* Must allocate new row descriptor */
	result = Row_Alloc();
	if unlikely(!result)
		goto err;

	/* Initialize the new row */
	DeeObject_Init(result, &Row_Type);
	Dee_weakref_support_init(result);
	Dee_atomic_rwlock_init(&result->r_lock);
	result->r_query = self;
	Dee_Incref(self);
	result->r_rowfmt = NULL; /* Lazily allocated */
	result->r_cells  = NULL; /* Lazily allocated */

	/* Remember that this is the current row */
	if unlikely(Query_LockDB(self))
		goto err_r;
	existing_row = (DREF Row *)Dee_weakref_cmpxch(&self->q_row, NULL,
	                                              (DeeObject *)result);
	ASSERT(existing_row != (DREF Row *)ITER_DONE);
	Query_UnlockDB(self);

	/* Check for case: another thread also allocated the row */
	if unlikely(existing_row) {
		Dee_weakref_support_fini(result);
		Dee_DecrefNokill(self); /* result->r_query */
		Dee_DecrefNokill(&Row_Type);
		Row_Free(result);
		return existing_row;
	}
	return result;
err_r:
	Dee_weakref_support_fini(result);
	Dee_DecrefNokill(self); /* result->r_query */
	Dee_DecrefNokill(&Row_Type);
	Row_Free(result);
err:
	return NULL;
}

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
	if unlikely(Query_LockDB(self))
		goto err;
	result = sqlite3_expanded_sql(self->q_stmt);
	Query_UnlockDB(self);
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
 * Returns "(DREF Row *)ITER_DONE" when there are no more rows. */
INTERN WUNUSED NONNULL((1)) DREF Row *DCALL
Query_Step(Query *__restrict self) {
	int rc;
	DREF Row *result;

	/* Allocate the row */
	result = Row_Alloc();
	if unlikely(!result)
		goto err;

	/* Lock query and DB, and detach old row */
again_with_row:
	if unlikely(Query_LockDBAndDetachRow(self))
		goto err_r;
	ASSERT(Dee_weakref_getaddr(&self->q_row) == NULL);

	/* Perform the SQL step */
	rc = sqlite3_step(self->q_stmt);
	if unlikely(rc != SQLITE_ROW) {
		if likely(rc == SQLITE_DONE) {
			Query_UnlockDB(self);
			Row_Free(result);
			return (DREF Row *)ITER_DONE;
		}
		rc = err_sql_throwerror(rc, ERR_SQL_THROWERROR_F_UNLOCK_DB, self->q_db, NULL);
		if unlikely(rc)
			goto err_r;
		goto again_with_row;
	}

	/* Initialize the new row and remember it */
	DeeObject_Init(result, &Row_Type);
	Dee_weakref_support_init(result);
	Dee_atomic_rwlock_init(&result->r_lock);
	result->r_query = self;
	Dee_Incref(self);
	result->r_rowfmt = NULL; /* Lazily allocated */
	result->r_cells  = NULL; /* Lazily allocated */
	Dee_weakref_set_forced(&self->q_row, (DeeObject *)result);

	/* Unlock DB */
	Query_UnlockDB(self);
	return result;
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
	int rc;
	uint64_t changes = 0;
	uint64_t changes_at_start;
	DB *db = self->q_db;
again:
	if unlikely(Query_LockDBAndDetachRow(self))
		goto err;
	changes_at_start = (uint64_t)sqlite3_changes64(db->db_db);
	do {
		rc = sqlite3_step(self->q_stmt);
	} while (rc == SQLITE_ROW);
	changes += (uint64_t)sqlite3_changes64(db->db_db) - changes_at_start;
	if (rc != SQLITE_DONE && rc != SQLITE_OK) {
		rc = err_sql_throwerror(rc, ERR_SQL_THROWERROR_F_UNLOCK_DB, db, NULL);
		if (rc == 0)
			goto again;
		goto err;
	}
	Query_UnlockDB(self);
	return changes;
err:
	return (uint64_t)-1;
}

/* Skip at most `count' rows, returning the actual # of skipped rows.
 * @return: (uint64_t)-1: Error
 * @return: * : The # of skipped rows */
INTERN WUNUSED NONNULL((1)) uint64_t DCALL
Query_Skip(Query *__restrict self, uint64_t count) {
	int rc;
	uint64_t result = 0;
	DB *db = self->q_db;
	if unlikely(!count)
		return 0;
again:
	if unlikely(Query_LockDBAndDetachRow(self))
		goto err;
	do {
		rc = sqlite3_step(self->q_stmt);
		if (rc != SQLITE_ROW)
			break;
	} while (++result < count);
	if (rc != SQLITE_DONE && rc != SQLITE_OK) {
		rc = err_sql_throwerror(rc, ERR_SQL_THROWERROR_F_UNLOCK_DB, db, NULL);
		if (rc == 0)
			goto again;
		goto err;
	}
	Query_UnlockDB(self);
	return result;
err:
	return (uint64_t)-1;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
query_exec(Query *__restrict self, size_t argc, DeeObject *const *argv) {
	uint64_t rows;
	DeeArg_Unpack0(err, argc, argv, "exec");
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
	DeeArg_Unpack0(err, argc, argv, "step");
	result = Dee_AsObject(Query_Step(self));
	if (result == ITER_DONE)
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
	DeeArg_Unpack1X(err, argc, argv, "skip", &args.count, UNPu64, DeeObject_AsUInt64);
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
	/* TODO: fetchone()->?GRow                 (read 1 row, assert that there are no other rows, then return that row as a record)  */
	/* TODO: fetchall()->?S?GRow               (same as "this.frozen") */
	/* TODO: fetch(limit:?Dint)->?S?GRow       (same as "this[:limit].frozen") */

	/* TODO: Directly expose sqlite3_reset() */
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

PRIVATE struct type_member tpconst query_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &QueryIterator_Type),
	TYPE_MEMBER_CONST("ItemType", &Row_Type),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject Query_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "Query",
	/* .tp_doc      = */ DOC("TODO"),
	/* NOTE: "TP_FVARIABLE" because of our custom destroy function
	 *       (so must prevent _hostasm from using stack allocation) */
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL | TP_FVARIABLE,
	/* .tp_weakrefs = */ Dee_WEAKREF_SUPPORT_ADDR(Query),
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_VAR(
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ NULL,
			/* tp_deep_ctor:   */ NULL,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ NULL, // TODO: &query_init_kw,
			/* tp_serialize:   */ NULL, /* Queries can't be serialized */
			/* tp_free:        */ NULL
		),
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
	/* .tp_class_members = */ query_class_members
};

DECL_END

#endif /* !GUARD_DEX_SQLITE3_QUERY_C */
