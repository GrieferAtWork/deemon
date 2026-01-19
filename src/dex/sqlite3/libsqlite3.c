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
#ifndef GUARD_DEX_SQLITE3_LIBSQLITE3_C
#define GUARD_DEX_SQLITE3_LIBSQLITE3_C 1
#define CONFIG_BUILDING_LIBSQLITE3
#define DEE_SOURCE

#include "libsqlite3.h"
/**/

#include <deemon/api.h>

#include <deemon/dex.h>
#include <deemon/module.h>
#include <deemon/object.h>
#include <deemon/util/lock.h>

#include "sqlite3-external.h"

DECL_BEGIN

PRIVATE Dee_refcnt_t libsqlite3_initialized = 0;
PRIVATE Dee_shared_lock_t libsqlite3_initialized_lock = Dee_SHARED_LOCK_INIT;

PRIVATE WUNUSED int DCALL libsqlite3_init_impl(void) {
	int result;
again:
	result = sqlite3_initialize();
	if likely(result == SQLITE_OK)
		return 0;
	result = err_sql_throwerror(result, ERR_SQL_THROWERROR_F_NORMAL, NULL, NULL);
	if (result == 0)
		goto again;
	return result;
}

PRIVATE void DCALL libsqlite3_fini_impl(void) {
	(void)sqlite3_shutdown();
}

/* These functions are called when a "DB_Type" object is created/destroyed.
 * Internally, these keep a running counter such that:
 * - The first call does `sqlite3_initialize()' and throws an error if something went wrong
 * - The last call does `sqlite3_shutdown()'
 * @return: 0 : Success
 * @return: -1: An error was thrown */
INTERN WUNUSED int DCALL libsqlite3_init(void) {
	int result = Dee_shared_lock_acquire(&libsqlite3_initialized_lock);
	if likely(result == 0) {
		if (libsqlite3_initialized++ == 0)
			result = libsqlite3_init_impl();
		Dee_shared_lock_release(&libsqlite3_initialized_lock);
	}
	return result;
}

INTERN void DCALL libsqlite3_fini(void) {
	Dee_shared_lock_acquire_noint(&libsqlite3_initialized_lock);
	if (--libsqlite3_initialized == 0)
		libsqlite3_fini_impl();
	Dee_shared_lock_release(&libsqlite3_initialized_lock);
}


DEX_BEGIN
/* sqlite3 dex module:
 *
 * >> import * from sqlite3;
 * >> local db = DB("mydb.db");
 * >> // function exec(sql: string, args: {Object...} | {string: Object}): none;
 * >> db.exec(r"
 * >> CREATE TABLE my_table1(id INT PRIMARY KEY, v1 INT);
 * >> CREATE TABLE my_table2(id INT PRIMARY KEY, v2 INT);
 * >> INSERT INTO my_table1 (id, v1) VALUES (10, 10), (11, 20);
 * >> INSERT INTO my_table2 (id, v1) VALUES (12, 30), (13, 40);
 * >> ");
 * >>
 * >> // function query(sql: string, args: {Object...} | {string: Object}): Query;
 * >> db.query(r"SELECT * from my_table1 WHERE id = :id AND v1 < :limit", { "id": 10, "limit": 99 });
 * >> db.query(r"SELECT * from my_table1 WHERE id = ? AND v1 < ?", { 10, 99 });
 *
 * NOTE: "sqlite3_stmt" get lazily pre-compiled and are then stored alongside
 *       deemon's `DeeStringObject' (and are only destroyed when the corresponding
 *       string is, making use of `DeeString_AddFiniHook()')
 *
 * The "Query" type returned by "DB.query()" then:
 * - Extends "Sequence" (meaning you can do stuff like ".first" to get the first row)
 * - Also has methods like "fetchone()" (hint: ".fetchall()" is the same as ".frozen")
 * - Implements "operator iter(): QueryIterator", which calls "sqlite3_step()" and yield columns as `Row'.
 *   - The "Row" type then:
 *     - Implements "operator getitem(index: int): Object" to lookup columns by index (using "sqlite3_column_*()")
 *     - Implements "operator getitem(name: string): Object" to lookup columns by name (using "sqlite3_column_*()")
 *     - Implements "operator size(): int" to return the # of columns in the query (using "sqlite3_column_count()")
 *     - Implements an iterator "RowIterator" that yields {Object...} (same as >> for (local index: [:#this]) yield this[index])
 *   - When a Query steps ahead to the next row, but the "Row" object for the current row
 *     hasn't been destroyed yet, then the old "Row" is updated to contain copies of all
 *     old columns (meaning that the apparent contents of some "Row" don't change if the
 *     query is advanced)
 *
 * Above, `Object' as returned or passed into sqlite3 is always mapped as:
 * - SQLITE_INTEGER: int
 * - SQLITE_FLOAT:   float
 * - SQLITE_TEXT:    string
 * - SQLITE_BLOB:    DeeBytesObject
 * - SQLITE_NULL:    none
 */
DEX_MEMBER_F_NODOC("DB", &DB_Type, MODSYM_FREADONLY),
DEX_MEMBER_F_NODOC("Query", &Query_Type, MODSYM_FREADONLY),
DEX_MEMBER_F_NODOC("QueryIterator", &QueryIterator_Type, MODSYM_FREADONLY),
DEX_MEMBER_F_NODOC("Row", &Row_Type, MODSYM_FREADONLY),
DEX_MEMBER_F_NODOC("SQLError", &SQLError_Type, MODSYM_FREADONLY),
DEX_MEMBER_F_NODOC("SQLSyntaxError", &SQLSyntaxError_Type, MODSYM_FREADONLY),
DEX_MEMBER_F_NODOC("SQLConstraintError", &SQLConstraintError_Type, MODSYM_FREADONLY),

/* Internal types */
DEX_MEMBER_F_NODOC("_RowFmt", &RowFmt_Type, MODSYM_FREADONLY),
DEX_MEMBER_F_NODOC("_RowFmtColumns", &RowFmtColumns_Type, MODSYM_FREADONLY),
DEX_MEMBER_F_NODOC("_CellFmt", &CellFmt_Type, MODSYM_FREADONLY),

DEX_END(NULL, NULL, NULL);

DECL_END

#endif /* !GUARD_DEX_SQLITE3_LIBSQLITE3_C */
