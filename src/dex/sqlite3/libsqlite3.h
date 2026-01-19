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
#ifndef GUARD_DEX_SQLITE3_LIBSQLITE3_H
#define GUARD_DEX_SQLITE3_LIBSQLITE3_H 1

#include <deemon/api.h>

#include <deemon/alloc.h>
#include <deemon/bytes.h>
#include <deemon/error_types.h>
#include <deemon/gc.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/system-features.h>
#include <deemon/thread.h>
#include <deemon/types.h>
#include <deemon/util/atomic.h>
#include <deemon/util/lock.h>

#include <hybrid/sequence/list.h>

#include "sqlite3-external.h"

#include <stddef.h>
#include <stdint.h>

DECL_BEGIN


/************************************************************************/
/* Library Types                                                        */
/************************************************************************/
typedef struct cellfmt_object /*  */ CellFmt;       /* CellFmt_Type */
typedef struct rowfmt_columns_object RowFmtColumns; /* RowFmtColumns_Type */
typedef struct rowfmt_object /*   */ RowFmt;        /* RowFmt_Type */
typedef struct row_object /*      */ Row;           /* Row_Type */
typedef struct query_object /*    */ Query;         /* Query_Type */
typedef struct db_object /*       */ DB;            /* DB_Type */
TAILQ_HEAD(query_tailq, query_object);

INTDEF DeeTypeObject CellFmt_Type;       /* Item type of "_RowFmt" */
INTDEF DeeTypeObject RowFmtColumns_Type; /* {string...} -- Type for "_RowFmt.columns" and "Row.keys" */
INTDEF DeeTypeObject RowFmt_Type;
INTDEF DeeTypeObject Row_Type;
INTDEF DeeTypeObject QueryIterator_Type;
INTDEF DeeTypeObject Query_Type;
INTDEF DeeTypeObject DB_Type;



/************************************************************************/
/* >>> DB                                                               */
/************************************************************************/

struct query_cache_list {
	size_t                           qcl_count;    /* [lock(:db_querycache_lock)] # of queries (only `0' for `query_cache_empty_list_PTR') */
#ifndef Dee_MallocUsableSizeNonNull
	size_t                           qcl_alloc;    /* [const] Allocated space */
#define query_cache_list_getalloc(self)    ((self)->qcl_alloc)
#define query_cache_list_setalloc(self, v) ((self)->qcl_alloc = (v))
#else /* !Dee_MallocUsableSizeNonNull */
#define query_cache_list_getalloc(self) \
	((self) == query_cache_empty_list_PTR ? 0 : ((Dee_MallocUsableSizeNonNull(self) - offsetof(struct query_cache_list, qcl_queries)) / sizeof(Query *)))
#define query_cache_list_setalloc(self, v) (void)0
#endif /* Dee_MallocUsableSizeNonNull */
	COMPILER_FLEXIBLE_ARRAY(Query *, qcl_queries); /* [1..1][lock(:db_querycache_lock)]
	                                                * Similarly-hashed queries (sorted by "addrof(q_sql) ASC")
	                                                * NOTE: There may be multiple queries for the same string! */
};

#define query_cache_list_sizeof(count)                                   \
	_Dee_MallococBufsize(offsetof(struct query_cache_list, qcl_queries), \
	                     count, sizeof(Query *))
#define query_cache_list_alloc(count)                                                        \
	((struct query_cache_list *)Dee_Mallococ(offsetof(struct query_cache_list, qcl_queries), \
	                                         count, sizeof(Query *)))
#define query_cache_list_trycalloc(count)                                                       \
	((struct query_cache_list *)Dee_TryCallococ(offsetof(struct query_cache_list, qcl_queries), \
	                                            count, sizeof(Query *)))
#define _query_cache_list_tryrealloc(self, count)                                                      \
	((struct query_cache_list *)Dee_TryReallococ(self, offsetof(struct query_cache_list, qcl_queries), \
	                                             count, sizeof(Query *)))
#define query_cache_list_tryrealloc(self, count)                                      \
	((self) != query_cache_empty_list_PTR ? _query_cache_list_tryrealloc(self, count) \
	                                      : query_cache_list_trycalloc(count))
#define _query_cache_list_free(self) Dee_Free(self)
#define query_cache_list_free(self)  ((self) != query_cache_empty_list_PTR ? _query_cache_list_free(self) : (void)0)

struct query_cache_empty_list_struct {
	size_t qcl_count;
#ifndef Dee_MallocUsableSizeNonNull
	size_t qcl_alloc;
#endif /* !Dee_MallocUsableSizeNonNull */
};
INTDEF struct query_cache_empty_list_struct const query_cache_empty_list_;
#define query_cache_empty_list_PTR ((struct query_cache_list *)&query_cache_empty_list_)
#define query_cache_list_remove(self, index)               \
	(void)(--(self)->qcl_count,                            \
	       memmovedownc(&(self)->qcl_queries[(index)],     \
	                    &(self)->qcl_queries[(index) + 1], \
	                    (self)->qcl_count - (index),       \
	                    sizeof(Query *)))
#define query_cache_list_insert(self, index, query)      \
	(void)(memmoveupc(&(self)->qcl_queries[(index) + 1], \
	                  &(self)->qcl_queries[(index)],     \
	                  (self)->qcl_count - (index),       \
	                  sizeof(Query *)),                  \
	       (self)->qcl_queries[(index)] = (query),       \
	       ++(self)->qcl_count)

/* Returns an index into `self->qcl_queries' of some query compiled against `string'
 * If no such query exists, `self->qcl_count' is returned. */
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) size_t DCALL
query_cache_list_indexof(struct query_cache_list const *__restrict self,
                         DeeStringObject const *__restrict string);

#ifndef DB_QUERYCACHE_SIZE
#define DB_QUERYCACHE_SIZE 64
#endif /* !DB_QUERYCACHE_SIZE */

#ifndef DEFAULT_DB_QUERYCACHE_UNUSED_LIMIT
#define DEFAULT_DB_QUERYCACHE_UNUSED_LIMIT 512
#endif /* !DEFAULT_DB_QUERYCACHE_UNUSED_LIMIT */

/* Returns an index into `DB::db_querycache' for a given `DeeStringObject *string_ob' */
#define DB_QUERYCACHE_HASHOF(string_ob) \
	(Dee_HashPointer(string_ob) & (DB_QUERYCACHE_SIZE - 1))

struct db_string_fini_hook {
	struct Dee_string_fini_hook dsfh_hook; /* Underlying hook */
	WEAKREF(DB)                 dsfh_db;   /* [0..1] Weak reference to DB */
};
#define db_string_fini_hook_alloc()    DeeObject_MALLOC(struct db_string_fini_hook)
#define db_string_fini_hook_free(self) DeeObject_FREE(Dee_REQUIRES_TYPE(struct db_string_fini_hook *, self))
#define db_string_fini_hook_fromhooks(ptr) \
	COMPILER_CONTAINER_OF(ptr, struct db_string_fini_hook, dsfh_hook)
#define db_string_fini_hook_getdb(self) \
	((DREF DB *)Dee_weakref_lock(&(self)->dsfh_db))


struct db_thread_interrupt_hook {
	struct Dee_thread_interrupt_hook dtih_hook; /* Underlying hook */
	WEAKREF(DB)                      dtih_db;   /* [0..1] Weak reference to DB */
};
#define db_thread_interrupt_hook_alloc()    DeeObject_MALLOC(struct db_thread_interrupt_hook)
#define db_thread_interrupt_hook_free(self) DeeObject_FREE(Dee_REQUIRES_TYPE(struct db_thread_interrupt_hook *, self))
#define db_thread_interrupt_hook_fromhooks(ptr) \
	COMPILER_CONTAINER_OF(ptr, struct db_thread_interrupt_hook, dtih_hook)
#define db_thread_interrupt_hook_getdb(self) \
	((DREF DB *)Dee_weakref_lock(&(self)->dtih_db))


struct db_object {
	OBJECT_HEAD
	sqlite3                 *db_db;                      /* [?..1][const] Database context */
	sqlite3_stmt            *db_freelist;                /* [0..N][lock(ATOMIC)] List of statements that should be finalized */
#ifndef CONFIG_NO_THREADS
	Dee_shared_lock_t        db_dblock;                  /* Lock for `db_db' */
#endif /* !CONFIG_NO_THREADS */
	DeeThreadObject         *db_thread;                  /* [0..1][lock(db_dblock)] Thread that is holding `db_dblock' */
	size_t                   db_querycache_size;         /* [lock(db_querycache_lock)] Current size of the query cache */
	struct query_cache_list *db_querycache[DB_QUERYCACHE_SIZE]; /* [1..1][lock(db_querycache_lock)] Query cache. */
	struct query_tailq       db_querycache_unused;       /* [0..n][lock(db_querycache_lock)] List of queries that aren't in use (sorted by least-recently-used to most-recently-used) */
	size_t                   db_querycache_unused_count; /* [lock(db_querycache_lock)] # of elements in `db_querycache_unused' */
	size_t                   db_querycache_unused_limit; /* [lock(db_querycache_lock)] Max value of `db_querycache_unused_count' before old queries are destroyed (even if their SQL strings weren't destroyed, yet) */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_rwlock_t      db_querycache_lock;         /* Lock for `db_querycache' */
#endif /* !CONFIG_NO_THREADS */
	DREF struct db_string_fini_hook      *db_sf_hook;    /* [1..1][const] DeeStringObject finalization hook */
	DREF struct db_thread_interrupt_hook *db_ti_hook;    /* [1..1][const] Thread interrupt hook */
	WEAKREF_SUPPORT                                      /* Weak referencing support */
};

/* Use these to serialize all sqlite3 calls that may access the DB (`sqlite3 *') */
INTDEF WUNUSED NONNULL((1)) bool DCALL DB_TryLock(DB *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL DB_Lock(DB *__restrict self);
INTDEF NONNULL((1)) void DCALL DB_Unlock(DB *__restrict self);
#define DB_WaitFor(self) Dee_shared_lock_waitfor(&(self)->db_dblock)
#define DB_IsInterruptible(self, thread) (atomic_read(&(self)->db_thread) == (thread))

/* Safely call `sqlite3_finalize(stmt)' */
INTDEF NONNULL((1)) void DCALL DB_FinalizeStmt(DB *__restrict self, sqlite3_stmt *stmt);

#define DB_querycache_unused_insert(self, query) \
	(void)(++(self)->db_querycache_unused_count, \
	       TAILQ_INSERT_TAIL(&(self)->db_querycache_unused, query, q_unused))
#define DB_querycache_unused_remove(self, query)                         \
	(void)(TAILQ_REMOVE(&(self)->db_querycache_unused, query, q_unused), \
	       --(self)->db_querycache_unused_count)

#define DB_QueryCache_LockReading(self)    Dee_atomic_rwlock_reading(&(self)->db_querycache_lock)
#define DB_QueryCache_LockWriting(self)    Dee_atomic_rwlock_writing(&(self)->db_querycache_lock)
#define DB_QueryCache_LockTryRead(self)    Dee_atomic_rwlock_tryread(&(self)->db_querycache_lock)
#define DB_QueryCache_LockTryWrite(self)   Dee_atomic_rwlock_trywrite(&(self)->db_querycache_lock)
#define DB_QueryCache_LockCanRead(self)    Dee_atomic_rwlock_canread(&(self)->db_querycache_lock)
#define DB_QueryCache_LockCanWrite(self)   Dee_atomic_rwlock_canwrite(&(self)->db_querycache_lock)
#define DB_QueryCache_LockWaitRead(self)   Dee_atomic_rwlock_waitread(&(self)->db_querycache_lock)
#define DB_QueryCache_LockWaitWrite(self)  Dee_atomic_rwlock_waitwrite(&(self)->db_querycache_lock)
#define DB_QueryCache_LockRead(self)       Dee_atomic_rwlock_read(&(self)->db_querycache_lock)
#define DB_QueryCache_LockWrite(self)      Dee_atomic_rwlock_write(&(self)->db_querycache_lock)
#define DB_QueryCache_LockTryUpgrade(self) Dee_atomic_rwlock_tryupgrade(&(self)->db_querycache_lock)
#define DB_QueryCache_LockUpgrade(self)    Dee_atomic_rwlock_upgrade(&(self)->db_querycache_lock)
#define DB_QueryCache_LockDowngrade(self)  Dee_atomic_rwlock_downgrade(&(self)->db_querycache_lock)
#define DB_QueryCache_LockEndWrite(self)   Dee_atomic_rwlock_endwrite(&(self)->db_querycache_lock)
#define DB_QueryCache_LockEndRead(self)    Dee_atomic_rwlock_endread(&(self)->db_querycache_lock)
#define DB_QueryCache_LockEnd(self)        Dee_atomic_rwlock_end(&(self)->db_querycache_lock)

/* Free the oldest entry from "db_querycache_unused" and call "DB_QueryCache_LockEndWrite()" */
INTDEF NONNULL((1)) void DCALL
db_free_oldest_unused_query_and_unlock(DB *__restrict self);

/* The main function for compiling strings as SQL code. This function automatically
 * ties for make use the query cache to re-use previously used instances of queries
 * compiled against the same `sql', so-long as `sql' hasn't gotten destroyed in the
 * mean time:
 *
 * - Search `db_querycache' for a pre-existing Query linked against `sql'
 *   - Only consider queries that aren't in use (iow: `!Query_InUse(query)')
 *   - If one such query is found, set its `ob_refcnt = 1' (thus marking it
 *     as in-use) and return it.
 *   - If no such query is found, create+compile a new Query and insert it
 *     into the `db_querycache' of the database.
 * - When a query is created, we `DeeString_EnableFiniHook(sql)' so we get notified
 *   if a string that may appear in `Query::q_sql' is destroyed. Only once that has
 *   happened, will we:
 *   - call `sqlite3_finalize()' to destroy `Query::q_stmt'
 *   - Remove the query from the associated DB's `db_querycache'
 *   - Actually DeeObject_FREE() the query
 * - When the query is destroyed normally (its ob_refcnt hits 0):
 *   - AtomicCompareExchange refcnt of `q_sql' from 1 to 0:
 *     - If successful, decref `q_db' and then destroy `q_sql' (its string-fini-hook
 *       will do all remaining cleanup)
 *     - Otherwise, call `sqlite3_reset()' and `sqlite3_clear_bindings()' on the query
 *     - Lock the query cache of `q_db'
 *     - Add the query to the unused list of `q_db'
 *     - Unlock the query cache of `q_db'
 *     - Decref `q_db'
 *     - Decref `q_sql'
 *
 *
 * NOTE: The "Query" object returned here is *NEVER* DeeObject_IsShared!
 *       iow: `return->ob_refcnt == 1'
 *
 * @param: p_utf8_offset_of_next_stmt: when non-NULL, given `sql' is allowed to
 *                                     contain multiple SQL statements, and this
 *                                     pointer is set to the byte-offset within
 *                                     the UTF-8 representation of `sql', of the
 *                                     start of the next statement. If there was
 *                                     only 1 statement, this is set to `0'.
 *                                     When NULL, an error is thrown if `sql'
 *                                     contains more than 1 statement.
 * @return: DB_NEWQUERY_NOQUERY: Indicates that no SQL was compiled (query is empty or just a comments) */
INTDEF WUNUSED NONNULL((1, 2)) DREF Query *DCALL
DB_NewQuery(DB *__restrict self, DeeStringObject *__restrict sql,
            size_t *p_utf8_offset_of_next_stmt);
#define DB_NEWQUERY_NOQUERY ((DREF Query *)ITER_DONE)












/************************************************************************/
/* >>> QUERY                                                            */
/************************************************************************/
struct query_object {
	OBJECT_HEAD                         /* [@ob_type: ref_if(true)] */
	WEAKREF_SUPPORT                     /* [valid_if(Query_InUse(self))] Weak references */
	DREF DB                  *q_db;     /* [ref_if(Query_InUse(self))][1..1][const] Associated database */
	DREF DeeStringObject     *q_sql;    /* [ref_if(Query_InUse(self))][1..1][const] DeeStringObject that was used to compile this query */
	WEAKREF(Row)              q_row;    /* [0..1][lock(READ(API), WRITE(q_db->q_db && API))][valid_if(Query_InUse(self))] Cached row pointer (during `sqlite3_step()', if this weakref is still valid, this row is updated with copies of data from all columns) */
	DREF RowFmt              *q_rowfmt; /* [valid_if(!WAS_DESTROYED(q_sql))][owned][0..1][lock(READ(ATOMIC), WRITE(q_db->q_db && WRITE_ONCE))] Descriptor for how rows are formatted */
	sqlite3_stmt             *q_stmt;   /* [valid_if(!WAS_DESTROYED(q_sql))][owned][?..1][const]
	                                     * Statement context. This only gets finalized when `q_sql' is destroyed */
	size_t                    q_sql_utf8_nextstmt_offset; /* [const] Byte-offset into UTF-8 repr of `q_sql' to the start some other statement (or `0' if there is none) */
	TAILQ_ENTRY(query_object) q_unused; /* [0..1][lock(q_db->db_querycache_lock)][if(Query_InUse(self), [0..0])] Entry in list of unused queries. */
};
#define Query_TryLockDB(self) DB_TryLock((self)->q_db)
#define Query_LockDB(self)    DB_Lock((self)->q_db)
#define Query_UnlockDB(self)  DB_Unlock((self)->q_db)
#define Query_WaitForDB(self) DB_WaitFor((self)->q_db)

/* Common initialization for re-use after "ob_refcnt" was "0" */
#define Query_InitCommon(self)              \
	(Dee_Incref((self)->q_db),              \
	 Dee_Incref((self)->q_sql),             \
	 Dee_weakref_initempty(&(self)->q_row), \
	 weakref_support_init(self),            \
	 TAILQ_ENTRY_UNBOUND_INIT(&result->q_unused))

/* Common finalization for when "ob_refcnt == 0" */
#define Query_FiniCommon(self)         \
	(Dee_weakref_fini(&(self)->q_row), \
	 weakref_support_fini(self))

/* Allocate new query */
#define Query_Alloc() DeeObject_MALLOC(Query)

/* First-time initialization */
#define Query_InitOnce(self, db, sql)   \
	(DeeObject_Init(self, &Query_Type), \
	 (self)->q_db     = (db),           \
	 (self)->q_sql    = (sql),          \
	 (self)->q_rowfmt = NULL,           \
	 Query_InitCommon(self))

/* Last-time finalization (+ free) */
#define Query_Free(self, db) \
	(DB_FinalizeStmt(db, (self)->q_stmt), _Query_Free(self))
#define _Query_Free(self)                    \
	(Dee_XDecref((self)->q_rowfmt),          \
	 ASSERT((self)->ob_type == &Query_Type), \
	 DeeObject_FREE(self),                   \
	 Dee_DecrefNokill(&Query_Type))

/* Check if the query is in-use (returns "false" even if the query hasn't been marked
 * as unused, yet, meaning it is still being finalized and pre-pared for re-use) */
#define Query_InUse(self) (atomic_read(&(self)->ob_refcnt) != 0)

/* Check if the query was marked as unused (caller must be holding `q_db->db_querycache_lock') */
#define Query_IsUnused(self) TAILQ_ISBOUND(self, q_unused)

/* Return (and lazily allocate on first use) the RowFmt descriptor of this query. */
INTDEF WUNUSED NONNULL((1)) RowFmt *DCALL Query_GetRowFmt(Query *__restrict self);

/* Same as `Query_LockDB()', but ensure that `q_row' is unbound,
 * and any potential old row has been detached (given its own copy
 * of cell data)
 * @return: 0 : Success
 * @return: -1: Error */
INTDEF WUNUSED NONNULL((1)) int DCALL Query_LockDBAndDetachRow(Query *__restrict self);

/* Return a reference to the Row descriptor of a given Query.
 * When the first step hasn't been executed yet, the returned
 * row will not contain any valid data,  */
INTDEF WUNUSED NONNULL((1)) DREF Row *DCALL Query_GetRow(Query *__restrict self);

/* Advance the query by 1 step and return the resulting row.
 * Returns "(DREF Row *)ITER_DONE" when there are no more rows. */
INTDEF WUNUSED NONNULL((1)) DREF Row *DCALL Query_Step(Query *__restrict self);


/* Execute `self' until there is no more data present.
 * @return: (uint64_t)-1: Error
 * @return: * : The # of affected rows */
INTDEF WUNUSED NONNULL((1)) uint64_t DCALL Query_Exec(Query *__restrict self);

/* Skip at most `count' rows, returning the actual # of skipped rows.
 * @return: (uint64_t)-1: Error
 * @return: * : The # of skipped rows */
INTDEF WUNUSED NONNULL((1)) uint64_t DCALL Query_Skip(Query *__restrict self, uint64_t count);





/************************************************************************/
/* >>> CELL                                                             */
/************************************************************************/

/* Possible values for `struct cell::c_type' */
#define CELLTYPE_OBJECT 0
#define CELLTYPE_NONE   1
#define CELLTYPE_INT    2
#define CELLTYPE_FLOAT  3
struct cell {
	unsigned int c_type; /* One of `CELLTYPE_*' */
	union {
		DREF DeeObject *d_obj;   /* CELLTYPE_OBJECT */
		int64_t         d_int;   /* CELLTYPE_INT */
		double          d_float; /* CELLTYPE_FLOAT */
	} c_data;
};

#define cell_fini(self)                  \
	(((self)->c_type == CELLTYPE_OBJECT) \
	 ? Dee_Decref((self)->c_data.d_obj)  \
	 : (void)0)
#define cell_visit(self)                 \
	(((self)->c_type == CELLTYPE_OBJECT) \
	 ? Dee_Visit((self)->c_data.d_obj)   \
	 : (void)0)

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL cell_getvalue(struct cell *__restrict self);
INTDEF NONNULL((1)) void DCALL cell_destroyrow(struct cell *__restrict data, unsigned int ncol);
INTDEF NONNULL((1, 3)) void DCALL cell_visitrow(struct cell *__restrict data, unsigned int ncol, Dee_visit_t proc, void *arg);

struct cellfmt {
	DREF DeeStringObject *cfmt_name;     /* [1..1] Name of this column (~ala `sqlite3_column_name()') */
	DREF DeeStringObject *cfmt_decltype; /* [0..1] Type of this column (~ala `sqlite3_column_decltype()') */
};
#define cellfmt_fini(self)          \
	(Dee_Decref((self)->cfmt_name), \
	 Dee_XDecref((self)->cfmt_decltype))

struct cellfmt_object {
	OBJECT_HEAD
	DREF RowFmt          *cfo_fmt;  /* [1..1][const] Linked row format */
	struct cellfmt const *cfo_cell; /* [1..1][const] struct cell being described */
};




/************************************************************************/
/* >>> ROW                                                              */
/************************************************************************/
struct rowfmt_columns_object {
	OBJECT_HEAD
	DREF RowFmt *rfc_fmt; /* [1..1][const] Linked row format */
};

struct rowfmt_object { /* Item type is "CellFmt" */
	OBJECT_HEAD
	unsigned int                            rf_ncol;  /* [const] # of columns in result set */
	COMPILER_FLEXIBLE_ARRAY(struct cellfmt, rf_cols); /* [const] cell specs */
};
#define RowFmt_Alloc(ncol) \
	((RowFmt *)DeeObject_Mallocc(offsetof(RowFmt, rf_cols), ncol, sizeof(struct cellfmt)))
#define RowFmt_TryAlloc(ncol) \
	((RowFmt *)DeeObject_TryMallocc(offsetof(RowFmt, rf_cols), ncol, sizeof(struct cellfmt)))
#define RowFmt_Sizeof(ncol) \
	_Dee_MallococBufsize(offsetof(RowFmt, rf_cols), ncol, sizeof(struct cellfmt))
#define RowFmt_Free(self) DeeObject_Free(self)

struct row_object {
	OBJECT_HEAD
	WEAKREF_SUPPORT
#ifndef CONFIG_NO_THREADS
	Dee_atomic_rwlock_t r_lock;   /* Lock for this row (needed to replace effective data
	                               * with copy if still alive during `sqlite3_step()') */
#endif /* !CONFIG_NO_THREADS */
	DREF Query         *r_query;  /* [0..1][lock(r_lock && r_query->q_db->db_dblock && CLEAR_ONCE)]
	                               * Original query (set to "NULL" when `sqlite3_step()' is called on query) */
	DREF RowFmt        *r_rowfmt; /* [0..1][if(r_query == NULL, [1..1])][lock(r_lock && WRITE_ONCE)]
	                               * Data-layout of rows (always non-NULL when "r_query == NULL") */
	struct cell        *r_cells;  /* [0..r_rowfmt->rf_ncol][if(r_query == NULL, [1..1])][lock(r_lock && WRITE_ONCE)][owned]
	                               * Data-blob for column (always non-NULL when "r_query == NULL") */
};

#define Row_LockReading(self)    Dee_atomic_rwlock_reading(&(self)->r_lock)
#define Row_LockWriting(self)    Dee_atomic_rwlock_writing(&(self)->r_lock)
#define Row_LockTryRead(self)    Dee_atomic_rwlock_tryread(&(self)->r_lock)
#define Row_LockTryWrite(self)   Dee_atomic_rwlock_trywrite(&(self)->r_lock)
#define Row_LockCanRead(self)    Dee_atomic_rwlock_canread(&(self)->r_lock)
#define Row_LockCanWrite(self)   Dee_atomic_rwlock_canwrite(&(self)->r_lock)
#define Row_LockWaitRead(self)   Dee_atomic_rwlock_waitread(&(self)->r_lock)
#define Row_LockWaitWrite(self)  Dee_atomic_rwlock_waitwrite(&(self)->r_lock)
#define Row_LockRead(self)       Dee_atomic_rwlock_read(&(self)->r_lock)
#define Row_LockWrite(self)      Dee_atomic_rwlock_write(&(self)->r_lock)
#define Row_LockTryUpgrade(self) Dee_atomic_rwlock_tryupgrade(&(self)->r_lock)
#define Row_LockUpgrade(self)    Dee_atomic_rwlock_upgrade(&(self)->r_lock)
#define Row_LockDowngrade(self)  Dee_atomic_rwlock_downgrade(&(self)->r_lock)
#define Row_LockEndWrite(self)   Dee_atomic_rwlock_endwrite(&(self)->r_lock)
#define Row_LockEndRead(self)    Dee_atomic_rwlock_endread(&(self)->r_lock)
#define Row_LockEnd(self)        Dee_atomic_rwlock_end(&(self)->r_lock)

#define Row_Alloc()    DeeObject_MALLOC(Row)
#define Row_Free(self) DeeObject_FREE(Dee_REQUIRES_TYPE(Row *, self))




/************************************************************************/
/* Deemon-specific helpers for SQLite API functions                     */
/************************************************************************/
INTDEF WUNUSED NONNULL((1, 4)) int DCALL
dee_sqlite3_bind_string(DB *__restrict db, sqlite3_stmt *stmt, int index, DeeStringObject *self);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL
dee_sqlite3_bind_bytes(DB *__restrict db, sqlite3_stmt *stmt, int index, DeeBytesObject *self);

/* Bind "ob" to query parameter "index" of "stmt"
 * @return: 0 : Success
 * @return: -1: An error was thrown */
INTDEF WUNUSED NONNULL((1, 4)) int DCALL
dee_sqlite3_bind_object(DB *__restrict db, sqlite3_stmt *stmt, int index, DeeObject *self);
#define T_SQL_OBJECT "?X6?Dstring?Dint?Dbool?N?Dfloat?DBytes"

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
INTDEF WUNUSED NONNULL((1, 3)) Dee_ssize_t DCALL
dee_sqlite3_bind_params(DB *__restrict db, sqlite3_stmt *stmt,
                        DeeObject *params, size_t unnamed_start);






/************************************************************************/
/* Error handling                                                       */
/************************************************************************/

/* Generic SQL error type. -- Only used for SQL-specific errors. If the
 * error code returned by SQL can be explained using a more relevant
 * `DeeError_*' type, that type of error is thrown instead. */
typedef struct {
	DeeSystemErrorObject  sqe_system;     /* Underlying system error (set to `sqlite3_system_errno()')
	                                       * Underlying message is set to `sqlite3_errmsg()' / `sqlite3_errstr()' */
	DREF DeeStringObject *sqe_errmsg;     /* [0..1][const] SQL error message */
	DREF DeeStringObject *sqe_sql;        /* [0..1][const] The failing SQL source code */
	int                   sqe_sqloffutf8; /* [valid_if(sqe_sql != NULL)][const] Byte offset into utf8-repr of "sqe_sql"
	                                       * where the error happened (when `sqlite3_error_offset()' would return `-1',
	                                       * we set `sqe_sql' to `NULL' to indicate that this isn't a SQL parser problem). */
	int                   sqe_ecode;      /* [const] SQLite error code (one of `') */
} SQLError;

INTDEF DeeTypeObject SQLError_Type;                /* extends DeeError_SystemError */
INTDEF DeeTypeObject /**/ SQLSyntaxError_Type;     /* extends SQLError_Type */
INTDEF DeeTypeObject /**/ SQLConstraintError_Type; /* extends SQLError_Type */

/* TODO: Special class for [TODO: what else?] */


/* Throw an error and possibly unlock the DB
 * @return: 0 : try again
 * @return: -1: Error was thrown */
INTDEF ATTR_COLD WUNUSED int DCALL err_sql_throwerror(int errcode, unsigned int flags, DB *db, DeeStringObject *sql);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_multiple_statements(DeeStringObject *__restrict sql);
#define ERR_SQL_THROWERROR_F_NORMAL    0x0000
#define ERR_SQL_THROWERROR_F_UNLOCK_DB 0x0001 /* The "db" argument is non-NULL and **MUST** be unlocked, no matter what will be returned. */

/************************************************************************/
/* Initialization                                                       */
/************************************************************************/

/* These functions are called when a "DB_Type" object is created/destroyed.
 * Internally, these keep a running counter such that:
 * - The first call does `sqlite3_initialize()' and throws an error if something went wrong
 * - The last call does `sqlite3_shutdown()'
 * @return: 0 : Success
 * @return: -1: An error was thrown */
INTDEF WUNUSED int DCALL libsqlite3_init(void);
INTDEF void DCALL libsqlite3_fini(void);


DECL_END

#endif /* !GUARD_DEX_SQLITE3_LIBSQLITE3_H */
