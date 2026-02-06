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
#ifndef GUARD_DEX_SQLITE3_DB_C
#define GUARD_DEX_SQLITE3_DB_C 1
#define CONFIG_BUILDING_LIBSQLITE3
#define DEE_SOURCE

#include "libsqlite3.h"
/**/

#include <deemon/api.h>

#include <deemon/alloc.h>           /* DeeObject_FREE, Dee_CollectMemory, Dee_MallocUsableSizeNonNull, Dee_TYPE_CONSTRUCTOR_INIT_FIXED */
#include <deemon/arg.h>             /* DeeArg_Unpack1, DeeArg_Unpack1Or2 */
#include <deemon/error.h>           /* DeeError_SyntaxError, DeeError_Throwf */
#include <deemon/format.h>          /* DeeFormat_PRINT */
#include <deemon/int.h>             /* DeeInt_NewSize, DeeInt_NewUInt64 */
#include <deemon/object.h>          /* DREF, DeeObject, DeeObject_*, DeeTypeObject, Dee_AsObject, Dee_Decref*, Dee_WEAKREF_SUPPORT_ADDR, Dee_formatprinter_t, Dee_ssize_t, Dee_weakref_support_fini, Dee_weakref_support_init, OBJECT_HEAD_INIT */
#include <deemon/string.h>          /* DeeString*, Dee_string_fini_hook, Dee_string_fini_hook_decref, STRING_ERROR_FIGNORE, WSTR_LENGTH */
#include <deemon/stringutils.h>     /* Dee_unicode_skipspaceutf8 */
#include <deemon/system-features.h> /* DeeSystem_DEFINE_memsetp, memcpy */
#include <deemon/thread.h>          /* DeeThreadObject, DeeThread_AddInterruptHook, DeeThread_DisableInterruptHooks, DeeThread_EnableInterruptHooks, DeeThread_RemoveInterruptHook, DeeThread_Self, DeeThread_WasInterrupted, Dee_thread_interrupt_hook, Dee_thread_interrupt_hook_decref */
#include <deemon/tuple.h>           /* Dee_EmptyTuple */
#include <deemon/type.h>            /* DeeObject_IsShared, DeeType_Type, Dee_TYPE_CONSTRUCTOR_INIT_FIXED, TF_NONE, TP_FNORMAL, TYPE_*, type_* */
#include <deemon/util/atomic.h>     /* atomic_* */
#include <deemon/util/lock.h>       /* Dee_atomic_rwlock_init, Dee_shared_lock_* */
#include <deemon/util/weakref.h>    /* Dee_weakref_fini, Dee_weakref_init */

#include <hybrid/sequence/list.h> /* TAILQ_* */
#include <hybrid/typecore.h>      /* __UINTPTR_TYPE__ */

#include "sqlite3-external.h"

#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* NULL, size_t */
#include <stdint.h>  /* uint64_t */

#ifndef INT_MAX
#include <hybrid/limitcore.h> /* __INT_MAX__ */
#define INT_MAX __INT_MAX__
#endif /* !INT_MAX */

DECL_BEGIN

#ifndef CONFIG_HAVE_memsetp
#define CONFIG_HAVE_memsetp
#define memsetp(dst, pointer, num_pointers) \
	dee_memsetp(dst, (__UINTPTR_TYPE__)(pointer), num_pointers)
DeeSystem_DEFINE_memsetp(dee_memsetp)
#endif /* !CONFIG_HAVE_memsetp */



INTERN_CONST struct query_cache_empty_list_struct const
query_cache_empty_list_ = {
	0
#ifndef Dee_MallocUsableSizeNonNull /* CONFIG_EXPERIMENTAL_CUSTOM_HEAP */
	, 0
#endif /* !Dee_MallocUsableSizeNonNull */
};

/* Returns an index into `self->qcl_queries' of some query compiled against `string'
 * If no such query exists, `self->qcl_count' is returned. */
INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) size_t DCALL
query_cache_list_indexof(struct query_cache_list const *__restrict self,
                         DeeStringObject const *__restrict string) {
	size_t lo = 0, hi = self->qcl_count;
	while (lo < hi) {
		size_t index = (lo + hi) >> 1;
		Query *query = self->qcl_queries[index];
		if (string < query->q_sql) {
			hi = index;
		} else if (string > query->q_sql) {
			lo = index + 1;
		} else {
			return index;
		}
	}
	return self->qcl_count;
}


/* Free the oldest entry from "db_querycache_unused" and call "DB_QueryCache_LockEndWrite()" */
INTERN NONNULL((1)) void DCALL
db_free_oldest_unused_query_and_unlock(DB *__restrict self) {
	Query *oldest_query = TAILQ_FIRST(&self->db_querycache_unused);
	size_t list_index = DB_QUERYCACHE_HASHOF(oldest_query->q_sql);
	struct query_cache_list *qc_list = self->db_querycache[list_index];
	size_t qc_index = query_cache_list_indexof(qc_list, oldest_query->q_sql);
	ASSERT(!Query_InUse(oldest_query));
	ASSERT(Query_IsUnused(oldest_query));
	ASSERT(qc_index < qc_list->qcl_count);
	ASSERT(qc_list != query_cache_empty_list_PTR);
	query_cache_list_remove(qc_list, qc_index);
	--self->db_querycache_size;
	TAILQ_REMOVE_HEAD(&self->db_querycache_unused, q_unused);
	--self->db_querycache_unused_count;
	if (qc_list->qcl_count == 0) {
		/* Last query removed from list -> replace with empty-list-pointer */
		self->db_querycache[list_index] = query_cache_empty_list_PTR;
		DB_QueryCache_LockEndWrite(self);
		_query_cache_list_free(qc_list);
	} else {
		DB_QueryCache_LockEndWrite(self);
	}

	/* Free "oldest_query" */
	Query_Free(oldest_query, self);
}








/************************************************************************/
/************************************************************************/
/*                                                                      */
/* DB                                                                   */
/*                                                                      */
/************************************************************************/
/************************************************************************/

SQLITE_API sqlite3_stmt *sqlite3_stmt_getnextfree(sqlite3_stmt *pStmt);
SQLITE_API void sqlite3_stmt_setnextfree(sqlite3_stmt *pStmt, sqlite3_stmt *pNext);

PRIVATE NONNULL((1)) void DCALL
db_free_list_clear(DB *__restrict self) {
	sqlite3_stmt *flist;
	flist = atomic_xch(&self->db_freelist, NULL);
	while (flist) {
		sqlite3_stmt *next;
		next = sqlite3_stmt_getnextfree(flist);
		(void)sqlite3_finalize(flist);
		flist = next;
	}
}

PRIVATE NONNULL((1, 2)) void DCALL
db_free_list_insert(DB *__restrict self, sqlite3_stmt *pStmt) {
	sqlite3_stmt *last;
	do {
		last = atomic_read(&self->db_freelist);
		sqlite3_stmt_setnextfree(pStmt, last);
	} while (!atomic_cmpxch_weak(&self->db_freelist, last, pStmt));
}

/* Use these to serialize all sqlite3 calls that may access the DB (`sqlite3 *') */
INTERN WUNUSED NONNULL((1)) bool DCALL
DB_TryLock(DB *__restrict self) {
	bool result = Dee_shared_lock_tryacquire(&self->db_dblock);
	if (result) {
		DeeThreadObject *me = DeeThread_Self();
		atomic_write(&self->db_thread, me);
		DeeThread_EnableInterruptHooks(me);
	}
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
DB_Lock(DB *__restrict self) {
	DeeThreadObject *me;
	if unlikely(Dee_shared_lock_acquire(&self->db_dblock))
		goto err;
	me = DeeThread_Self();
	atomic_write(&self->db_thread, me);
	DeeThread_EnableInterruptHooks(me);
	return 0;
err:
	return -1;
}

INTERN NONNULL((1)) void DCALL
DB_Unlock(DB *__restrict self) {
again:
	ASSERT(self->db_thread == DeeThread_Self());
	DeeThread_DisableInterruptHooks(self->db_thread);
	atomic_write(&self->db_thread, NULL);
	Dee_shared_lock_release(&self->db_dblock);
	if unlikely(atomic_read(&self->db_freelist) != NULL) {
		if (DB_TryLock(self)) {
			db_free_list_clear(self);
			goto again;
		}
	}
}


/* Safely call `sqlite3_finalize(stmt)' */
INTERN NONNULL((1)) void DCALL
DB_FinalizeStmt(DB *__restrict self, sqlite3_stmt *stmt) {
	if (DB_TryLock(self)) {
		(void)sqlite3_finalize(stmt);
		DB_Unlock(self);
	} else {
		/* This sqlite3_finalize() needs a DB lock -- only way to get that here
		 * is via a lockop (as seen in KOS).
		 *
		 * NOTE: Can even get here due to our own thread holding the DB lock:
		 * >> local s = "select * from foo";
		 * >> __asm__("" : "+X" (s)); // Prevent constant propagation
		 * >> local t = s.upper();    // runtime-string
		 * >> db.query(t);            // Create query for runtime-string
		 * >> db.query("INSERT INTO bar (v) VALUES (?)", t)...; // Bind query string in another statment
		 * >> [...]  // No more usages of "t" here
		 *
		 * Since to bind strings, we gift sqlite3 a reference to our string,
		 * *it* is allowed to call the destructor whenever it pleases, so it
		 * may actually call that destructor while the DB lock is held.
		 *
		 * If that happens, string-fini-hooks could invoke arbitrary user-code,
		 * which is something we can't prevent (so we're just going to ignore
		 * it), but it may also try to finalize the statement created by the
		 * initial "db.query(t)" call above, which could then lead us here in
		 * a context where it is our own thread that is locking the DB.
		 *
		 * NOTE: However, for us to be able to chain "stmt", sqlite3 needs some
		 *       kind of way for us to read/write a pointer-sized "user_data"
		 *       field to/from a "sqlite3_stmt" object.
		 */
		db_free_list_insert(self, stmt);

		/* In case the lock became available again in the meantime,
		 * reap the free-statement-list now so unused statements
		 * won't stay in there indefinitely. */
		if (DB_TryLock(self))
			DB_Unlock(self);
	}
}


PRIVATE WUNUSED NONNULL((1, 2)) Query *DCALL
db_find_unused_query_near(struct query_cache_list *__restrict list,
                          DeeStringObject *__restrict sql, size_t list_index) {
	size_t neighbor_list_index;
	Query *result = list->qcl_queries[list_index];
	if (Query_IsUnused(result))
		return result;

	/* Check if there are unused neighboring entries. */
	neighbor_list_index = list_index;
	while (neighbor_list_index > 0) {
		--neighbor_list_index;
		result = list->qcl_queries[neighbor_list_index];
		if (result->q_sql != sql)
			break;
		if (Query_IsUnused(result))
			return result;
	}
	neighbor_list_index = list_index;
	while (neighbor_list_index < (list->qcl_count - 1)) {
		++neighbor_list_index;
		result = list->qcl_queries[neighbor_list_index];
		if (result->q_sql != sql)
			break;
		if (Query_IsUnused(result))
			return result;
	}
	return NULL;
}

/* Find an unused, cached query for `sql'. Caller must already be holding lock */
PRIVATE WUNUSED NONNULL((1, 2)) Query *DCALL
db_find_unused_query(DB *__restrict self, DeeStringObject *__restrict sql) {
	size_t list_index;
	size_t hash_index = DB_QUERYCACHE_HASHOF(sql);
	struct query_cache_list *list;
	list = self->db_querycache[hash_index];
	ASSERT(list);
	list_index = query_cache_list_indexof(list, sql);
	if (list_index < list->qcl_count) {
		/* We got a cache hit! -> check if this hit is marked as unused */
		return db_find_unused_query_near(list, sql, list_index);
	}
	/* No (unused) cache available */
	return NULL;
}


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
INTERN WUNUSED NONNULL((1, 2)) DREF Query *DCALL
DB_NewQuery(DB *__restrict self, DeeStringObject *__restrict sql,
            size_t *p_utf8_offset_of_next_stmt) {
	Query *result;
	Query *existing_result;
	size_t list_index;
	size_t hash_index;
	struct query_cache_list *list;
	DB_QueryCache_LockRead(self);

	/* Check if we have a cached, unused query for `sql' */
	result = db_find_unused_query(self, sql);
	if (result) {
		ASSERT(Query_IsUnused(result));
		ASSERT(!Query_InUse(result));
		if (p_utf8_offset_of_next_stmt) {
			*p_utf8_offset_of_next_stmt = result->q_sql_utf8_nextstmt_offset;
		} else if (result->q_sql_utf8_nextstmt_offset) {
			DB_QueryCache_LockEndRead(self);
			err_multiple_statements(sql);
			goto err;
		}
		DB_querycache_unused_remove(self, result);
		ASSERT(result->q_db == self);
		ASSERT(result->q_sql == sql);
		Query_InitCommon(result);
		ASSERT(!Query_IsUnused(result));
		DB_QueryCache_LockEndRead(self);
		atomic_write(&result->ob_refcnt, 1); /* Mask as in-use */
		ASSERT(Query_InUse(result));

		/* In order to reset the query, we need a lock to the DB */
		if unlikely(DB_Lock(self)) {
			Dee_DecrefDokill(result);
			goto err;
		}
		(void)sqlite3_reset(result->q_stmt);
		(void)sqlite3_clear_bindings(result->q_stmt);
		DB_Unlock(self);

		return result;
	}
	DB_QueryCache_LockEndRead(self);

	/* No cache available -> must construct a new query. */
	result = Query_Alloc();
	if unlikely(!result)
		goto err;

	/* Make sure that fini hooks are enabled for "sql" */
	if unlikely(DeeString_EnableFiniHook((DeeObject *)sql))
		goto err_r;

	/* Compile (prepare) SQL code */
	{
		int rc;
		char const *sql_utf8, *sql_tail = NULL;
		sql_utf8 = DeeString_AsUtf8((DeeObject *)sql);
		if unlikely(!sql_utf8)
			goto err_r;
again_prepare:
		if unlikely(DB_Lock(self))
			goto err_r;
		rc = sqlite3_prepare_v3(self->db_db, sql_utf8,
		                        (int)WSTR_LENGTH(sql_utf8),
		                        SQLITE_PREPARE_PERSISTENT, /* Hint PERSISTENT because we cache the query */
		                        &result->q_stmt, &sql_tail );
		if unlikely(rc != SQLITE_OK) {
			rc = err_sql_throwerror(rc, ERR_SQL_THROWERROR_F_UNLOCK_DB, self, sql);
			if (rc == 0)
				goto again_prepare;
			goto err_r;
		}
		DB_Unlock(self);
		result->q_sql_utf8_nextstmt_offset = 0;
		if (sql_tail) {
			sql_tail = Dee_unicode_skipspaceutf8(sql_tail);
			if (*sql_tail && sql_tail > sql_utf8)
				result->q_sql_utf8_nextstmt_offset = (size_t)(sql_tail - sql_utf8);
		}
		if (p_utf8_offset_of_next_stmt) {
			*p_utf8_offset_of_next_stmt = result->q_sql_utf8_nextstmt_offset;
		} else if (result->q_sql_utf8_nextstmt_offset) {
			err_multiple_statements(sql);
			goto err_r_sql;
		}

		/* When source SQL is just one big comment, "q_stmt" is set to "NULL" */
		if (!result->q_stmt) {
			DeeObject_FREE(result);
			return DB_NEWQUERY_NOQUERY;
		}
	}

	/* Fill in remaining fields of new query. */
	Query_InitOnce(result, self, sql);

	/* Add new query to query cache */
	DB_QueryCache_LockWrite(self);
	hash_index = DB_QUERYCACHE_HASHOF(sql);
	list = self->db_querycache[hash_index];
	ASSERT(list);
	list_index = query_cache_list_indexof(list, sql);
	if unlikely(list_index < list->qcl_count) {
		/* We got a cache hit! -> check if this hit is marked as unused */
		existing_result = db_find_unused_query_near(list, sql, list_index);
		if unlikely(existing_result) {
use_existing_result:
			ASSERT(Query_IsUnused(existing_result));
			ASSERT(!Query_InUse(existing_result));
			if (p_utf8_offset_of_next_stmt) {
				*p_utf8_offset_of_next_stmt = existing_result->q_sql_utf8_nextstmt_offset;
			} else if (existing_result->q_sql_utf8_nextstmt_offset) {
				DB_QueryCache_LockEndRead(self);
				err_multiple_statements(sql);
				goto err;
			}
			DB_querycache_unused_remove(self, existing_result);
			ASSERT(existing_result->q_db == self);
			ASSERT(existing_result->q_sql == sql);
			Query_InitCommon(existing_result);
			ASSERT(!Query_IsUnused(existing_result));
			DB_QueryCache_LockEndRead(self);
			atomic_write(&existing_result->ob_refcnt, 1); /* Mask as in-use */
			ASSERT(Query_InUse(existing_result));
			Dee_DecrefNokill(&Query_Type); /* result->ob_type */
			Dee_DecrefNokill(self);        /* result->q_db */
			Dee_DecrefNokill(sql);         /* result->q_sql */
			DB_FinalizeStmt(self, result->q_stmt);
			DeeObject_FREE(result);
			return existing_result;
		}
	}

	/* Insert "result" into "list" at "list_index" */
	{
		size_t cur_alloc = query_cache_list_getalloc(list);
		size_t min_alloc = list->qcl_count + 1;
		if (min_alloc > cur_alloc) {
			list = query_cache_list_tryrealloc(list, min_alloc);
			if unlikely(!list) {
				struct query_cache_list *new_list;
				/* Must try-hard allocate a new list */
				DB_QueryCache_LockEndWrite(self);
				new_list = query_cache_list_alloc(min_alloc);
				if unlikely(!new_list)
					goto err_r_sql_misc;
				DB_QueryCache_LockWrite(self);

				list = self->db_querycache[hash_index];
				ASSERT(list);
				list_index = query_cache_list_indexof(list, sql);
				if unlikely(list_index < list->qcl_count) {
					/* We got a cache hit! -> check if this hit is marked as unused */
					existing_result = db_find_unused_query_near(list, sql, list_index);
					if unlikely(existing_result) {
						_query_cache_list_free(new_list);
						goto use_existing_result;
					}
				}
				cur_alloc = query_cache_list_getalloc(list);
				if (min_alloc < cur_alloc) {
					_query_cache_list_free(new_list);
					goto use_list;
				}
				new_list = (struct query_cache_list *)memcpy(new_list, list,
				                                             query_cache_list_sizeof(list->qcl_count));
				query_cache_list_free(list);
				list = new_list;
			}
			query_cache_list_setalloc(list, min_alloc);
			self->db_querycache[hash_index] = list;
		}
use_list:
		query_cache_list_insert(list, list_index, result);
		++self->db_querycache_size;
	}

	/* Release query cache lock */
	DB_QueryCache_LockEndWrite(self);

	/* Done! */
	return result;
err_r_sql_misc:
	Dee_DecrefNokill(&Query_Type); /* result->ob_type */
	Dee_DecrefNokill(self);        /* result->q_db */
	Dee_DecrefNokill(sql);         /* result->q_sql */
err_r_sql:
	DB_FinalizeStmt(self, result->q_stmt);
err_r:
	DeeObject_FREE(result);
err:
	return NULL;
}



PRIVATE NONNULL((1, 2)) void DCALL
db_string_onfini(DB *__restrict self, DeeStringObject const *__restrict sql) {
	Query *dead_query;
	size_t qc_index, list_index = DB_QUERYCACHE_HASHOF(sql);
	struct query_cache_list *qc_list;
	bool sql_has_more_queries;
	DB_QueryCache_LockRead(self);
	qc_list = self->db_querycache[list_index];
	ASSERT(qc_list);
	qc_index = query_cache_list_indexof(qc_list, sql);
	if (qc_index >= qc_list->qcl_count) {
		DB_QueryCache_LockEndRead(self);
		return;
	}
	if (!DB_QueryCache_LockTryUpgrade(self)) {
		DB_QueryCache_LockEndRead(self);
again_acquire_write_lock:
		DB_QueryCache_LockWrite(self);
		qc_list = self->db_querycache[list_index];
		ASSERT(qc_list);
		qc_index = query_cache_list_indexof(qc_list, sql);
		if unlikely(qc_index >= qc_list->qcl_count) {
			DB_QueryCache_LockEndWrite(self);
			return;
		}
	}

	/* Check if there are more queries using the same `sql' */
#define qc_list_min_index 0
#define qc_list_max_index (qc_list->qcl_count - 1)
	sql_has_more_queries = (qc_index > qc_list_min_index && qc_list->qcl_queries[qc_index - 1]->q_sql == sql) ||
	                       (qc_index < qc_list_max_index && qc_list->qcl_queries[qc_index + 1]->q_sql == sql);
#undef qc_list_min_index
#undef qc_list_max_index

	/* Remove query from cache and list of unused queries */
	dead_query = qc_list->qcl_queries[qc_index];
	ASSERT(!Query_InUse(dead_query));

	/* Only remove from unused list if bound -- the query may not
	 * actually be unbound if its destruction happened to coincide
	 * with the destruction of the associated SQL source string. */
	if (TAILQ_ISBOUND(dead_query, q_unused))
		DB_querycache_unused_remove(self, dead_query);

	/* Remove query from cache list. */
	query_cache_list_remove(qc_list, qc_index);
	--self->db_querycache_size;
	if unlikely(qc_list->qcl_count == 0) {
		/* Last query removed from list -> replace with empty-list-pointer */
		self->db_querycache[list_index] = query_cache_empty_list_PTR;
		DB_QueryCache_LockEndWrite(self);
		_query_cache_list_free(qc_list);
	} else {
		DB_QueryCache_LockEndWrite(self);
	}

	/* Free query */
	Query_Free(dead_query, self);

	/* If there are more queries, then we must remove those, too. */
	if (sql_has_more_queries)
		goto again_acquire_write_lock;
}

PRIVATE NONNULL((1, 2)) void DCALL
db_thread_onwake(DB *__restrict self, DeeThreadObject *__restrict thread) {
	if (DB_IsInterruptible(self, thread))
		sqlite3_interrupt(self->db_db);
}


PRIVATE NONNULL((1)) void DCALL
db_string_fini_hook_destroy(struct Dee_string_fini_hook *__restrict self) {
	struct db_string_fini_hook *me = db_string_fini_hook_fromhooks(self);
	Dee_weakref_fini(&me->dsfh_db);
	db_string_fini_hook_free(me);
}

PRIVATE NONNULL((1, 2)) void DCALL
db_string_fini_hook_onfini(struct Dee_string_fini_hook *__restrict self,
                           DeeStringObject const *__restrict string) {
	struct db_string_fini_hook *me = db_string_fini_hook_fromhooks(self);
	DREF DB *db = db_string_fini_hook_getdb(me);
	if likely(db) {
		db_string_onfini(db, string);
		Dee_Decref_unlikely(db);
	}
}

PRIVATE NONNULL((1)) void DCALL
db_thread_interrupt_hook_destroy(struct Dee_thread_interrupt_hook *__restrict self) {
	struct db_thread_interrupt_hook *me = db_thread_interrupt_hook_fromhooks(self);
	Dee_weakref_fini(&me->dtih_db);
	db_thread_interrupt_hook_free(me);
}

PRIVATE NONNULL((1, 2)) void DCALL
db_thread_interrupt_hook_onwake(struct Dee_thread_interrupt_hook *__restrict self,
                                DeeThreadObject *__restrict thread) {
	struct db_thread_interrupt_hook *me = db_thread_interrupt_hook_fromhooks(self);
	DREF DB *db = db_thread_interrupt_hook_getdb(me);
	if likely(db) {
		db_thread_onwake(db, thread);
		Dee_Decref_unlikely(db);
	}
}


PRIVATE int db_sqlite_progress_handler(void *UNUSED(ignored)) {
	/* >> If the progress callback returns non-zero, the operation is interrupted */
	DeeThreadObject *thread = DeeThread_Self();
	return DeeThread_WasInterrupted(thread);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
db_init(DB *__restrict self, size_t argc, DeeObject *const *argv) {
	int rc;
	DeeObject *filename;
	char const *utf8_filename;
	DREF struct db_string_fini_hook *sf_hook;
	DREF struct db_thread_interrupt_hook *ti_hook;
	DeeArg_Unpack1(err, argc, argv, "DB", &filename);
	if (DeeObject_AssertTypeExact(filename, &DeeString_Type))
		goto err;
	utf8_filename = DeeString_AsUtf8(filename);
	if unlikely(!utf8_filename)
		goto err;
	sf_hook = db_string_fini_hook_alloc();
	if unlikely(!sf_hook)
		goto err;
	ti_hook = db_thread_interrupt_hook_alloc();
	if unlikely(!ti_hook)
		goto err_sf_hook;
	if unlikely(libsqlite3_init())
		goto err_sf_hook_ti_hook;

	/* TODO: Allow user-code to specify/config "flags" as:
	 * one of (these are the only combinations allowed by sqlite):
	 * - SQLITE_OPEN_READONLY
	 * - SQLITE_OPEN_READWRITE
	 * - SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE
	 * - SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_DELETEONCLOSE
	 *
	 * Optionally or'd with (each of these is its own bool-keyword-argument):
	 * - uri=!f       -- SQLITE_OPEN_URI
	 * - nofollow=!f  -- SQLITE_OPEN_NOFOLLOW
	 *
	 * -- TODO: SQLITE_OPEN_MAIN_DB
	 * -- TODO: SQLITE_OPEN_TEMP_DB
	 * -- TODO: SQLITE_OPEN_TRANSIENT_DB
	 * -- TODO: SQLITE_OPEN_MAIN_JOURNAL
	 * -- TODO: SQLITE_OPEN_TEMP_JOURNAL
	 * -- TODO: SQLITE_OPEN_SUBJOURNAL
	 * -- TODO: SQLITE_OPEN_SUPER_JOURNAL
	 * -- TODO: SQLITE_OPEN_WAL
	 * -- TODO: SQLITE_OPEN_AUTOPROXY
	 * -- TODO: SQLITE_OPEN_MASTER_JOURNAL
	 *
	 * The following flags cannot be controlled:
	 * - SQLITE_OPEN_MEMORY       (never set; implicitly available by using ":memory:" as filename)
	 * - SQLITE_OPEN_NOMUTEX      (never set; deemon does its own (enforced) mutex handling)
	 * - SQLITE_OPEN_FULLMUTEX    (never set; deemon does its own (enforced) mutex handling)
	 * - SQLITE_OPEN_SHAREDCACHE  (never set; shared cache is disabled as recommended by sqlite docs)
	 * - SQLITE_OPEN_PRIVATECACHE (never set; shared cache is disabled as recommended by sqlite docs)
	 * - SQLITE_OPEN_EXRESCODE    (always set; error handling is done internally, so user can't control this)
	 * - SQLITE_OPEN_EXCLUSIVE    (never set; docs warn that it doesn't do O_EXCL-behavior, so it's useless)
	 */
	/* TODO: Allow user-code to specify "zVfs" */
again_open:
	rc = sqlite3_open_v2(utf8_filename, &self->db_db,
	                     SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE |
	                     SQLITE_OPEN_EXRESCODE,
	                     NULL);
	if (rc != SQLITE_OK) {
		rc = err_sql_throwerror(rc, ERR_SQL_THROWERROR_F_NORMAL, self, NULL);
		(void)sqlite3_close_v2(self->db_db);
		if (rc == 0)
			goto again_open;
		goto err_sf_hook_ti_hook_lib;
	}

	/* Fill in remaining fields... */
	memsetp(self->db_querycache, query_cache_empty_list_PTR, DB_QUERYCACHE_SIZE);
	TAILQ_INIT(&self->db_querycache_unused);
	self->db_querycache_size = 0;
	self->db_querycache_unused_count = 0;
	self->db_querycache_unused_limit = DEFAULT_DB_QUERYCACHE_UNUSED_LIMIT; /* TODO: Allow user-code to configure (using keyword arguments) */
	Dee_atomic_rwlock_init(&self->db_querycache_lock);
	self->db_sf_hook = sf_hook;
	self->db_ti_hook = ti_hook;
	Dee_weakref_support_init(self);
	Dee_shared_lock_init(&self->db_dblock);
	self->db_thread = NULL;
	self->db_freelist = NULL;

	/* Initialize string finalization hooks. */
	sf_hook->dsfh_hook.sfh_refcnt  = 1;
	sf_hook->dsfh_hook.sfh_destroy = &db_string_fini_hook_destroy;
	sf_hook->dsfh_hook.sfh_onfini  = &db_string_fini_hook_onfini;
	Dee_weakref_init(&sf_hook->dsfh_db, Dee_AsObject(self), NULL);

	/* Initialize thread interrupt hooks. */
	ti_hook->dtih_hook.tih_refcnt  = 1;
	ti_hook->dtih_hook.tih_destroy = &db_thread_interrupt_hook_destroy;
	ti_hook->dtih_hook.tih_onwake  = &db_thread_interrupt_hook_onwake;
	Dee_weakref_init(&ti_hook->dtih_db, Dee_AsObject(self), NULL);

	/* Register string finalization hooks. */
	if unlikely(DeeString_AddFiniHook(&self->db_sf_hook->dsfh_hook))
		goto err_sf_hook_ti_hook_lib_self;

	/* Register string finalization hooks. */
	if unlikely(DeeThread_AddInterruptHook(&self->db_ti_hook->dtih_hook))
		goto err_sf_hook_ti_hook_lib_self_sf_hook;

	/* Because of how SQLite interfaces with system libraries, interrupt
	 * injection may be a bit flaky when it comes to forcing long-running
	 * queries to end prematurely (reason: "xMutexEnter" returns "void",
	 * meaning there is no way of saying "mutex not entered; fail with
	 * reason 'SQLITE_INTERRUPT'").
	 *
	 * To fix that issue, also inject a "Progress handler" that checks if
	 * the calling thread has been interrupted every couple thousand of
	 * virtual opcodes, acting as a fallback to always allow queries to
	 * be canceled. */
	sqlite3_progress_handler(self->db_db, 100000, &db_sqlite_progress_handler, NULL);

	return 0;
err_sf_hook_ti_hook_lib_self_sf_hook:
	DeeString_RemoveFiniHook(&self->db_sf_hook->dsfh_hook);
err_sf_hook_ti_hook_lib_self:
	Dee_weakref_fini(&sf_hook->dsfh_db);
	Dee_weakref_support_fini(self);
	(void)sqlite3_close_v2(self->db_db);
err_sf_hook_ti_hook_lib:
	libsqlite3_fini();
err_sf_hook_ti_hook:
	db_thread_interrupt_hook_free(ti_hook);
err_sf_hook:
	db_string_fini_hook_free(sf_hook);
err:
	return -1;
}


PRIVATE NONNULL((1)) void DCALL
db_fini(DB *__restrict self) {
	size_t cache_index;

	/* Remove hooks. */
	(void)DeeThread_RemoveInterruptHook(&self->db_ti_hook->dtih_hook);
	(void)DeeString_RemoveFiniHook(&self->db_sf_hook->dsfh_hook);
	Dee_thread_interrupt_hook_decref(&self->db_ti_hook->dtih_hook);
	Dee_string_fini_hook_decref(&self->db_sf_hook->dsfh_hook);

	/* At this point, all remaining queries **MUST** be unused (since
	* used queries would hold reference to us, meaning we wouldn't be
	* getting destroyed) */
	ASSERT(self->db_querycache_size == self->db_querycache_unused_count);

	/* Destroy all still-cached, unused queries. */
	for (cache_index = 0; cache_index < COMPILER_LENOF(self->db_querycache); ++cache_index) {
		struct query_cache_list *list = self->db_querycache[cache_index];
		ASSERT((list->qcl_count == 0) == (list == query_cache_empty_list_PTR));
		if (list != query_cache_empty_list_PTR) {
			size_t list_index = 0;
			do {
				Query *query = list->qcl_queries[list_index];
				ASSERT(query);
				ASSERT(!Query_InUse(query));
				ASSERT(TAILQ_ISBOUND(query, q_unused));
				/* Directly finalize statement -> DB is going away (meaning we're the
				 * last living thread able to see it), so no need to play around with
				 * locks anymore! */
				(void)sqlite3_finalize(query->q_stmt);
				_Query_Free(query);
			} while (++list_index < list->qcl_count);
			_query_cache_list_free(list);
		}
	}

	/* Close the underlying database */
	(void)sqlite3_close_v2(self->db_db);

	/* Close library (if this is the last DB to go away) */
	libsqlite3_fini();
}

PRIVATE NONNULL((1)) bool DCALL
db_cc(DB *__restrict self) {
	bool result = false;
again:
	DB_QueryCache_LockWrite(self);
	if (!TAILQ_EMPTY(&self->db_querycache_unused)) {
		db_free_oldest_unused_query_and_unlock(self);
		result = true;
		goto again;
	}
	DB_QueryCache_LockEndWrite(self);

	/* Also try to invoke `sqlite3_db_release_memory()' */
	if (!result) {
		if (DB_TryLock(self)) {
			/* Sadly, this function doesn't return if it managed to free
			 * something, so to prevent infinite loops, never return true
			 * at this point... */
			(void)sqlite3_db_release_memory(self->db_db);
			DB_Unlock(self);
		}
#ifdef SQLITE_ENABLE_MEMORY_MANAGEMENT
		if (sqlite3_release_memory(INT_MAX) > 0)
			result = true;
#endif /* SQLITE_ENABLE_MEMORY_MANAGEMENT */
	}
	return result;
}




/* Execute `stmt' until there is no more data present.
 * @return: (uint64_t)-1: Error
 * @return: * : The # of affected rows */
PRIVATE WUNUSED NONNULL((1)) uint64_t DCALL
db_exec_stmt(DB *__restrict self, sqlite3_stmt *stmt) {
	int rc;
	uint64_t changes = 0;
	uint64_t changes_at_start;
again:
	if unlikely(DB_Lock(self))
		goto err;
	changes_at_start = (uint64_t)sqlite3_changes64(self->db_db);
	do {
		rc = sqlite3_step(stmt);
	} while (rc == SQLITE_ROW);
	changes += (uint64_t)sqlite3_changes64(self->db_db) - changes_at_start;
	if (rc != SQLITE_DONE && rc != SQLITE_OK) {
		rc = err_sql_throwerror(rc, ERR_SQL_THROWERROR_F_UNLOCK_DB, self, NULL);
		if (rc == 0)
			goto again;
		goto err;
	}
	DB_Unlock(self);
	return changes;
err:
	return (uint64_t)-1;
}

PRIVATE NONNULL((1)) DREF Query *DCALL
db_query(DB *__restrict self, size_t argc, DeeObject *const *argv) {
	DREF Query *result;
	DeeObject *sql, *params = Dee_EmptyTuple;
	DeeArg_Unpack1Or2(err, argc, argv, "query", &sql, &params);
	if (DeeObject_AssertTypeExact(sql, &DeeString_Type))
		goto err;
	result = DB_NewQuery(self, (DeeStringObject *)sql, NULL);
	if unlikely(!result)
		goto err;
	if unlikely(result == DB_NEWQUERY_NOQUERY) {
		DeeError_Throwf(&DeeError_SyntaxError,
		                "Query is empty of contains only comments: %r",
		                sql);
		goto err;
	}

	/* Bind query "params". Because the query isn't being shared,
	 * we don't even have to lock it, yet! */
	ASSERT(!DeeObject_IsShared(result));
	if unlikely(dee_sqlite3_bind_params(self, result->q_stmt, params, 0) < 0)
		goto err_r;

	/* Return the fully initialized query. */
	return result;
err_r:
	Dee_DecrefDokill(result);
err:
	return NULL;
}

PRIVATE NONNULL((1)) DREF DeeObject *DCALL
db_exec(DB *__restrict self, size_t argc, DeeObject *const *argv) {
	DREF Query *query0;
	DeeStringObject *sql;
	DeeObject *params = Dee_EmptyTuple;
	char const *utf8_sql, *utf8_sql_orig;
	size_t offset_of_next_query;
	Dee_ssize_t bind_status;
	int rc;
	uint64_t changes;
	DeeArg_Unpack1Or2(err, argc, argv, "exec", &sql, &params);
	if (DeeObject_AssertTypeExact(sql, &DeeString_Type))
		goto err;
	utf8_sql = DeeString_AsUtf8((DeeObject *)sql);
	if unlikely(!utf8_sql)
		goto err;
	utf8_sql_orig = utf8_sql;
	query0 = DB_NewQuery(self, sql, &offset_of_next_query);
	if unlikely(!query0)
		goto err;
	if likely(query0 != DB_NEWQUERY_NOQUERY) {
		ASSERT(!DeeObject_IsShared(query0));
		bind_status = dee_sqlite3_bind_params(self, query0->q_stmt, params, 0);
		if unlikely(bind_status < 0)
			goto err_query0;
		changes = db_exec_stmt(self, query0->q_stmt);
		if unlikely(changes == (uint64_t)-1)
			goto err_query0;
		Dee_DecrefDokill(query0);
	} else {
		bind_status = 0;
		changes     = 0;
	}
	if (offset_of_next_query) {
		/* There are more statements that need executing! */
		sqlite3_stmt *stmt;
		size_t utf8_sql_length;
		char const *sql_tail;
		uint64_t part_changes;
		size_t params_offset;

		utf8_sql_length = WSTR_LENGTH(utf8_sql);
		utf8_sql += offset_of_next_query;
		utf8_sql_length -= offset_of_next_query;
		params_offset = (size_t)bind_status;

		/* Prepare next statement */
again_prepare:
		if unlikely(DB_Lock(self))
			goto err;
		sql_tail = NULL;
		rc = sqlite3_prepare_v2(self->db_db, utf8_sql,
		                        (int)utf8_sql_length,
		                        &stmt, &sql_tail);
		if unlikely(rc != SQLITE_OK) {
			if (utf8_sql == utf8_sql_orig) {
				rc = err_sql_throwerror(rc, ERR_SQL_THROWERROR_F_UNLOCK_DB, self, sql);
			} else {
				DREF DeeStringObject *sql_rem;
				sql_rem = (DREF DeeStringObject *)DeeString_TryNewUtf8(utf8_sql, utf8_sql_length,
				                                                       STRING_ERROR_FIGNORE);
				if unlikely(!sql_rem) {
					DB_Unlock(self);
					if (Dee_CollectMemory(utf8_sql_length))
						goto again_prepare;
					goto err;
				}
				rc = err_sql_throwerror(rc, ERR_SQL_THROWERROR_F_UNLOCK_DB, self, sql_rem);
				Dee_Decref_unlikely(sql_rem);
			}
			if (rc == 0)
				goto again_prepare;
			goto err;
		}
		DB_Unlock(self);

		/* When source SQL is just one big comment, "stmt" is set to "NULL" */
		if likely(stmt != NULL) {
			/* Bind parameters */
			bind_status = dee_sqlite3_bind_params(self, stmt, params, params_offset);
			if unlikely(bind_status < 0) {
				DB_FinalizeStmt(self, stmt);
				goto err;
			}
			params_offset += (size_t)bind_status;    /* Account for offset caused by "?"-parameters in relation to future statements. */
			part_changes = db_exec_stmt(self, stmt); /* Execute statement */
			DB_FinalizeStmt(self, stmt);             /* Destroy the statement */
			if unlikely(part_changes == (uint64_t)-1)
				goto err;  /* Check for errors that may have happened during execution */
			changes += part_changes;
		}

		/* Check if there are more queries that need executing */
		if (sql_tail != NULL) {
			sql_tail = Dee_unicode_skipspaceutf8(sql_tail);
			if (*sql_tail) {
				utf8_sql_length -= (size_t)(sql_tail - utf8_sql);
				utf8_sql = sql_tail;
				goto again_prepare;
			}
		}
	}
	return DeeInt_NewUInt64(changes);
err_query0:
	Dee_DecrefDokill(query0);
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
db_print(DB *__restrict self, Dee_formatprinter_t printer, void *arg) {
	(void)self; /* TODO: Print some extra info here */
	return DeeFormat_PRINT(printer, arg, "<SQLite3 Database>");
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
db_printrepr(DB *__restrict self, Dee_formatprinter_t printer, void *arg) {
	(void)self; /* TODO: Print some extra info here */
	return DeeFormat_PRINT(printer, arg, "DB(TODO)");
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
db_get_last_insert_rowid(DB *__restrict self) {
	sqlite3_int64 result;
	if (DB_Lock(self))
		goto err;
	result = sqlite3_last_insert_rowid(self->db_db);
	DB_Unlock(self);
	return DeeInt_NewUInt64((uint64_t)result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
db_set_last_insert_rowid_f(DB *self, uint64_t rowid) {
	int result = DB_Lock(self);
	if likely(result == 0) {
		sqlite3_set_last_insert_rowid(self->db_db, (sqlite3_int64)rowid);
		DB_Unlock(self);
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
db_set_last_insert_rowid(DB *self, DeeObject *value) {
	uint64_t rowid;
	int result = DeeObject_AsUInt64(value, &rowid);
	if likely(result == 0)
		result = db_set_last_insert_rowid_f(self, rowid);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
db_get_qc_cur_unused(DB *__restrict self) {
	size_t result;
	DB_QueryCache_LockRead(self);
	result = self->db_querycache_unused_count;
	DB_QueryCache_LockEndRead(self);
	return DeeInt_NewSize(result);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
db_get_qc_max_unused(DB *__restrict self) {
	size_t result;
	DB_QueryCache_LockRead(self);
	result = self->db_querycache_unused_limit;
	DB_QueryCache_LockEndRead(self);
	return DeeInt_NewSize(result);
}

PRIVATE NONNULL((1)) void DCALL
db_set_qc_max_unused_v(DB *__restrict self, size_t limit) {
	DB_QueryCache_LockWrite(self);
	self->db_querycache_unused_limit = limit;
	while (self->db_querycache_unused_count > self->db_querycache_unused_limit) {
		bool last = (self->db_querycache_unused_count - 1) <=
		            (self->db_querycache_unused_limit);
		db_free_oldest_unused_query_and_unlock(self);
		if (last)
			return;
		DB_QueryCache_LockWrite(self);
	}
	DB_QueryCache_LockEndWrite(self);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
db_del_qc_max_unused(DB *__restrict self) {
	db_set_qc_max_unused_v(self, DEFAULT_DB_QUERYCACHE_UNUSED_LIMIT);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
db_set_qc_max_unused(DB *self, DeeObject *value) {
	size_t limit;
	int result = DeeObject_AsSize(value, &limit);
	if likely(result == 0)
		db_set_qc_max_unused_v(self, limit);
	return result;
}


PRIVATE struct type_method tpconst db_methods[] = {
	TYPE_METHOD("exec", &db_exec, "(sql:?Dstring,params:?X2?M?Dstring" T_SQL_OBJECT "?S" T_SQL_OBJECT "=!T0)"),
	TYPE_METHOD("query", &db_query, "(sql:?Dstring,params:?X2?M?Dstring" T_SQL_OBJECT "?S" T_SQL_OBJECT "=!T0)->?GQuery"),
	/* TODO: Expose sqlite3_blob_open() */
	/* TODO: Expose sqlite3_db_cacheflush() */
	/* TODO: Expose sqlite3_db_config() */
	/* TODO: Expose sqlite3_db_filename() */
	/* TODO: Expose sqlite3_db_name()  (maybe also expose as a custom proxy object that allows easy enumeration of attached DBs) */
	/* TODO: Expose sqlite3_db_readonly() */
	/* TODO: Expose sqlite3_db_status() */
	/* TODO: Expose sqlite3_file_control() */
	/* TODO: Expose sqlite3_get_autocommit() */
	/* TODO: Expose sqlite3_get_clientdata() + sqlite3_set_clientdata() (these should be usable to encapsulate a single "DeeObject" linked to every database) */
	/* TODO: Directly expose sqlite3_interrupt() (already gets called during `DeeThread_Wake()') */
	/* TODO: Directly expose sqlite3_is_interrupted() */
	/* TODO: Expose sqlite3_limit() */
	/* TODO: Expose sqlite3_table_column_metadata() */
	/* TODO: Expose sqlite3_txn_state() */
	TYPE_METHOD_END
};

PRIVATE struct type_getset tpconst db_getsets[] = {
	/* TODO: schemas->?GSchemas  -- {string: Schema}  (using "sqlite3_db_name()")
	 * - "Schema" (value type of "Schemas") should then have another getset
	 *   - "tables->?GTables", which is another {string: Table}
	 * - "Table" (value type of "Tables") should then have another getset
	 *   "rows->?GRows", which is another {int: Row} (where the "int" is the
	 *   "_ROWID_" of rows) (XXX: this doesn't work for "WITHOUT ROWID" tables)
	 */

	TYPE_GETSET_AB("last_insert_rowid",
	               &db_get_last_insert_rowid, NULL,
	               &db_set_last_insert_rowid,
	               "->?Dint\n"
	               "Interface to get/set #Csqlite3_last_insert_rowid and #Csqlite3_set_last_insert_rowid"),
	TYPE_GETTER_AB("qc_cur_unused", &db_get_qc_cur_unused,
	               "->?Dint\n"
	               "Return the number of unused (unreferenced) queries with "
	               "not-yet-destroyed template SQL strings that are being cached"),
	TYPE_GETSET_AB("qc_max_unused",
	               &db_get_qc_max_unused,
	               &db_del_qc_max_unused,
	               &db_set_qc_max_unused,
	               "->?Dint\n"
	               "Get/set the max number of unused (unreferenced) queries "
	               /**/ "with not-yet-destroyed template SQL strings to keep cached\n"
	               "Deleting this attribute restores the default "
	               /**/ "$" PP_STR(DEFAULT_DB_QUERYCACHE_UNUSED_LIMIT)),
	/* TODO: Expose cached queries (in the form of a `{string...}'-Sequence) */
	TYPE_GETSET_END
};

PRIVATE struct type_gc tpconst db_gc = {
	/* .tp_clear  = */ NULL,
	/* .tp_pclear = */ NULL,
	/* .tp_gcprio = */ 0,
	/* .tp_cc     = */ (bool (DCALL *)(DeeObject *__restrict))&db_cc,
};


INTERN DeeTypeObject DB_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "DB",
	/* .tp_doc      = */ DOC("(filename:?Dstring)"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ Dee_WEAKREF_SUPPORT_ADDR(DB),
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DB,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ NULL,
			/* tp_deep_ctor:   */ NULL,
			/* tp_any_ctor:    */ &db_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL /* Can't be serialized */
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&db_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ NULL,
		/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&db_print,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&db_printrepr,
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ &db_gc, /* Only here for "tp_cc" -- clears the query cache */
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ db_methods,
	/* .tp_getsets       = */ db_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

DECL_END

#endif /* !GUARD_DEX_SQLITE3_DB_C */
