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
#ifndef GUARD_DEX_SQLITE3_ROW_C
#define GUARD_DEX_SQLITE3_ROW_C 1
#define CONFIG_BUILDING_LIBSQLITE3
#define DEE_SOURCE

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/error.h>
#include <deemon/float.h>
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

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
Cell_GetValue(Cell *__restrict self) {
	switch (self->c_type) {
	case CELLTYPE_OBJECT:
		return_reference(self->c_data.d_obj);
	case CELLTYPE_NONE:
		return_none;
	case CELLTYPE_INT:
		return DeeInt_NewInt64(self->c_data.d_int);
	case CELLTYPE_FLOAT:
		return DeeFloat_New(self->c_data.d_float);
	default: __builtin_unreachable();
	}
}

INTERN WUNUSED NONNULL((1)) int DCALL
Cell_Init(Cell *__restrict self, sqlite3_stmt *stmt, int col) {
	int type = sqlite3_column_type(stmt, col);
	switch (type) {
	case SQLITE_INTEGER:
		self->c_type = CELLTYPE_INT;
		self->c_data.d_int = sqlite3_column_int64(stmt, col);
		break;
	case SQLITE_FLOAT:
		self->c_type = CELLTYPE_FLOAT;
		self->c_data.d_float = sqlite3_column_double(stmt, col);
		break;
		break;
	case SQLITE_TEXT:
		self->c_type = CELLTYPE_OBJECT;
		self->c_data.d_obj = (DREF DeeObject *)dee_sqlite3_column_string(stmt, col);
		if unlikely(!self->c_data.d_obj)
			goto err;
		break;
	case SQLITE_BLOB:
		self->c_type = CELLTYPE_OBJECT;
		self->c_data.d_obj = (DREF DeeObject *)dee_sqlite3_column_bytes(stmt, col);
		if unlikely(!self->c_data.d_obj)
			goto err;
		break;
	default:
		self->c_type = CELLTYPE_NONE;
		break;
	}
	return 0;
err:
	return -1;
}

/* Allocate a copy of columns from "stmt" as cell data */
INTERN WUNUSED Cell *DCALL
Cell_NewRow(size_t ncol, sqlite3_stmt *stmt) {
	size_t i;
	Cell *result = (Cell *)Dee_Mallocc(ncol, sizeof(Cell));
	if unlikely(!result)
		goto err;
	for (i = 0; i < ncol; ++i) {
		Cell *cell = &result[i];
		if unlikely(Cell_Init(cell, stmt, i))
			goto err_r_icol;
	}
	return result;
err_r_icol:
	while (i--)
		Cell_Fini(&result[i]);
/*err_r:*/
	Dee_Free(result);
err:
	return NULL;
}

/* Destroy cell data */
INTERN NONNULL((1)) void DCALL
Cell_DestroyRow(Cell *__restrict data, size_t ncol) {
	while (ncol--)
		Cell_Fini(&data[ncol]);
	Dee_Free(data);
}

INTERN NONNULL((1, 3)) void DCALL
Cell_VisitRow(Cell *__restrict data, size_t ncol, Dee_visit_t proc, void *arg) {
	while (ncol--)
		Cell_Visit(&data[ncol]);
}





PRIVATE WUNUSED NONNULL((1)) DREF RowFmt *DCALL
_Query_NewRowFmt(Query *__restrict self) {
	DREF RowFmt *result;
	size_t i, ncol;
	/* Need to lock the query because:
	 * >> sqlite3_column_name:
	 * >> >> The returned string pointer is valid until either the prepared statement
	 * >> >> is destroyed by sqlite3_finalize() or until the statement is automatically
	 * >> >> reprepared by the first call to sqlite3_step() for a particular run or
	 * >> >> until the next call to sqlite3_column_name() or sqlite3_column_name16() on
	 * >> >> the same column.
	 *
	 * iow: sqlite3_column_name() being called twice invalidates the previous string,
	 *      meaning that particular function isn't thread-safe!
	 */
	if (Query_Acquire(self))
		goto err;
	ncol = (size_t)sqlite3_column_count(self->q_stmt);
	result = RowFmt_Alloc(ncol);
	if unlikely(!result)
		goto err_unlock;
	DeeObject_Init(result, &RowFmt_Type);
	result->rf_ncol = ncol;
	for (i = 0; i < ncol; ++i) {
		DREF DeeStringObject *nameob, *decltypeob;
		CellFmt *fmt = &result->rf_cols[i];
		char const *name, *decltype_;

		/* Allocate name */
		do {
			name = sqlite3_column_name(self->q_stmt, i);
			/* >> If sqlite3_malloc() fails [...] then a NULL pointer is returned */
		} while (!name && Dee_CollectMemory(32));
		if unlikely(!name)
			goto err_unlock_r_i;
		nameob = (DREF DeeStringObject *)DeeString_NewUtf8(name, strlen(name),
		                                          STRING_ERROR_FIGNORE);
		if unlikely(!nameob)
			goto err_unlock_r_i;
		fmt->cfmt_name = nameob; /* Inherit reference */

		/* Allocate decltype */
		do {
			decltype_ = sqlite3_column_decltype(self->q_stmt, i);
			/* >> If sqlite3_malloc() fails [...] then a NULL pointer is returned */
		} while (!decltype_ && Dee_CollectMemory(32));
		if unlikely(!decltype_) {
err_unlock_r_i_name:
			Dee_Decref_likely(fmt->cfmt_name);
			goto err_unlock_r_i;
		}
		decltypeob = (DREF DeeStringObject *)DeeString_NewUtf8(decltype_, strlen(decltype_),
		                                          STRING_ERROR_FIGNORE);
		if unlikely(!decltypeob)
			goto err_unlock_r_i_name;
		fmt->cfmt_decltype = decltypeob; /* Inherit reference */
	}
	Query_Release(self);
	return result;
err_unlock_r_i:
	while (i--) {
		CellFmt *fmt = &result->rf_cols[i];
		CellFmt_Fini(fmt);
	}
/*err_unlock_r:*/
	RowFmt_Free(result);
err_unlock:
	Query_Release(self);
err:
	return NULL;
}

/* Return (and lazily allocate on first use) the RowFmt descriptor of this query. */
INTERN WUNUSED NONNULL((1)) RowFmt *DCALL
Query_GetRowFmt(Query *__restrict self) {
	RowFmt *result = atomic_read(&self->q_rowfmt);
	if (!result) {
		result = _Query_NewRowFmt(self);
		if likely(result) {
			if unlikely(!atomic_cmpxch(&self->q_rowfmt, NULL, result)) {
				Dee_DecrefDokill(result);
				result = atomic_read(&self->q_rowfmt);
				ASSERT(result);
			}
		}
	}
	return result;
}


/* Ensure that `self->q_row' is either dead or NULL.
 * If it isn't, try to copy row data into "q_row", then clear the weakref. */
#define QUERY_DETACHROWORUNLOCK_OK       0    /* Success, and lock was never lost */
#define QUERY_DETACHROWORUNLOCK_UNLOCKED 1    /* Success, but query lock was released at one point */
#define QUERY_DETACHROWORUNLOCK_ERR      (-1) /* Error, and query lock was released */
INTERN WUNUSED NONNULL((1)) int DCALL
Query_DetachRowOrUnlock(Query *__restrict self) {
	int result = QUERY_DETACHROWORUNLOCK_OK;
	DREF Row *row;
	RowFmt *rowfmt;
	Cell *rowdata;
again:
	row = (DREF Row *)Dee_weakref_lock(&self->q_row);
	if (!row) /* Row isn't cached -> nothing to detach! */
		return result;
	rowfmt = atomic_read(&self->q_rowfmt);
	if unlikely(rowfmt == NULL) {
		/* Must allocate new row format descriptor */
		Query_Release(self);
		result = QUERY_DETACHROWORUNLOCK_UNLOCKED;
		Dee_Decref_unlikely(row);
		if unlikely(Query_GetRowFmt(self) == NULL)
			goto err;
		Query_Acquire(self);
		goto again;
	}

	/* Check if "row" has already been detached. */
	Row_LockRead(row);
	ASSERT((row->r_query == self) || (row->r_query == NULL));
	if (row->r_query == NULL) {
		Row_LockEndRead(row);
		goto done_decref_row;
	}
	ASSERT(row->r_rowfmt == NULL);
	ASSERT(row->r_cells == NULL);
	Row_LockEndRead(row);

	/* Allocate data to detach row. */
	/* TODO: Should use TryMalloc and release query lock for this one, too
	 *       Same deal when it comes to creating new string/bytes objects... */
	rowdata = Cell_NewRow(rowfmt->rf_ncol, self->q_stmt);
	if unlikely(!rowdata)
		goto err_row;

	/* Detach the row by gifting it "rowdata" */
	Row_LockWrite(row);
	ASSERT((row->r_query == self) || (row->r_query == NULL));
	if (row->r_query == NULL) {
		Row_LockEndWrite(row);
		Cell_DestroyRow(rowdata, rowfmt->rf_ncol);
		goto done_decref_row;
	}
	ASSERT(row->r_rowfmt == NULL);
	ASSERT(row->r_cells == NULL);
	row->r_query = NULL;    /* Steal reference */
	Dee_DecrefNokill(self); /* Old reference from "row->r_query" */
	row->r_rowfmt = rowfmt;
	Dee_Incref(rowfmt);
	row->r_cells = rowdata; /* Gift data */
	Row_LockEndWrite(row);

	/* Clear our weakref to the row -> we're done now! */
done_decref_row:
	Dee_weakref_clear(&self->q_row);
	Dee_Decref_unlikely(row);
	return result;
err_row:
	Dee_Decref_unlikely(row);
err:
	return QUERY_DETACHROWORUNLOCK_ERR;
}

/* Same as `Query_Acquire()', but ensure that `q_row' is unbound,
 * and any potential old row has been detached (given its own copy
 * of cell data)
 * @return: 0 : Success
 * @return: -1: Error */
INTERN WUNUSED NONNULL((1)) int DCALL
Query_AcquireAndDetachRow(Query *__restrict self) {
	int result = Query_Acquire(self);
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
	weakref_support_init(result);
	Dee_atomic_rwlock_init(&result->r_lock);
	result->r_query = self;
	Dee_Incref(self);
	result->r_rowfmt = NULL; /* Lazily allocated */
	result->r_cells  = NULL; /* Lazily allocated */

	/* Remember that this is the current row */
	if unlikely(Query_Acquire(self))
		goto err_r;
	existing_row = (DREF Row *)Dee_weakref_cmpxch(&self->q_row, NULL,
	                                              (DeeObject *)result);
	ASSERT(existing_row != (DREF Row *)ITER_DONE);
	Query_Release(self);

	/* Check for case: another thread also allocated the row */
	if unlikely(existing_row) {
		weakref_support_fini(result);
		Dee_DecrefNokill(self); /* result->r_query */
		Dee_DecrefNokill(&Row_Type);
		Row_Free(result);
		return existing_row;
	}
	return result;
err_r:
	weakref_support_fini(result);
	Dee_DecrefNokill(self); /* result->r_query */
	Dee_DecrefNokill(&Row_Type);
	Row_Free(result);
err:
	return NULL;
}










/************************************************************************/
/************************************************************************/
/*                                                                      */
/* RowFmt                                                               */
/*                                                                      */
/************************************************************************/
/************************************************************************/

PRIVATE NONNULL((1)) void DCALL
rowfmt_fini(RowFmt *__restrict self) {
	size_t i;
	for (i = 0; i < self->rf_ncol; ++i)
		CellFmt_Fini(&self->rf_cols[i]);
}

/* Returns the index of column "column_name", or `(size_t)-1' if not found */
PRIVATE NONNULL((1, 2)) size_t DCALL
rowfmt_indexof_string(RowFmt *__restrict self, char const *column_name) {
	size_t i;
	for (i = 0; i < self->rf_ncol; ++i) {
		CellFmt *cell = &self->rf_cols[i];
		if (DeeString_EqualsCStr(cell->cfmt_name, column_name))
			return i;
	}
	return (size_t)-1;
}

PRIVATE NONNULL((1, 2)) size_t DCALL
rowfmt_indexof_string_len(RowFmt *__restrict self,
                          char const *column_name,
                          size_t column_namelen) {
	size_t i;
	for (i = 0; i < self->rf_ncol; ++i) {
		CellFmt *cell = &self->rf_cols[i];
		if (DeeString_EqualsBuf(cell->cfmt_name, column_name, column_namelen))
			return i;
	}
	return (size_t)-1;
}


INTERN DeeTypeObject RowFmt_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_RowFmt",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor        = */ (Dee_funptr_t)NULL,
				/* .tp_copy_ctor   = */ (Dee_funptr_t)NULL,
				/* .tp_deep_ctor   = */ (Dee_funptr_t)NULL,
				/* .tp_any_ctor    = */ (Dee_funptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(RowFmt),
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&rowfmt_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ NULL,
		/* .tp_print     = */ NULL, // TODO: (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&rowfmt_print,
		/* .tp_printrepr = */ NULL, // TODO: (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&rowfmt_printrepr,
	},
	/* .tp_visit         = */ NULL, /* Not needed -- only references DeeStringObject objects */
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL, /* TODO: user-access to column names and types */
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL, /* TODO: user-access to column names and types */
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};












/************************************************************************/
/************************************************************************/
/*                                                                      */
/* Row                                                                  */
/*                                                                      */
/************************************************************************/
/************************************************************************/

PRIVATE NONNULL((1)) void DCALL
row_fini(Row *__restrict self) {
	if (self->r_query) {
		ASSERT(!self->r_rowfmt);
		ASSERT(!self->r_cells);
		Dee_Decref_unlikely(self->r_query);
	} else {
		ASSERT(self->r_rowfmt);
		ASSERT(self->r_cells);
		Cell_DestroyRow(self->r_cells, self->r_rowfmt->rf_ncol);
		Dee_Decref(self->r_rowfmt);
	}
	weakref_support_fini(self);
}

PRIVATE NONNULL((1, 2)) void DCALL
row_visit(Row *__restrict self, Dee_visit_t proc, void *arg) {
	Row_LockRead(self);
	if (self->r_query) {
		ASSERT(!self->r_rowfmt);
		ASSERT(!self->r_cells);
		Dee_Visit(self->r_query);
	} else {
		ASSERT(self->r_rowfmt);
		ASSERT(self->r_cells);
		Cell_VisitRow(self->r_cells, self->r_rowfmt->rf_ncol, proc, arg);
		Dee_Visit(self->r_rowfmt);
	}
	Row_LockEndRead(self);
}

PRIVATE WUNUSED NONNULL((1)) DREF Query *DCALL
row_getquery(Row *__restrict self) {
	DREF Query *result;
	Row_LockRead(self);
	result = self->r_query;
	if unlikely(!result) {
		Row_LockEndRead(self);
		DeeError_Throwf(&DeeError_UnboundAttribute,
		                "Unbound attribute `Row.query'");
		return NULL;
	}
	Dee_Incref(result);
	Row_LockEndRead(self);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
row_boundquery(Row *__restrict self) {
	int result;
	Row_LockRead(self);
	result = Dee_BOUND_FROMBOOL(self->r_query != NULL);
	Row_LockEndRead(self);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF RowFmt *DCALL
row_getfmt(Row *__restrict self) {
	DREF RowFmt *result;
	Row_LockRead(self);
	if (self->r_query) {
		DREF Query *query;
		ASSERT(!self->r_rowfmt);
		query = self->r_query;
		Dee_Incref(query);
		Row_LockEndRead(self);
		result = Query_GetRowFmt(query);
		Dee_XIncref(result);
		Dee_Decref(query);
	} else {
		ASSERT(self->r_rowfmt);
		result = self->r_rowfmt;
		Dee_Incref(result);
		Row_LockEndRead(self);
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
row_size_fast(Row *__restrict self) {
	size_t result;
	Row_LockRead(self);
	if (self->r_query) {
		result = (size_t)sqlite3_column_count(self->r_query->q_stmt);
	} else {
		result = self->r_rowfmt->rf_ncol;
	}
	Row_LockEndRead(self);
	return result;
}

/* Upgrade a read-lock to "self" with a full lock to "query"
 * @param: again: label to start over from scratch (called with no locks held)
 * @param: err:   label to jump to on error (called with no locks held)
 * @param: Row *self:    Row for which the caller current has a read-lock (this lock will be released)
 * @param: Query *query: Query that the caller wants a lock to (this lock will be acquired on success)
 */
#define Row_ReplaceReadLockWithQueryLock(again, err, self, query) \
	do {                                                          \
		if (!Query_TryAcquire(query)) {                           \
			Dee_Incref(query);                                    \
			Row_LockEndRead(self);                                \
			if unlikely(Query_Acquire(query)) {                   \
				Dee_Decref(query);                                \
				goto err;                                         \
			}                                                     \
			if unlikely(!Row_LockTryRead(self)) {                 \
				Query_Release(query);                             \
				Dee_Decref(query);                                \
				goto again;                                       \
			}                                                     \
			if unlikely((self)->r_query != query) {               \
				Query_Release(query);                             \
				Row_LockEndRead(self);                            \
				Dee_Decref(query);                                \
				goto again;                                       \
			}                                                     \
			Dee_DecrefNokill(query);                              \
		}                                                         \
		Row_LockEndRead(self);                                    \
	}	__WHILE0


PRIVATE WUNUSED NONNULL((1)) size_t DCALL
row_indexof_string(Row *__restrict self, char const *column_name) {
	size_t result;
	DREF RowFmt *fmt = row_getfmt(self);
	if unlikely(!fmt)
		goto err;
	result = rowfmt_indexof_string(fmt, column_name);
	Dee_Decref(fmt);
	return result;
err:
	return (size_t)-2;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
row_indexof_string_len(Row *__restrict self,
                       char const *column_name,
                       size_t column_namelen) {
	size_t result;
	DREF RowFmt *fmt = row_getfmt(self);
	if unlikely(!fmt)
		goto err;
	result = rowfmt_indexof_string_len(fmt, column_name, column_namelen);
	Dee_Decref(fmt);
	return result;
err:
	return (size_t)-2;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
row_getitem_index(Row *__restrict self, size_t index) {
	DREF DeeObject *result;
	Query *query;
	size_t length;
	int column_type;
again:
	Row_LockRead(self);
	if ((query = self->r_query) == NULL) {
		Cell *cell;
		Row_LockEndRead(self);
		length = self->r_rowfmt->rf_ncol;
		if unlikely(index >= length)
			goto err_oob;
		cell = &self->r_cells[index];
		return Cell_GetValue(cell);
	}

	/* Complicated (but likely) case: active row */
	Row_ReplaceReadLockWithQueryLock(again, err, self, query);

	/* Got a lock to the query -> read data from sqlite */
	length = (size_t)sqlite3_column_count(query->q_stmt);
	if unlikely(index >= length) {
		Query_Release(query);
		goto err_oob;
	}
	column_type = sqlite3_column_type(query->q_stmt, (int)index);
	switch (column_type) {
	case SQLITE_INTEGER: {
		int64_t value = sqlite3_column_int64(query->q_stmt, (int)index);
		Query_Release(query);
		result = DeeInt_NewInt64(value);
	}	break;
	case SQLITE_FLOAT: {
		double value = sqlite3_column_double(query->q_stmt, (int)index);
		Query_Release(query);
		result = DeeFloat_New(value);
	}	break;
	case SQLITE_TEXT:
		result = (DREF DeeObject *)dee_sqlite3_column_string(query->q_stmt, (int)index);
		Query_Release(query);
		break;
	case SQLITE_BLOB:
		result = (DREF DeeObject *)dee_sqlite3_column_bytes(query->q_stmt, (int)index);
		Query_Release(query);
		break;
	default:
		result = DeeNone_NewRef();
		Query_Release(query);
		break;
	}
	return result;
err:
	return NULL;
err_oob:
	err_index_out_of_bounds((DeeObject *)self, index, length);
	goto err;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
row_getitem_string_hash(Row *__restrict self, char const *name, Dee_hash_t UNUSED(hash)) {
	size_t index = row_indexof_string(self, name);
	if unlikely(index == (size_t)-2)
		return NULL;
	if unlikely(index == (size_t)-1) {
		err_unknown_key_str((DeeObject *)self, name);
		return NULL;
	}
	return row_getitem_index(self, index);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
row_getitem_string_len_hash(Row *__restrict self, char const *name, size_t namelen, Dee_hash_t UNUSED(hash)) {
	size_t index = row_indexof_string_len(self, name, namelen);
	if unlikely(index == (size_t)-2)
		return NULL;
	if unlikely(index == (size_t)-1) {
		err_unknown_key_str_len((DeeObject *)self, name, namelen);
		return NULL;
	}
	return row_getitem_index(self, index);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
row_hasitem_string_hash(Row *__restrict self, char const *name, Dee_hash_t UNUSED(hash)) {
	size_t index = row_indexof_string(self, name);
	if unlikely(index == (size_t)-2)
		return -1;
	if unlikely(index == (size_t)-1)
		return 0;
	return 1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
row_hasitem_string_len_hash(Row *__restrict self, char const *name, size_t namelen, Dee_hash_t UNUSED(hash)) {
	size_t index = row_indexof_string_len(self, name, namelen);
	if unlikely(index == (size_t)-2)
		return -1;
	if unlikely(index == (size_t)-1)
		return 0;
	return 1;
}

PRIVATE struct type_seq row_seq = {
	/* .tp_iter                       = */ NULL,
	/* .tp_sizeob                     = */ NULL,
	/* .tp_contains                   = */ NULL,
	/* .tp_getitem                    = */ NULL,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ NULL,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&row_size_fast,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&row_size_fast,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&row_getitem_index,
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
	/* .tp_getitem_string_hash        = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&row_getitem_string_hash,
	/* .tp_delitem_string_hash        = */ NULL,
	/* .tp_setitem_string_hash        = */ NULL,
	/* .tp_bounditem_string_hash      = */ NULL,
	/* .tp_hasitem_string_hash        = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&row_hasitem_string_hash,
	/* .tp_trygetitem_string_len_hash = */ NULL,
	/* .tp_getitem_string_len_hash    = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&row_getitem_string_len_hash,
	/* .tp_delitem_string_len_hash    = */ NULL,
	/* .tp_setitem_string_len_hash    = */ NULL,
	/* .tp_bounditem_string_len_hash  = */ NULL,
	/* .tp_hasitem_string_len_hash    = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&row_hasitem_string_len_hash,
};


PRIVATE struct type_getset tpconst row_getsets[] = {
	TYPE_GETTER_BOUND("query", &row_getquery, &row_boundquery,
	                  "->?GQuery\n"
	                  "Returns the query linked to this row. "
	                  /**/ "Throws :UnboundAttribute if @this row has been "
	                  /**/ "detached (which happens when another row is read)"),
	TYPE_GETTER_AB("_fmt", &row_getfmt,
	               "->?G_RowFmt\n"
	               "Returns the format descriptor for rows of the associated query"),
	/* TODO: asrecord */
	TYPE_GETSET_END
};

INTERN DeeTypeObject Row_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "Row",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor        = */ (Dee_funptr_t)NULL,
				/* .tp_copy_ctor   = */ (Dee_funptr_t)NULL,
				/* .tp_deep_ctor   = */ (Dee_funptr_t)NULL,
				/* .tp_any_ctor    = */ (Dee_funptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(Row),
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&row_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ NULL,
		/* .tp_print     = */ NULL,
		/* .tp_printrepr = */ NULL,
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&row_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &row_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ row_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

DECL_END

#endif /* !GUARD_DEX_SQLITE3_ROW_C */
