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
#ifndef GUARD_DEX_SQLITE3_DB_C
#define GUARD_DEX_SQLITE3_DB_C 1
#define CONFIG_BUILDING_LIBSQLITE3
#define DEE_SOURCE

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/bytes.h>
#include <deemon/dex.h>
#include <deemon/error.h>
#include <deemon/float.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/method-hints.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/stringutils.h>
#include <deemon/super.h>
#include <deemon/system-features.h>
#include <deemon/thread.h>
#include <deemon/tuple.h>
#include <deemon/util/atomic.h>
#include <deemon/util/lock.h>
#include <deemon/util/rlock.h>

#include <hybrid/align.h>

/**/
#include "sqlite3-external.h"
#include "libsqlite3.h"

DECL_BEGIN

INTERN_CONST struct query_cache_empty_list_struct const
query_cache_empty_list_ = {
	0
#ifndef Dee_MallocUsableSizeNonNull
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


INTERN WUNUSED NONNULL((1)) int DCALL
DB_Lock(DB *__restrict self) {
	DeeThreadObject *me = DeeThread_Self();
again:
	sqlite3_mutex_enter(sqlite3_db_mutex(self->db_db));
#ifdef CONFIG_NO_THREADS
	self->db_interruptible = true;
#else /* CONFIG_NO_THREADS */
	atomic_write(&self->db_thread, me);
#endif /* !CONFIG_NO_THREADS */
	DeeThread_EnableInterruptHooks(me);
	if unlikely(DeeThread_WasInterrupted(me)) {
		DeeThread_DisableInterruptHooks(me);
#ifdef CONFIG_NO_THREADS
		self->db_interruptible = false;
#else /* CONFIG_NO_THREADS */
		atomic_write(&self->db_thread, NULL);
		sqlite3_mutex_leave(sqlite3_db_mutex(self->db_db));
#endif /* !CONFIG_NO_THREADS */
		if (DeeThread_CheckInterruptSelf(self))
			goto err;
		goto again;
	}
	return 0;
err:
	return -1;
}

INTERN NONNULL((1)) void DCALL
DB_Unlock(DB *__restrict self) {
#ifdef CONFIG_NO_THREADS
	DeeThreadObject *me = DeeThread_Self();
	DeeThread_DisableInterruptHooks(me);
	self->db_interruptible = false;
#else /* CONFIG_NO_THREADS */
	DeeThread_DisableInterruptHooks(self->db_thread);
	atomic_write(&self->db_thread, NULL);
#endif /* !CONFIG_NO_THREADS */
	sqlite3_mutex_leave(sqlite3_db_mutex(self->db_db));
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
	ASSERT(qc_list != &query_cache_empty_list);
	query_cache_list_remove(qc_list, qc_index);
	--self->db_querycache_size;
	TAILQ_REMOVE_HEAD(&self->db_querycache_unused, q_unused);
	--self->db_querycache_unused_count;
	if (qc_list->qcl_count == 0) {
		/* Last query removed from list -> replace with empty-list-pointer */
		self->db_querycache[list_index] = &query_cache_empty_list;
		DB_QueryCache_LockEndWrite(self);
		_query_cache_list_free(qc_list);
	} else {
		DB_QueryCache_LockEndWrite(self);
	}

	/* Free "oldest_query" */
	Query_Free(oldest_query);
}


/************************************************************************/
/************************************************************************/
/*                                                                      */
/* DB                                                                   */
/*                                                                      */
/************************************************************************/
/************************************************************************/

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
		return result;
	}
	DB_QueryCache_LockEndRead(self);

	/* No cache available -> must construct a new query. */
	result = Query_Alloc();
	if unlikely(!result)
		goto err;

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
			rc = err_sql_throwerror_and_maybe_unlock(self, sql_utf8, rc, NULL,
			                                         ERR_SQL_THROWERROR_F_ALLOW_RESTART |
			                                         ERR_SQL_THROWERROR_F_UNLOCK_DB);
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
			(void)sqlite3_finalize(result->q_stmt);
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
	(void)sqlite3_finalize(result->q_stmt);
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
		self->db_querycache[list_index] = &query_cache_empty_list;
		DB_QueryCache_LockEndWrite(self);
		_query_cache_list_free(qc_list);
	} else {
		DB_QueryCache_LockEndWrite(self);
	}

	/* Free query */
	Query_Free(dead_query);

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


PRIVATE WUNUSED NONNULL((1)) int DCALL
db_init(DB *__restrict self, size_t argc, DeeObject *const *argv) {
	int rc;
	DeeObject *filename;
	char const *utf8_filename;
	DREF struct db_string_fini_hook *sf_hook;
	DREF struct db_thread_interrupt_hook *ti_hook;
	_DeeArg_Unpack1(err, argc, argv, "DB", &filename);
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

	/* TODO: Allow user-code to specify "flags" */
	/* TODO: Allow user-code to specify "zVfs" */
again_open:
	rc = sqlite3_open_v2(utf8_filename, &self->db_db,
	                     SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE |
	                     SQLITE_OPEN_EXRESCODE,
	                     NULL);
	if (rc != SQLITE_OK) {
		rc = err_sql_throwerror_and_maybe_unlock(self, NULL, rc, NULL,
		                                         ERR_SQL_THROWERROR_F_ALLOW_RESTART);
		(void)sqlite3_close_v2(self->db_db);
		if (rc == 0)
			goto again_open;
		goto err_sf_hook_ti_hook;
	}

	/* Fill in remaining fields... */
	memsetp(self->db_querycache, &query_cache_empty_list, DB_QUERYCACHE_SIZE);
	TAILQ_INIT(&self->db_querycache_unused);
	self->db_querycache_size = 0;
	self->db_querycache_unused_count = 0;
	self->db_querycache_unused_limit = DEFAULT_DB_QUERYCACHE_UNUSED_LIMIT; /* TODO: Allow user-code to configure (using keyword arguments) */
	Dee_atomic_rwlock_init(&self->db_querycache_lock);
	self->db_sf_hook = sf_hook;
	self->db_ti_hook = ti_hook;
	weakref_support_init(self);

	/* Initialize string finalization hooks. */
	sf_hook->dsfh_hook.sfh_refcnt  = 1;
	sf_hook->dsfh_hook.sfh_destroy = &db_string_fini_hook_destroy;
	sf_hook->dsfh_hook.sfh_onfini  = &db_string_fini_hook_onfini;
	Dee_weakref_init(&sf_hook->dsfh_db, (DeeObject *)self, NULL);

	/* Initialize thread interrupt hooks. */
	ti_hook->dtih_hook.tih_refcnt  = 1;
	ti_hook->dtih_hook.tih_destroy = &db_thread_interrupt_hook_destroy;
	ti_hook->dtih_hook.tih_onwake  = &db_thread_interrupt_hook_onwake;
	Dee_weakref_init(&ti_hook->dtih_db, (DeeObject *)self, NULL);

	/* Register string finalization hooks. */
	if unlikely(DeeString_AddFiniHook(&self->db_sf_hook->dsfh_hook))
		goto err_sf_hook_ti_hook_self;

	/* Register string finalization hooks. */
	if unlikely(DeeThread_AddInterruptHook(&self->db_ti_hook->dtih_hook))
		goto err_sf_hook_ti_hook_self_sf_hook;

	return 0;
err_sf_hook_ti_hook_self_sf_hook:
	DeeString_RemoveFiniHook(&self->db_sf_hook->dsfh_hook);
err_sf_hook_ti_hook_self:
	Dee_weakref_fini(&sf_hook->dsfh_db);
	weakref_support_fini(self);
	(void)sqlite3_close_v2(self->db_db);
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
		ASSERT((list->qcl_count == 0) == (list == &query_cache_empty_list));
		if (list != &query_cache_empty_list) {
			size_t list_index = 0;
			do {
				Query *query = list->qcl_queries[list_index];
				ASSERT(query);
				ASSERT(!Query_InUse(query));
				ASSERT(TAILQ_ISBOUND(query, q_unused));
				Query_Free(query);
			} while (++list_index < list->qcl_count);
			_query_cache_list_free(list);
		}
	}

	/* Close the underlying database */
	(void)sqlite3_close_v2(self->db_db);
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
	return result;
}




/* Execute `stmt' until there is no more data present.
 * @return: (uint64_t)-1: Error
 * @return: * : The # of affected rows */
INTERN WUNUSED NONNULL((1)) uint64_t DCALL
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
		rc = err_sql_throwerror_and_maybe_unlock(self, NULL, rc, NULL,
		                                         ERR_SQL_THROWERROR_F_ALLOW_RESTART |
		                                         ERR_SQL_THROWERROR_F_UNLOCK_DB);
		if (rc == 0)
			goto again;
		goto err;
	}
	DB_Unlock(self);
	return changes;
err:
	return (uint64_t)-1;
}

/* Skip at most `count' rows in `stmt', returning the actual # of skipped rows.
 * @return: (size_t)-1: Error
 * @return: * : The # of skipped rows */
INTERN WUNUSED NONNULL((1)) size_t DCALL
db_skip_stmt(DB *__restrict self, sqlite3_stmt *stmt, size_t count) {
	int rc;
	size_t result = 0;
	if unlikely(!count)
		return 0;
again:
	if unlikely(DB_Lock(self))
		goto err;
	do {
		rc = sqlite3_step(stmt);
		if (rc != SQLITE_ROW)
			break;
	} while (++result < count);
	if (rc != SQLITE_DONE && rc != SQLITE_OK) {
		rc = err_sql_throwerror_and_maybe_unlock(self, NULL, rc, NULL,
		                                         ERR_SQL_THROWERROR_F_ALLOW_RESTART |
		                                         ERR_SQL_THROWERROR_F_UNLOCK_DB);
		if (rc == 0)
			goto again;
		goto err;
	}
	DB_Unlock(self);
	return result;
err:
	return (uint64_t)-1;
}



PRIVATE NONNULL((1)) DREF Query *DCALL
db_query(DB *__restrict self, size_t argc, DeeObject *const *argv) {
	DREF Query *result;
	DeeObject *sql, *params = Dee_EmptyTuple;
	_DeeArg_Unpack1Or2(err, argc, argv, "query", &sql, &params);
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
	if unlikely(dee_sqlite3_bind_params(result->q_stmt, params, 0) < 0)
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
	DeeObject *sql, *params = Dee_EmptyTuple;
	char const *utf8_sql;
	size_t offset_of_next_query;
	Dee_ssize_t bind_status;
	int rc;
	uint64_t changes;
	_DeeArg_Unpack1Or2(err, argc, argv, "exec", &sql, &params);
	if (DeeObject_AssertTypeExact(sql, &DeeString_Type))
		goto err;
	utf8_sql = DeeString_AsUtf8(sql);
	if unlikely(!utf8_sql)
		goto err;
	query0 = DB_NewQuery(self, (DeeStringObject *)sql, &offset_of_next_query);
	if unlikely(!query0)
		goto err;
	if likely(query0 != DB_NEWQUERY_NOQUERY) {
		ASSERT(!DeeObject_IsShared(query0));
		bind_status = dee_sqlite3_bind_params(query0->q_stmt, params, 0);
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
			rc = err_sql_throwerror_and_maybe_unlock(self, utf8_sql, rc, NULL,
			                                         ERR_SQL_THROWERROR_F_ALLOW_RESTART |
			                                         ERR_SQL_THROWERROR_F_UNLOCK_DB);
			if (rc == 0)
				goto again_prepare;
			goto err;
		}
		DB_Unlock(self);

		/* When source SQL is just one big comment, "stmt" is set to "NULL" */
		if likely(stmt != NULL) {
			/* Bind parameters */
			bind_status = dee_sqlite3_bind_params(stmt, params, params_offset);
			if unlikely(bind_status < 0) {
				(void)sqlite3_finalize(stmt);
				goto err;
			}
			/* Account for offset caused by "?"-parameters in relation to future statements. */
			params_offset += (size_t)bind_status;
			/* Execute statement */
			part_changes = db_exec_stmt(self, stmt);
			/* Destroy the statement */
			(void)sqlite3_finalize(stmt);
			/* Check for errors that may have happened during execution */
			if unlikely(part_changes == (uint64_t)-1)
				goto err;
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


PRIVATE struct type_method tpconst db_methods[] = {
	TYPE_METHOD("exec", &db_exec, "(sql:?Dstring,params:?X2?M?Dstring" T_SQL_OBJECT "?S" T_SQL_OBJECT "=!T0)"),
	TYPE_METHOD("query", &db_query, "(sql:?Dstring,params:?X2?M?Dstring" T_SQL_OBJECT "?S" T_SQL_OBJECT "=!T0)->?GQuery"),
	TYPE_METHOD_END
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
	/* .tp_weakrefs = */ WEAKREF_SUPPORT_ADDR(DB),
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor        = */ (Dee_funptr_t)NULL,
				/* .tp_copy_ctor   = */ (Dee_funptr_t)NULL,
				/* .tp_deep_ctor   = */ (Dee_funptr_t)NULL,
				/* .tp_any_ctor    = */ (Dee_funptr_t)&db_init,
				TYPE_FIXED_ALLOCATOR(DB),
			}
		},
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
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

DECL_END

#endif /* !GUARD_DEX_SQLITE3_DB_C */
