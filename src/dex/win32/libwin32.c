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
#ifndef GUARD_DEX_WIN32_LIBWIN32_C
#define GUARD_DEX_WIN32_LIBWIN32_C 1
#define CONFIG_BUILDING_LIBWIN32
#define DEE_SOURCE

#include "libwin32.h"
#if defined(CONFIG_HOST_WINDOWS) || defined(__DEEMON__)

#include <deemon/abi/ctypes.h>
#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bytes.h>
#include <deemon/dex.h>
#include <deemon/error.h>
#include <deemon/file.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/module.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/objmethod.h>
#include <deemon/seq.h>
#include <deemon/system-features.h> /* memcpyc(), ... */
#include <deemon/system.h>
#include <deemon/thread.h>
#include <deemon/tuple.h>
#include <deemon/util/atomic.h>

#include <hybrid/debug-alignment.h>
#include <hybrid/typecore.h>
/**/

#include <stddef.h> /* size_t */
#include <stdint.h> /* uint32_t, uint64_t, uintptr_t */

DECL_BEGIN

#ifndef PATH_MAX
#ifdef PATHMAX
#   define PATH_MAX PATHMAX
#elif defined(MAX_PATH)
#   define PATH_MAX MAX_PATH
#elif defined(MAXPATH)
#   define PATH_MAX MAXPATH
#else
#   define PATH_MAX 260
#endif
#endif

#undef STD_INPUT_HANDLE
#undef STD_OUTPUT_HANDLE
#undef STD_ERROR_HANDLE
#define STD_INPUT_HANDLE    (-10)
#define STD_OUTPUT_HANDLE   (-11)
#define STD_ERROR_HANDLE    (-12)

#ifndef LIST_MODULES_DEFAULT
#define LIST_MODULES_DEFAULT 0x0
#endif /* !LIST_MODULES_DEFAULT */
#ifndef LIST_MODULES_32BIT
#define LIST_MODULES_32BIT 0x1
#endif /* !LIST_MODULES_32BIT */
#ifndef LIST_MODULES_64BIT
#define LIST_MODULES_64BIT 0x2
#endif /* !LIST_MODULES_64BIT */

#ifndef NT_ERROR
#define NT_ERROR(Status) ((((ULONG)(Status)) >> 30) == 3)
#endif /* !NT_ERROR */

#ifndef DRIVE_UNKNOWN
#define DRIVE_UNKNOWN       0
#endif /* !DRIVE_UNKNOWN */
#ifndef DRIVE_NO_ROOT_DIR
#define DRIVE_NO_ROOT_DIR   1
#endif /* !DRIVE_NO_ROOT_DIR */
#ifndef DRIVE_REMOVABLE
#define DRIVE_REMOVABLE     2
#endif /* !DRIVE_REMOVABLE */
#ifndef DRIVE_FIXED
#define DRIVE_FIXED         3
#endif /* !DRIVE_FIXED */
#ifndef DRIVE_REMOTE
#define DRIVE_REMOTE        4
#endif /* !DRIVE_REMOTE */
#ifndef DRIVE_CDROM
#define DRIVE_CDROM         5
#endif /* !DRIVE_CDROM */
#ifndef DRIVE_RAMDISK
#define DRIVE_RAMDISK       6
#endif /* !DRIVE_RAMDISK */


/*[[[deemon
import * from rt.gen.dexutils;
import * from deemon;
local orig_stdout = File.stdout;
local allDecls = [];
include("constants.def");

function wgii(name, ...) {
	allDecls.append(name);
	gii(name, ...);
}

wgii("FILE_READ_DATA",            0x0001);
wgii("FILE_LIST_DIRECTORY",       0x0001);
wgii("FILE_WRITE_DATA",           0x0002);
wgii("FILE_ADD_FILE",             0x0002);
wgii("FILE_APPEND_DATA",          0x0004);
wgii("FILE_ADD_SUBDIRECTORY",     0x0004);
wgii("FILE_CREATE_PIPE_INSTANCE", 0x0004);
wgii("FILE_READ_EA",              0x0008);
wgii("FILE_WRITE_EA",             0x0010);
wgii("FILE_EXECUTE",              0x0020);
wgii("FILE_TRAVERSE",             0x0020);
wgii("FILE_DELETE_CHILD",         0x0040);
wgii("FILE_READ_ATTRIBUTES",      0x0080);
wgii("FILE_WRITE_ATTRIBUTES",     0x0100);

// STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | 0x1FF
wgii("FILE_ALL_ACCESS",  0x000f0000 | 0x00100000 | 0x1FF);

// STANDARD_RIGHTS_READ | FILE_READ_DATA | FILE_READ_ATTRIBUTES | FILE_READ_EA | SYNCHRONIZE
wgii("FILE_GENERIC_READ", 0x00020000 | 0x0001 | 0x0080 | 0x0008 | 0x00100000);

// STANDARD_RIGHTS_WRITE | FILE_WRITE_DATA | FILE_WRITE_ATTRIBUTES | FILE_WRITE_EA | FILE_APPEND_DATA | SYNCHRONIZE
wgii("FILE_GENERIC_WRITE", 0x00020000 | 0x0002 | 0x0100 | 0x0010 | 0x0004 | 0x00100000);

// STANDARD_RIGHTS_EXECUTE | FILE_READ_ATTRIBUTES | FILE_EXECUTE | SYNCHRONIZE
wgii("FILE_GENERIC_EXECUTE", 0x00020000 | 0x0080 | 0x0020 | 0x00100000);

wgii("FILE_SHARE_READ",                     0x00000001);
wgii("FILE_SHARE_WRITE",                    0x00000002);
wgii("FILE_SHARE_DELETE",                   0x00000004);
wgii("FILE_ATTRIBUTE_READONLY",             0x00000001);
wgii("FILE_ATTRIBUTE_HIDDEN",               0x00000002);
wgii("FILE_ATTRIBUTE_SYSTEM",               0x00000004);
wgii("FILE_ATTRIBUTE_DIRECTORY",            0x00000010);
wgii("FILE_ATTRIBUTE_ARCHIVE",              0x00000020);
wgii("FILE_ATTRIBUTE_DEVICE",               0x00000040);
wgii("FILE_ATTRIBUTE_NORMAL",               0x00000080);
wgii("FILE_ATTRIBUTE_TEMPORARY",            0x00000100);
wgii("FILE_ATTRIBUTE_SPARSE_FILE",          0x00000200);
wgii("FILE_ATTRIBUTE_REPARSE_POINT",        0x00000400);
wgii("FILE_ATTRIBUTE_COMPRESSED",           0x00000800);
wgii("FILE_ATTRIBUTE_OFFLINE",              0x00001000);
wgii("FILE_ATTRIBUTE_NOT_CONTENT_INDEXED",  0x00002000);
wgii("FILE_ATTRIBUTE_ENCRYPTED",            0x00004000);
wgii("FILE_ATTRIBUTE_INTEGRITY_STREAM",     0x00008000);
wgii("FILE_ATTRIBUTE_VIRTUAL",              0x00010000);
wgii("FILE_ATTRIBUTE_NO_SCRUB_DATA",        0x00020000);
wgii("FILE_ATTRIBUTE_EA",                   0x00040000);
wgii("FILE_NOTIFY_CHANGE_FILE_NAME",        0x00000001);
wgii("FILE_NOTIFY_CHANGE_DIR_NAME",         0x00000002);
wgii("FILE_NOTIFY_CHANGE_ATTRIBUTES",       0x00000004);
wgii("FILE_NOTIFY_CHANGE_SIZE",             0x00000008);
wgii("FILE_NOTIFY_CHANGE_LAST_WRITE",       0x00000010);
wgii("FILE_NOTIFY_CHANGE_LAST_ACCESS",      0x00000020);
wgii("FILE_NOTIFY_CHANGE_CREATION",         0x00000040);
wgii("FILE_NOTIFY_CHANGE_SECURITY",         0x00000100);
wgii("FILE_ACTION_ADDED",                   0x00000001);
wgii("FILE_ACTION_REMOVED",                 0x00000002);
wgii("FILE_ACTION_MODIFIED",                0x00000003);
wgii("FILE_ACTION_RENAMED_OLD_NAME",        0x00000004);
wgii("FILE_ACTION_RENAMED_NEW_NAME",        0x00000005);
wgii("MAILSLOT_NO_MESSAGE",                 0xffffffff);
wgii("MAILSLOT_WAIT_FOREVER",               0xffffffff);
wgii("FILE_CASE_SENSITIVE_SEARCH",          0x00000001);
wgii("FILE_CASE_PRESERVED_NAMES",           0x00000002);
wgii("FILE_UNICODE_ON_DISK",                0x00000004);
wgii("FILE_PERSISTENT_ACLS",                0x00000008);
wgii("FILE_FILE_COMPRESSION",               0x00000010);
wgii("FILE_VOLUME_QUOTAS",                  0x00000020);
wgii("FILE_SUPPORTS_SPARSE_FILES",          0x00000040);
wgii("FILE_SUPPORTS_REPARSE_POINTS",        0x00000080);
wgii("FILE_SUPPORTS_REMOTE_STORAGE",        0x00000100);
wgii("FILE_VOLUME_IS_COMPRESSED",           0x00008000);
wgii("FILE_SUPPORTS_OBJECT_IDS",            0x00010000);
wgii("FILE_SUPPORTS_ENCRYPTION",            0x00020000);
wgii("FILE_NAMED_STREAMS",                  0x00040000);
wgii("FILE_READ_ONLY_VOLUME",               0x00080000);
wgii("FILE_SEQUENTIAL_WRITE_ONCE",          0x00100000);
wgii("FILE_SUPPORTS_TRANSACTIONS",          0x00200000);
wgii("FILE_SUPPORTS_HARD_LINKS",            0x00400000);
wgii("FILE_SUPPORTS_EXTENDED_ATTRIBUTES",   0x00800000);
wgii("FILE_SUPPORTS_OPEN_BY_FILE_ID",       0x01000000);
wgii("FILE_SUPPORTS_USN_JOURNAL",           0x02000000);
wgii("FILE_SUPPORTS_INTEGRITY_STREAMS",     0x04000000);

wgii("FILE_FLAG_WRITE_THROUGH",         0x80000000);
wgii("FILE_FLAG_OVERLAPPED",            0x40000000);
wgii("FILE_FLAG_NO_BUFFERING",          0x20000000);
wgii("FILE_FLAG_RANDOM_ACCESS",         0x10000000);
wgii("FILE_FLAG_SEQUENTIAL_SCAN",       0x08000000);
wgii("FILE_FLAG_DELETE_ON_CLOSE",       0x04000000);
wgii("FILE_FLAG_BACKUP_SEMANTICS",      0x02000000);
wgii("FILE_FLAG_POSIX_SEMANTICS",       0x01000000);
wgii("FILE_FLAG_SESSION_AWARE",         0x00800000);
wgii("FILE_FLAG_OPEN_REPARSE_POINT",    0x00200000);
wgii("FILE_FLAG_OPEN_NO_RECALL",        0x00100000);
wgii("FILE_FLAG_FIRST_PIPE_INSTANCE",   0x00080000);
wgii("FILE_FLAG_OPEN_REQUIRING_OPLOCK", 0x00040000);

wgii("CREATE_NEW",        1);
wgii("CREATE_ALWAYS",     2);
wgii("OPEN_EXISTING",     3);
wgii("OPEN_ALWAYS",       4);
wgii("TRUNCATE_EXISTING", 5);

wgii("INVALID_FILE_SIZE",        0xffffffff);
wgii("INVALID_SET_FILE_POINTER", 0xffffffff);
wgii("INVALID_FILE_ATTRIBUTES",  0xffffffff);

wgii("FILE_BEGIN",   0);
wgii("FILE_CURRENT", 1);
wgii("FILE_END",     2);

wgii("INFINITE",       0xffffffff);
wgii("WAIT_ABANDONED", 0x00000080);
wgii("WAIT_OBJECT_0",  0x00000000);
wgii("WAIT_TIMEOUT",   0x00000102);
wgii("WAIT_FAILED",    0xffffffff);

wgii("FILE_TYPE_UNKNOWN", 0x0000);
wgii("FILE_TYPE_DISK",    0x0001);
wgii("FILE_TYPE_CHAR",    0x0002);
wgii("FILE_TYPE_PIPE",    0x0003);
wgii("FILE_TYPE_REMOTE",  0x8000);

wgii("DRIVE_UNKNOWN",     0);
wgii("DRIVE_NO_ROOT_DIR", 1);
wgii("DRIVE_REMOVABLE",   2);
wgii("DRIVE_FIXED",       3);
wgii("DRIVE_REMOTE",      4);
wgii("DRIVE_CDROM",       5);
wgii("DRIVE_RAMDISK",     6);

wgii("DELETE",                   0x00010000);
wgii("READ_CONTROL",             0x00020000);
wgii("WRITE_DAC",                0x00040000);
wgii("WRITE_OWNER",              0x00080000);
wgii("SYNCHRONIZE",              0x00100000);
wgii("STANDARD_RIGHTS_REQUIRED", 0x000f0000);
wgii("STANDARD_RIGHTS_READ",     0x00020000); // READ_CONTROL
wgii("STANDARD_RIGHTS_WRITE",    0x00020000); // READ_CONTROL
wgii("STANDARD_RIGHTS_EXECUTE",  0x00020000); // READ_CONTROL
wgii("SPECIFIC_RIGHTS_ALL",      0x0000ffff);
wgii("STANDARD_RIGHTS_ALL",      0x001f0000);
wgii("ACCESS_SYSTEM_SECURITY",   0x01000000);
wgii("MAXIMUM_ALLOWED",          0x02000000);
wgii("GENERIC_ALL",              0x10000000);
wgii("GENERIC_EXECUTE",          0x20000000);
wgii("GENERIC_WRITE",            0x40000000);
wgii("GENERIC_READ",             0x80000000);

wgii("SECTION_QUERY",                0x0001);
wgii("SECTION_MAP_WRITE",            0x0002);
wgii("SECTION_MAP_READ",             0x0004);
wgii("SECTION_MAP_EXECUTE",          0x0008);
wgii("SECTION_EXTEND_SIZE",          0x0010);
wgii("SECTION_MAP_EXECUTE_EXPLICIT", 0x0020);
// STANDARD_RIGHTS_REQUIRED | SECTION_QUERY | SECTION_MAP_WRITE | SECTION_MAP_READ | SECTION_MAP_EXECUTE | SECTION_EXTEND_SIZE
wgii("SECTION_ALL_ACCESS",           0x000f0000 | 0x0001 | 0x0002 | 0x0004 | 0x0008 | 0x0010);
wgii("SESSION_QUERY_ACCESS",         0x0001);
wgii("SESSION_MODIFY_ACCESS",        0x0002);
// STANDARD_RIGHTS_REQUIRED | SESSION_QUERY_ACCESS | SESSION_MODIFY_ACCESS
wgii("SESSION_ALL_ACCESS",           0x000f0000 | 0x0001 | 0x0002);

wgii("PAGE_NOACCESS",           0x01);
wgii("PAGE_READONLY",           0x02);
wgii("PAGE_READWRITE",          0x04);
wgii("PAGE_WRITECOPY",          0x08);
wgii("PAGE_EXECUTE",            0x10);
wgii("PAGE_EXECUTE_READ",       0x20);
wgii("PAGE_EXECUTE_READWRITE",  0x40);
wgii("PAGE_EXECUTE_WRITECOPY",  0x80);
wgii("PAGE_GUARD",              0x100);
wgii("PAGE_NOCACHE",            0x200);
wgii("PAGE_WRITECOMBINE",       0x400);
wgii("PAGE_REVERT_TO_FILE_MAP", 0x80000000);

wgii("MEM_COMMIT",                  0x1000);
wgii("MEM_RESERVE",                 0x2000);
wgii("MEM_DECOMMIT",                0x4000);
wgii("MEM_RELEASE",                 0x8000);
wgii("MEM_FREE",                    0x10000);
wgii("MEM_PRIVATE",                 0x20000);
wgii("MEM_MAPPED",                  0x40000);
wgii("MEM_RESET",                   0x80000);
wgii("MEM_TOP_DOWN",                0x100000);
wgii("MEM_WRITE_WATCH",             0x200000);
wgii("MEM_PHYSICAL",                0x400000);
wgii("MEM_ROTATE",                  0x800000);
wgii("MEM_DIFFERENT_IMAGE_BASE_OK", 0x800000);
wgii("MEM_RESET_UNDO",              0x1000000);
wgii("MEM_LARGE_PAGES",             0x20000000);
wgii("MEM_4MB_PAGES",               0x80000000);

wgii("SEC_FILE",             0x00800000);
wgii("SEC_IMAGE",            0x01000000);
wgii("SEC_PROTECTED_IMAGE",  0x02000000);
wgii("SEC_RESERVE",          0x04000000);
wgii("SEC_COMMIT",           0x08000000);
wgii("SEC_NOCACHE",          0x10000000);
wgii("SEC_WRITECOMBINE",     0x40000000);
wgii("SEC_LARGE_PAGES",      0x80000000);
wgii("SEC_IMAGE_NO_EXECUTE", 0x01000000 | 0x10000000); // SEC_IMAGE | SEC_NOCACHE
wgii("MEM_IMAGE",            0x01000000); // SEC_IMAGE
wgii("WRITE_WATCH_FLAG_RESET",  0x01);
wgii("MEM_UNMAP_WITH_TRANSIENT_BOOST",  0x01);

wgii("FILE_MAP_WRITE",      0x0002); // SECTION_MAP_WRITE
wgii("FILE_MAP_READ",       0x0004); // SECTION_MAP_READ
wgii("FILE_MAP_ALL_ACCESS", 0x000f0000 | 0x0001 | 0x0002 | 0x0004 | 0x0008 | 0x0010); // SECTION_ALL_ACCESS
wgii("FILE_MAP_COPY",       0x00000001);
wgii("FILE_MAP_RESERVE",    0x80000000);
wgii("FILE_MAP_EXECUTE",    0x0020); // SECTION_MAP_EXECUTE_EXPLICIT
//TODO: wgii("FILE_MAP_LARGE_PAGES");
//TODO: wgii("FILE_MAP_TARGETS_INVALID");

wgii("STD_INPUT_HANDLE",  -10);
wgii("STD_OUTPUT_HANDLE", -11);
wgii("STD_ERROR_HANDLE",  -12);

wgii("PIPE_ACCESS_INBOUND",  0x00000001);
wgii("PIPE_ACCESS_OUTBOUND", 0x00000002);
wgii("PIPE_ACCESS_DUPLEX",   0x00000003);

wgii("PIPE_CLIENT_END", 0x00000000);
wgii("PIPE_SERVER_END", 0x00000001);

wgii("PIPE_WAIT",                  0x00000000);
wgii("PIPE_NOWAIT",                0x00000001);
wgii("PIPE_READMODE_BYTE",         0x00000000);
wgii("PIPE_READMODE_MESSAGE",      0x00000002);
wgii("PIPE_TYPE_BYTE",             0x00000000);
wgii("PIPE_TYPE_MESSAGE",          0x00000004);
wgii("PIPE_ACCEPT_REMOTE_CLIENTS", 0x00000000);
wgii("PIPE_REJECT_REMOTE_CLIENTS", 0x00000008);

wgii("PIPE_UNLIMITED_INSTANCES", 255);

wgii("NMPWAIT_WAIT_FOREVER",     0xffffffff);
wgii("NMPWAIT_NOWAIT",           0x00000001);
wgii("NMPWAIT_USE_DEFAULT_WAIT", 0x00000000);

wgii("PROCESS_TERMINATE",                 0x0001);
wgii("PROCESS_CREATE_THREAD",             0x0002);
wgii("PROCESS_SET_SESSIONID",             0x0004);
wgii("PROCESS_VM_OPERATION",              0x0008);
wgii("PROCESS_VM_READ",                   0x0010);
wgii("PROCESS_VM_WRITE",                  0x0020);
wgii("PROCESS_DUP_HANDLE",                0x0040);
wgii("PROCESS_CREATE_PROCESS",            0x0080);
wgii("PROCESS_SET_QUOTA",                 0x0100);
wgii("PROCESS_SET_INFORMATION",           0x0200);
wgii("PROCESS_QUERY_INFORMATION",         0x0400);
wgii("PROCESS_SUSPEND_RESUME",            0x0800);
wgii("PROCESS_QUERY_LIMITED_INFORMATION", 0x1000);
wgii("PROCESS_SET_LIMITED_INFORMATION",   0x2000);

// STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | 0xfff
wgii("PROCESS_ALL_ACCESS", 0x000f0000 | 0x00100000 | 0xFFF);

wgii("THREAD_TERMINATE",                 0x0001);
wgii("THREAD_SUSPEND_RESUME",            0x0002);
wgii("THREAD_GET_CONTEXT",               0x0008);
wgii("THREAD_SET_CONTEXT",               0x0010);
wgii("THREAD_QUERY_INFORMATION",         0x0040);
wgii("THREAD_SET_INFORMATION",           0x0020);
wgii("THREAD_SET_THREAD_TOKEN",          0x0080);
wgii("THREAD_IMPERSONATE",               0x0100);
wgii("THREAD_DIRECT_IMPERSONATION",      0x0200);
wgii("THREAD_SET_LIMITED_INFORMATION",   0x0400);
wgii("THREAD_QUERY_LIMITED_INFORMATION", 0x0800);
wgii("THREAD_RESUME",                    0x1000);

// STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | 0x3ff
wgii("THREAD_ALL_ACCESS", 0x000f0000 | 0x00100000 | 0x3ff);

wgii("JOB_OBJECT_ASSIGN_PROCESS",          0x0001);
wgii("JOB_OBJECT_SET_ATTRIBUTES",          0x0002);
wgii("JOB_OBJECT_QUERY",                   0x0004);
wgii("JOB_OBJECT_TERMINATE",               0x0008);
wgii("JOB_OBJECT_SET_SECURITY_ATTRIBUTES", 0x0010);

// STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | 0x1f
wgii("JOB_OBJECT_ALL_ACCESS", 0x000f0000 | 0x00100000 | 0x1f);

wgii("LIST_MODULES_DEFAULT", 0);
wgii("LIST_MODULES_32BIT",   1);
wgii("LIST_MODULES_64BIT",   2);
wgii("LIST_MODULES_ALL",     3); // LIST_MODULES_32BIT | LIST_MODULES_64BIT

wgii("DUPLICATE_CLOSE_SOURCE", 0x00000001);
wgii("DUPLICATE_SAME_ACCESS",  0x00000002);

File.stdout = orig_stdout;
print "#define LIBWIN32_CONSTANTS_DEFS \\";
for (local x: allDecls)
	print("\tLIBWIN32_", x, "_DEF \\");
print "/" "**" "/";


]]]*/
#include "constants.def"
#define LIBWIN32_CONSTANTS_DEFS \
	LIBWIN32_FILE_READ_DATA_DEF \
	LIBWIN32_FILE_LIST_DIRECTORY_DEF \
	LIBWIN32_FILE_WRITE_DATA_DEF \
	LIBWIN32_FILE_ADD_FILE_DEF \
	LIBWIN32_FILE_APPEND_DATA_DEF \
	LIBWIN32_FILE_ADD_SUBDIRECTORY_DEF \
	LIBWIN32_FILE_CREATE_PIPE_INSTANCE_DEF \
	LIBWIN32_FILE_READ_EA_DEF \
	LIBWIN32_FILE_WRITE_EA_DEF \
	LIBWIN32_FILE_EXECUTE_DEF \
	LIBWIN32_FILE_TRAVERSE_DEF \
	LIBWIN32_FILE_DELETE_CHILD_DEF \
	LIBWIN32_FILE_READ_ATTRIBUTES_DEF \
	LIBWIN32_FILE_WRITE_ATTRIBUTES_DEF \
	LIBWIN32_FILE_ALL_ACCESS_DEF \
	LIBWIN32_FILE_GENERIC_READ_DEF \
	LIBWIN32_FILE_GENERIC_WRITE_DEF \
	LIBWIN32_FILE_GENERIC_EXECUTE_DEF \
	LIBWIN32_FILE_SHARE_READ_DEF \
	LIBWIN32_FILE_SHARE_WRITE_DEF \
	LIBWIN32_FILE_SHARE_DELETE_DEF \
	LIBWIN32_FILE_ATTRIBUTE_READONLY_DEF \
	LIBWIN32_FILE_ATTRIBUTE_HIDDEN_DEF \
	LIBWIN32_FILE_ATTRIBUTE_SYSTEM_DEF \
	LIBWIN32_FILE_ATTRIBUTE_DIRECTORY_DEF \
	LIBWIN32_FILE_ATTRIBUTE_ARCHIVE_DEF \
	LIBWIN32_FILE_ATTRIBUTE_DEVICE_DEF \
	LIBWIN32_FILE_ATTRIBUTE_NORMAL_DEF \
	LIBWIN32_FILE_ATTRIBUTE_TEMPORARY_DEF \
	LIBWIN32_FILE_ATTRIBUTE_SPARSE_FILE_DEF \
	LIBWIN32_FILE_ATTRIBUTE_REPARSE_POINT_DEF \
	LIBWIN32_FILE_ATTRIBUTE_COMPRESSED_DEF \
	LIBWIN32_FILE_ATTRIBUTE_OFFLINE_DEF \
	LIBWIN32_FILE_ATTRIBUTE_NOT_CONTENT_INDEXED_DEF \
	LIBWIN32_FILE_ATTRIBUTE_ENCRYPTED_DEF \
	LIBWIN32_FILE_ATTRIBUTE_INTEGRITY_STREAM_DEF \
	LIBWIN32_FILE_ATTRIBUTE_VIRTUAL_DEF \
	LIBWIN32_FILE_ATTRIBUTE_NO_SCRUB_DATA_DEF \
	LIBWIN32_FILE_ATTRIBUTE_EA_DEF \
	LIBWIN32_FILE_NOTIFY_CHANGE_FILE_NAME_DEF \
	LIBWIN32_FILE_NOTIFY_CHANGE_DIR_NAME_DEF \
	LIBWIN32_FILE_NOTIFY_CHANGE_ATTRIBUTES_DEF \
	LIBWIN32_FILE_NOTIFY_CHANGE_SIZE_DEF \
	LIBWIN32_FILE_NOTIFY_CHANGE_LAST_WRITE_DEF \
	LIBWIN32_FILE_NOTIFY_CHANGE_LAST_ACCESS_DEF \
	LIBWIN32_FILE_NOTIFY_CHANGE_CREATION_DEF \
	LIBWIN32_FILE_NOTIFY_CHANGE_SECURITY_DEF \
	LIBWIN32_FILE_ACTION_ADDED_DEF \
	LIBWIN32_FILE_ACTION_REMOVED_DEF \
	LIBWIN32_FILE_ACTION_MODIFIED_DEF \
	LIBWIN32_FILE_ACTION_RENAMED_OLD_NAME_DEF \
	LIBWIN32_FILE_ACTION_RENAMED_NEW_NAME_DEF \
	LIBWIN32_MAILSLOT_NO_MESSAGE_DEF \
	LIBWIN32_MAILSLOT_WAIT_FOREVER_DEF \
	LIBWIN32_FILE_CASE_SENSITIVE_SEARCH_DEF \
	LIBWIN32_FILE_CASE_PRESERVED_NAMES_DEF \
	LIBWIN32_FILE_UNICODE_ON_DISK_DEF \
	LIBWIN32_FILE_PERSISTENT_ACLS_DEF \
	LIBWIN32_FILE_FILE_COMPRESSION_DEF \
	LIBWIN32_FILE_VOLUME_QUOTAS_DEF \
	LIBWIN32_FILE_SUPPORTS_SPARSE_FILES_DEF \
	LIBWIN32_FILE_SUPPORTS_REPARSE_POINTS_DEF \
	LIBWIN32_FILE_SUPPORTS_REMOTE_STORAGE_DEF \
	LIBWIN32_FILE_VOLUME_IS_COMPRESSED_DEF \
	LIBWIN32_FILE_SUPPORTS_OBJECT_IDS_DEF \
	LIBWIN32_FILE_SUPPORTS_ENCRYPTION_DEF \
	LIBWIN32_FILE_NAMED_STREAMS_DEF \
	LIBWIN32_FILE_READ_ONLY_VOLUME_DEF \
	LIBWIN32_FILE_SEQUENTIAL_WRITE_ONCE_DEF \
	LIBWIN32_FILE_SUPPORTS_TRANSACTIONS_DEF \
	LIBWIN32_FILE_SUPPORTS_HARD_LINKS_DEF \
	LIBWIN32_FILE_SUPPORTS_EXTENDED_ATTRIBUTES_DEF \
	LIBWIN32_FILE_SUPPORTS_OPEN_BY_FILE_ID_DEF \
	LIBWIN32_FILE_SUPPORTS_USN_JOURNAL_DEF \
	LIBWIN32_FILE_SUPPORTS_INTEGRITY_STREAMS_DEF \
	LIBWIN32_FILE_FLAG_WRITE_THROUGH_DEF \
	LIBWIN32_FILE_FLAG_OVERLAPPED_DEF \
	LIBWIN32_FILE_FLAG_NO_BUFFERING_DEF \
	LIBWIN32_FILE_FLAG_RANDOM_ACCESS_DEF \
	LIBWIN32_FILE_FLAG_SEQUENTIAL_SCAN_DEF \
	LIBWIN32_FILE_FLAG_DELETE_ON_CLOSE_DEF \
	LIBWIN32_FILE_FLAG_BACKUP_SEMANTICS_DEF \
	LIBWIN32_FILE_FLAG_POSIX_SEMANTICS_DEF \
	LIBWIN32_FILE_FLAG_SESSION_AWARE_DEF \
	LIBWIN32_FILE_FLAG_OPEN_REPARSE_POINT_DEF \
	LIBWIN32_FILE_FLAG_OPEN_NO_RECALL_DEF \
	LIBWIN32_FILE_FLAG_FIRST_PIPE_INSTANCE_DEF \
	LIBWIN32_FILE_FLAG_OPEN_REQUIRING_OPLOCK_DEF \
	LIBWIN32_CREATE_NEW_DEF \
	LIBWIN32_CREATE_ALWAYS_DEF \
	LIBWIN32_OPEN_EXISTING_DEF \
	LIBWIN32_OPEN_ALWAYS_DEF \
	LIBWIN32_TRUNCATE_EXISTING_DEF \
	LIBWIN32_INVALID_FILE_SIZE_DEF \
	LIBWIN32_INVALID_SET_FILE_POINTER_DEF \
	LIBWIN32_INVALID_FILE_ATTRIBUTES_DEF \
	LIBWIN32_FILE_BEGIN_DEF \
	LIBWIN32_FILE_CURRENT_DEF \
	LIBWIN32_FILE_END_DEF \
	LIBWIN32_INFINITE_DEF \
	LIBWIN32_WAIT_ABANDONED_DEF \
	LIBWIN32_WAIT_OBJECT_0_DEF \
	LIBWIN32_WAIT_TIMEOUT_DEF \
	LIBWIN32_WAIT_FAILED_DEF \
	LIBWIN32_FILE_TYPE_UNKNOWN_DEF \
	LIBWIN32_FILE_TYPE_DISK_DEF \
	LIBWIN32_FILE_TYPE_CHAR_DEF \
	LIBWIN32_FILE_TYPE_PIPE_DEF \
	LIBWIN32_FILE_TYPE_REMOTE_DEF \
	LIBWIN32_DRIVE_UNKNOWN_DEF \
	LIBWIN32_DRIVE_NO_ROOT_DIR_DEF \
	LIBWIN32_DRIVE_REMOVABLE_DEF \
	LIBWIN32_DRIVE_FIXED_DEF \
	LIBWIN32_DRIVE_REMOTE_DEF \
	LIBWIN32_DRIVE_CDROM_DEF \
	LIBWIN32_DRIVE_RAMDISK_DEF \
	LIBWIN32_DELETE_DEF \
	LIBWIN32_READ_CONTROL_DEF \
	LIBWIN32_WRITE_DAC_DEF \
	LIBWIN32_WRITE_OWNER_DEF \
	LIBWIN32_SYNCHRONIZE_DEF \
	LIBWIN32_STANDARD_RIGHTS_REQUIRED_DEF \
	LIBWIN32_STANDARD_RIGHTS_READ_DEF \
	LIBWIN32_STANDARD_RIGHTS_WRITE_DEF \
	LIBWIN32_STANDARD_RIGHTS_EXECUTE_DEF \
	LIBWIN32_SPECIFIC_RIGHTS_ALL_DEF \
	LIBWIN32_STANDARD_RIGHTS_ALL_DEF \
	LIBWIN32_ACCESS_SYSTEM_SECURITY_DEF \
	LIBWIN32_MAXIMUM_ALLOWED_DEF \
	LIBWIN32_GENERIC_ALL_DEF \
	LIBWIN32_GENERIC_EXECUTE_DEF \
	LIBWIN32_GENERIC_WRITE_DEF \
	LIBWIN32_GENERIC_READ_DEF \
	LIBWIN32_SECTION_QUERY_DEF \
	LIBWIN32_SECTION_MAP_WRITE_DEF \
	LIBWIN32_SECTION_MAP_READ_DEF \
	LIBWIN32_SECTION_MAP_EXECUTE_DEF \
	LIBWIN32_SECTION_EXTEND_SIZE_DEF \
	LIBWIN32_SECTION_MAP_EXECUTE_EXPLICIT_DEF \
	LIBWIN32_SECTION_ALL_ACCESS_DEF \
	LIBWIN32_SESSION_QUERY_ACCESS_DEF \
	LIBWIN32_SESSION_MODIFY_ACCESS_DEF \
	LIBWIN32_SESSION_ALL_ACCESS_DEF \
	LIBWIN32_PAGE_NOACCESS_DEF \
	LIBWIN32_PAGE_READONLY_DEF \
	LIBWIN32_PAGE_READWRITE_DEF \
	LIBWIN32_PAGE_WRITECOPY_DEF \
	LIBWIN32_PAGE_EXECUTE_DEF \
	LIBWIN32_PAGE_EXECUTE_READ_DEF \
	LIBWIN32_PAGE_EXECUTE_READWRITE_DEF \
	LIBWIN32_PAGE_EXECUTE_WRITECOPY_DEF \
	LIBWIN32_PAGE_GUARD_DEF \
	LIBWIN32_PAGE_NOCACHE_DEF \
	LIBWIN32_PAGE_WRITECOMBINE_DEF \
	LIBWIN32_PAGE_REVERT_TO_FILE_MAP_DEF \
	LIBWIN32_MEM_COMMIT_DEF \
	LIBWIN32_MEM_RESERVE_DEF \
	LIBWIN32_MEM_DECOMMIT_DEF \
	LIBWIN32_MEM_RELEASE_DEF \
	LIBWIN32_MEM_FREE_DEF \
	LIBWIN32_MEM_PRIVATE_DEF \
	LIBWIN32_MEM_MAPPED_DEF \
	LIBWIN32_MEM_RESET_DEF \
	LIBWIN32_MEM_TOP_DOWN_DEF \
	LIBWIN32_MEM_WRITE_WATCH_DEF \
	LIBWIN32_MEM_PHYSICAL_DEF \
	LIBWIN32_MEM_ROTATE_DEF \
	LIBWIN32_MEM_DIFFERENT_IMAGE_BASE_OK_DEF \
	LIBWIN32_MEM_RESET_UNDO_DEF \
	LIBWIN32_MEM_LARGE_PAGES_DEF \
	LIBWIN32_MEM_4MB_PAGES_DEF \
	LIBWIN32_SEC_FILE_DEF \
	LIBWIN32_SEC_IMAGE_DEF \
	LIBWIN32_SEC_PROTECTED_IMAGE_DEF \
	LIBWIN32_SEC_RESERVE_DEF \
	LIBWIN32_SEC_COMMIT_DEF \
	LIBWIN32_SEC_NOCACHE_DEF \
	LIBWIN32_SEC_WRITECOMBINE_DEF \
	LIBWIN32_SEC_LARGE_PAGES_DEF \
	LIBWIN32_SEC_IMAGE_NO_EXECUTE_DEF \
	LIBWIN32_MEM_IMAGE_DEF \
	LIBWIN32_WRITE_WATCH_FLAG_RESET_DEF \
	LIBWIN32_MEM_UNMAP_WITH_TRANSIENT_BOOST_DEF \
	LIBWIN32_FILE_MAP_WRITE_DEF \
	LIBWIN32_FILE_MAP_READ_DEF \
	LIBWIN32_FILE_MAP_ALL_ACCESS_DEF \
	LIBWIN32_FILE_MAP_COPY_DEF \
	LIBWIN32_FILE_MAP_RESERVE_DEF \
	LIBWIN32_FILE_MAP_EXECUTE_DEF \
	LIBWIN32_STD_INPUT_HANDLE_DEF \
	LIBWIN32_STD_OUTPUT_HANDLE_DEF \
	LIBWIN32_STD_ERROR_HANDLE_DEF \
	LIBWIN32_PIPE_ACCESS_INBOUND_DEF \
	LIBWIN32_PIPE_ACCESS_OUTBOUND_DEF \
	LIBWIN32_PIPE_ACCESS_DUPLEX_DEF \
	LIBWIN32_PIPE_CLIENT_END_DEF \
	LIBWIN32_PIPE_SERVER_END_DEF \
	LIBWIN32_PIPE_WAIT_DEF \
	LIBWIN32_PIPE_NOWAIT_DEF \
	LIBWIN32_PIPE_READMODE_BYTE_DEF \
	LIBWIN32_PIPE_READMODE_MESSAGE_DEF \
	LIBWIN32_PIPE_TYPE_BYTE_DEF \
	LIBWIN32_PIPE_TYPE_MESSAGE_DEF \
	LIBWIN32_PIPE_ACCEPT_REMOTE_CLIENTS_DEF \
	LIBWIN32_PIPE_REJECT_REMOTE_CLIENTS_DEF \
	LIBWIN32_PIPE_UNLIMITED_INSTANCES_DEF \
	LIBWIN32_NMPWAIT_WAIT_FOREVER_DEF \
	LIBWIN32_NMPWAIT_NOWAIT_DEF \
	LIBWIN32_NMPWAIT_USE_DEFAULT_WAIT_DEF \
	LIBWIN32_PROCESS_TERMINATE_DEF \
	LIBWIN32_PROCESS_CREATE_THREAD_DEF \
	LIBWIN32_PROCESS_SET_SESSIONID_DEF \
	LIBWIN32_PROCESS_VM_OPERATION_DEF \
	LIBWIN32_PROCESS_VM_READ_DEF \
	LIBWIN32_PROCESS_VM_WRITE_DEF \
	LIBWIN32_PROCESS_DUP_HANDLE_DEF \
	LIBWIN32_PROCESS_CREATE_PROCESS_DEF \
	LIBWIN32_PROCESS_SET_QUOTA_DEF \
	LIBWIN32_PROCESS_SET_INFORMATION_DEF \
	LIBWIN32_PROCESS_QUERY_INFORMATION_DEF \
	LIBWIN32_PROCESS_SUSPEND_RESUME_DEF \
	LIBWIN32_PROCESS_QUERY_LIMITED_INFORMATION_DEF \
	LIBWIN32_PROCESS_SET_LIMITED_INFORMATION_DEF \
	LIBWIN32_PROCESS_ALL_ACCESS_DEF \
	LIBWIN32_THREAD_TERMINATE_DEF \
	LIBWIN32_THREAD_SUSPEND_RESUME_DEF \
	LIBWIN32_THREAD_GET_CONTEXT_DEF \
	LIBWIN32_THREAD_SET_CONTEXT_DEF \
	LIBWIN32_THREAD_QUERY_INFORMATION_DEF \
	LIBWIN32_THREAD_SET_INFORMATION_DEF \
	LIBWIN32_THREAD_SET_THREAD_TOKEN_DEF \
	LIBWIN32_THREAD_IMPERSONATE_DEF \
	LIBWIN32_THREAD_DIRECT_IMPERSONATION_DEF \
	LIBWIN32_THREAD_SET_LIMITED_INFORMATION_DEF \
	LIBWIN32_THREAD_QUERY_LIMITED_INFORMATION_DEF \
	LIBWIN32_THREAD_RESUME_DEF \
	LIBWIN32_THREAD_ALL_ACCESS_DEF \
	LIBWIN32_JOB_OBJECT_ASSIGN_PROCESS_DEF \
	LIBWIN32_JOB_OBJECT_SET_ATTRIBUTES_DEF \
	LIBWIN32_JOB_OBJECT_QUERY_DEF \
	LIBWIN32_JOB_OBJECT_TERMINATE_DEF \
	LIBWIN32_JOB_OBJECT_SET_SECURITY_ATTRIBUTES_DEF \
	LIBWIN32_JOB_OBJECT_ALL_ACCESS_DEF \
	LIBWIN32_LIST_MODULES_DEFAULT_DEF \
	LIBWIN32_LIST_MODULES_32BIT_DEF \
	LIBWIN32_LIST_MODULES_64BIT_DEF \
	LIBWIN32_LIST_MODULES_ALL_DEF \
	LIBWIN32_DUPLICATE_CLOSE_SOURCE_DEF \
	LIBWIN32_DUPLICATE_SAME_ACCESS_DEF \
/**/
/*[[[end]]]*/


typedef struct {
	OBJECT_HEAD
	HANDLE ho_handle; /* The bound handle. */
} DeeHandleObject;

#ifndef LIBWIN32_KWDS_HHANDLE_DEFINED
#define LIBWIN32_KWDS_HHANDLE_DEFINED 1
PRIVATE DEFINE_KWLIST(libwin32_kwds_hHandle, { K(hHandle), KEND });
#endif /* !LIBWIN32_KWDS_HHANDLE_DEFINED */

PRIVATE WUNUSED NONNULL((1)) int DCALL
handle_init_kw(DeeHandleObject *__restrict self,
               size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *hHandle = Dee_None;
	if (DeeArg_UnpackStructKw(argc, argv, kw, libwin32_kwds_hHandle,
	                          "|o:HANDLE", &hHandle))
		goto err;
	if (DeeNone_Check(hHandle)) {
		/* none/default */
		self->ho_handle = INVALID_HANDLE_VALUE;
	} else if (DeeInt_Check(hHandle)) {
		/* Integer handle value */
		if (DeeInt_AsUIntptr(hHandle, (uintptr_t *)&self->ho_handle))
			goto err;
	} else {
		DREF DeeObject *ptr_attrib;
		/* Check for ctypes pointer support */
		ptr_attrib = DeeObject_GetAttrString(hHandle, "__ptr__");
		if (ptr_attrib) {
			int error;
			error = DeeObject_AsUIntptr(ptr_attrib, (uintptr_t *)&self->ho_handle);
			Dee_Decref(ptr_attrib);
			if unlikely(error)
				goto err;
		} else {
			if (!DeeError_Catch(&DeeError_AttributeError) &&
			    !DeeError_Catch(&DeeError_NotImplemented))
				goto err;
			/* Fallback: Numeric-like -> Convert to an integer. */
			if (DeeObject_AsUIntptr(hHandle, (uintptr_t *)&self->ho_handle))
				goto err;
		}
	}
	return 0;
err:
	return -1;
}

#if __SIZEOF_POINTER__ > 4
PRIVATE WUNUSED NONNULL((1)) int DCALL
handle_int64(DeeHandleObject *__restrict self,
             int64_t *__restrict result) {
	*result = (int64_t)(uint64_t)self->ho_handle;
	return INT_UNSIGNED;
}
#else /* __SIZEOF_POINTER__ > 4 */
PRIVATE WUNUSED NONNULL((1)) int DCALL
handle_int32(DeeHandleObject *__restrict self,
             int32_t *__restrict result) {
	*result = (int32_t)(uint32_t)self->ho_handle;
	return INT_UNSIGNED;
}
#endif /* __SIZEOF_POINTER__ <= 4 */


PRIVATE struct type_math handle_math = {
#if __SIZEOF_POINTER__ > 4
	/* .tp_int32 = */ (int (DCALL *)(DeeObject *__restrict, int32_t *__restrict))NULL,
	/* .tp_int64 = */ (int (DCALL *)(DeeObject *__restrict, int64_t *__restrict))&handle_int64
#else /* __SIZEOF_POINTER__ > 4 */
	/* .tp_int32 = */ (int (DCALL *)(DeeObject *__restrict, int32_t *__restrict))&handle_int32,
	/* .tp_int64 = */ (int (DCALL *)(DeeObject *__restrict, int64_t *__restrict))NULL
#endif /* __SIZEOF_POINTER__ <= 4 */
};

PRIVATE struct type_member tpconst handle_members[] = {
	TYPE_MEMBER_FIELD(Dee_fd_osfhandle_GETSET,
	                  STRUCT_UINTPTR_T,
	                  offsetof(DeeHandleObject, ho_handle)),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
handle_print(DeeHandleObject *__restrict self,
             Dee_formatprinter_t printer, void *arg) {
	return DeeFormat_Printf(printer, arg, "<handle %p>", self->ho_handle);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
handle_printrepr(DeeHandleObject *__restrict self,
                 Dee_formatprinter_t printer, void *arg) {
	return DeeFormat_Printf(printer, arg, "HANDLE(0x%p)", self->ho_handle);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
handle_bool(DeeHandleObject *__restrict self) {
	return self->ho_handle != 0;
}

PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
handle_hash(DeeHandleObject *__restrict self) {
	return Dee_HashPointer(self->ho_handle);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
handle_compare(DeeHandleObject *self, DeeObject *some_object) {
	HANDLE hOtherHandle;
	if (DeeNTSystem_TryGetHandle(some_object, &hOtherHandle))
		goto err;
	Dee_return_compare(self->ho_handle, hOtherHandle);
err:
	return Dee_COMPARE_ERR;
}

PRIVATE struct type_cmp handle_cmp = {
	/* .tp_hash       = */ (Dee_hash_t (DCALL *)(DeeObject *__restrict))&handle_hash,
	/* .tp_compare_eq = */ NULL,
	/* .tp_compare    = */ (int (DCALL *)(DeeObject *, DeeObject *))&handle_compare,
};

PRIVATE DeeTypeObject DeeHandle_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "HANDLE",
	/* .tp_doc      = */ DOC("(hHandle:?Dint=!GINVALID_HANDLE_VALUE)\n"
	                         "(hHandle:?Aptr?Ectypes:void)\n"
	                         "NOTE: Passing ?N for @hHandle will also "
	                         "initialize the HANDLE as :INVALID_HANDLE_VALUE"),
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
				TYPE_FIXED_ALLOCATOR(DeeHandleObject),
				/* .tp_any_ctor_kw = */ (Dee_funptr_t)&handle_init_kw,
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ (int (DCALL *)(DeeObject *__restrict))&handle_bool,
		/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&handle_print,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&handle_printrepr
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ &handle_math,
	/* .tp_cmp           = */ &handle_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ handle_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

PRIVATE DeeHandleObject Dee_INVALID_HANDLE_VALUE = {
	Dee_OBJECT_HEAD_INIT(&DeeHandle_Type),
	/* .ho_handle = */ INVALID_HANDLE_VALUE
};


/* Create an object wrapper for the given `hHandle'
 * The returned object implements an attribute `osfhandle_np',
 * thus allowing it to be used APIs using `DeeNTSystem_GetHandle()'
 * in order to access an underlying Windows HANDLE.
 * NOTE: The returned object does _NOT_ automatically close the bound
 *       handle. - This wrapper is only used for binary compatibility
 *       with the internal interfaces for handles vs. file descriptors! */
#define libwin32_CreateHandle_ALWAYS_RETURNS_UNIQUE_REFERENCE
PRIVATE WUNUSED DREF DeeObject *DCALL
libwin32_CreateHandle(HANDLE hHandle) {
	DREF DeeHandleObject *result;
	result = DeeObject_MALLOC(DeeHandleObject);
	if unlikely(!result)
		goto done;
	result->ho_handle = hHandle;
	DeeObject_Init(result, &DeeHandle_Type);
done:
	return (DREF DeeObject *)result;
}


#if 1
#define ERROR_OR_BOOL ""
#define MAYBE_NONE(x) x
#define RETURN_ERROR(dwError, ...)                           \
	do {                                                     \
		DeeNTSystem_ThrowErrorf(NULL, dwError, __VA_ARGS__); \
		return NULL;                                         \
	}	__WHILE0
#define RETURN_ERROR_OR_FALSE  RETURN_ERROR
#define RETURN_SUCCESS_OR_TRUE return_none
#else
#define ERROR_OR_BOOL "->?Dbool"
#define MAYBE_NONE(x) "?X2" x "?N"
#define RETURN_ERROR(dwError, ...)          return_none
#define RETURN_ERROR_OR_FALSE(dwError, ...) return_false
#define RETURN_SUCCESS_OR_TRUE              return_true
#endif




/*[[[deemon import("rt.gen.dexutils").gw("GetHandle", "hHandle:nt:HANDLE->?GHANDLE"); ]]]*/
#define LIBWIN32_GETHANDLE_DEF          DEX_MEMBER_F("GetHandle", &libwin32_GetHandle, DEXSYM_READONLY, "(hHandle:?X3?Dint?DFile?Ewin32:HANDLE)->?GHANDLE"),
#define LIBWIN32_GETHANDLE_DEF_DOC(doc) DEX_MEMBER_F("GetHandle", &libwin32_GetHandle, DEXSYM_READONLY, "(hHandle:?X3?Dint?DFile?Ewin32:HANDLE)->?GHANDLE\n" doc),
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetHandle_f_impl(HANDLE hHandle);
#ifndef DEFINED_kwlist__hHandle
#define DEFINED_kwlist__hHandle
PRIVATE DEFINE_KWLIST(kwlist__hHandle, { KEX("hHandle", 0x69103c85, 0xc1f4ea73037322b8), KEND });
#endif /* !DEFINED_kwlist__hHandle */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetHandle_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *raw_hHandle;
	} args;
	HANDLE hHandle;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__hHandle, "o:GetHandle", &args))
		goto err;
	if unlikely(DeeNTSystem_TryGetHandle(args.raw_hHandle, (void **)&hHandle))
		goto err;
	return libwin32_GetHandle_f_impl(hHandle);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_GetHandle, &libwin32_GetHandle_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetHandle_f_impl(HANDLE hHandle)
/*[[[end]]]*/
{
	return libwin32_CreateHandle(hHandle);
}

/*[[[deemon import("rt.gen.dexutils").gw("GetLastError", "->?Dint", ispure: true); ]]]*/
#define LIBWIN32_GETLASTERROR_DEF          DEX_MEMBER_F("GetLastError", &libwin32_GetLastError, DEXSYM_READONLY, "->?Dint"),
#define LIBWIN32_GETLASTERROR_DEF_DOC(doc) DEX_MEMBER_F("GetLastError", &libwin32_GetLastError, DEXSYM_READONLY, "->?Dint\n" doc),
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetLastError_f_impl(void);
PRIVATE DEFINE_CMETHOD0(libwin32_GetLastError, &libwin32_GetLastError_f_impl, METHOD_FPURECALL);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetLastError_f_impl(void)
/*[[[end]]]*/
{
	return DeeInt_NewUInt32((uint32_t)GetLastError());
}

/*[[[deemon import("rt.gen.dexutils").gw("SetLastError", "dwErrCode:nt:DWORD"); ]]]*/
#define LIBWIN32_SETLASTERROR_DEF          DEX_MEMBER_F("SetLastError", &libwin32_SetLastError, DEXSYM_READONLY, "(dwErrCode:?Dint)"),
#define LIBWIN32_SETLASTERROR_DEF_DOC(doc) DEX_MEMBER_F("SetLastError", &libwin32_SetLastError, DEXSYM_READONLY, "(dwErrCode:?Dint)\n" doc),
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_SetLastError_f_impl(DWORD dwErrCode);
#ifndef DEFINED_kwlist__dwErrCode
#define DEFINED_kwlist__dwErrCode
PRIVATE DEFINE_KWLIST(kwlist__dwErrCode, { KEX("dwErrCode", 0x21a86465, 0x31f35d28a4a3ee44), KEND });
#endif /* !DEFINED_kwlist__dwErrCode */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_SetLastError_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DWORD dwErrCode;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__dwErrCode, UNPu32 ":SetLastError", &args))
		goto err;
	return libwin32_SetLastError_f_impl(args.dwErrCode);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_SetLastError, &libwin32_SetLastError_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_SetLastError_f_impl(DWORD dwErrCode)
/*[[[end]]]*/
{
	SetLastError(dwErrCode);
	return_none;
}

/*[[[deemon import("rt.gen.dexutils").gw("CloseHandle", "hObject:nt:HANDLE" ERROR_OR_BOOL); ]]]*/
#define LIBWIN32_CLOSEHANDLE_DEF          DEX_MEMBER_F("CloseHandle", &libwin32_CloseHandle, DEXSYM_READONLY, "(hObject:?X3?Dint?DFile?Ewin32:HANDLE)"),
#define LIBWIN32_CLOSEHANDLE_DEF_DOC(doc) DEX_MEMBER_F("CloseHandle", &libwin32_CloseHandle, DEXSYM_READONLY, "(hObject:?X3?Dint?DFile?Ewin32:HANDLE)\n" doc),
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_CloseHandle_f_impl(HANDLE hObject);
#ifndef DEFINED_kwlist__hObject
#define DEFINED_kwlist__hObject
PRIVATE DEFINE_KWLIST(kwlist__hObject, { KEX("hObject", 0xa4f9501a, 0x52389b26f803592d), KEND });
#endif /* !DEFINED_kwlist__hObject */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_CloseHandle_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *raw_hObject;
	} args;
	HANDLE hObject;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__hObject, "o:CloseHandle", &args))
		goto err;
	if unlikely(DeeNTSystem_TryGetHandle(args.raw_hObject, (void **)&hObject))
		goto err;
	return libwin32_CloseHandle_f_impl(hObject);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_CloseHandle, &libwin32_CloseHandle_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_CloseHandle_f_impl(HANDLE hObject)
/*[[[end]]]*/
{
	BOOL bResult;
again:
	DBG_ALIGNMENT_DISABLE();
	bResult = CloseHandle(hObject);
	if (!bResult) {
		DWORD dwError;
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(dwError)) {
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		RETURN_ERROR_OR_FALSE(dwError,
		                      "Failed to close handle %p",
		                      hObject);
	}
	DBG_ALIGNMENT_ENABLE();
	RETURN_SUCCESS_OR_TRUE;
err:
	return NULL;
}

/*[[[deemon import("rt.gen.dexutils").gw("DuplicateHandle",
      "hSourceProcessHandle:nt:HANDLE"
     ",hSourceHandle:nt:HANDLE"
     ",hTargetProcessHandle:nt:HANDLE"
     ",dwDesiredAccess:nt:DWORD=0"
     ",bInheritHandle:c:bool=true"
     ",dwOptions:nt:DWORD=DUPLICATE_SAME_ACCESS=!GDUPLICATE_SAME_ACCESS"
     "->" MAYBE_NONE("?GHANDLE")
     ); ]]]*/
#define LIBWIN32_DUPLICATEHANDLE_DEF          DEX_MEMBER_F("DuplicateHandle", &libwin32_DuplicateHandle, DEXSYM_READONLY, "(hSourceProcessHandle:?X3?Dint?DFile?Ewin32:HANDLE,hSourceHandle:?X3?Dint?DFile?Ewin32:HANDLE,hTargetProcessHandle:?X3?Dint?DFile?Ewin32:HANDLE,dwDesiredAccess=!0,bInheritHandle=!t,dwOptions:?Dint=!GDUPLICATE_SAME_ACCESS)->?GHANDLE"),
#define LIBWIN32_DUPLICATEHANDLE_DEF_DOC(doc) DEX_MEMBER_F("DuplicateHandle", &libwin32_DuplicateHandle, DEXSYM_READONLY, "(hSourceProcessHandle:?X3?Dint?DFile?Ewin32:HANDLE,hSourceHandle:?X3?Dint?DFile?Ewin32:HANDLE,hTargetProcessHandle:?X3?Dint?DFile?Ewin32:HANDLE,dwDesiredAccess=!0,bInheritHandle=!t,dwOptions:?Dint=!GDUPLICATE_SAME_ACCESS)->?GHANDLE\n" doc),
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_DuplicateHandle_f_impl(HANDLE hSourceProcessHandle, HANDLE hSourceHandle, HANDLE hTargetProcessHandle, DWORD dwDesiredAccess, bool bInheritHandle, DWORD dwOptions);
#ifndef DEFINED_kwlist__hSourceProcessHandle_hSourceHandle_hTargetProcessHandle_dwDesiredAccess_bInheritHandle_dwOptions
#define DEFINED_kwlist__hSourceProcessHandle_hSourceHandle_hTargetProcessHandle_dwDesiredAccess_bInheritHandle_dwOptions
PRIVATE DEFINE_KWLIST(kwlist__hSourceProcessHandle_hSourceHandle_hTargetProcessHandle_dwDesiredAccess_bInheritHandle_dwOptions, { KEX("hSourceProcessHandle", 0x15959012, 0x6c94a47c683276a), KEX("hSourceHandle", 0xa91a4ec5, 0x9e152391b4209159), KEX("hTargetProcessHandle", 0x4bed2e66, 0x2ffd7723cd6ebc81), KEX("dwDesiredAccess", 0xa4c2866d, 0x5a4b46d72691f2f), KEX("bInheritHandle", 0x632313fa, 0x9e7213d20a3bbbb4), KEX("dwOptions", 0x7d14185f, 0x6ff6f73a4db63965), KEND });
#endif /* !DEFINED_kwlist__hSourceProcessHandle_hSourceHandle_hTargetProcessHandle_dwDesiredAccess_bInheritHandle_dwOptions */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_DuplicateHandle_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *raw_hSourceProcessHandle;
		DeeObject *raw_hSourceHandle;
		DeeObject *raw_hTargetProcessHandle;
		DWORD dwDesiredAccess;
		bool bInheritHandle;
		DWORD dwOptions;
	} args;
	HANDLE hSourceProcessHandle;
	HANDLE hSourceHandle;
	HANDLE hTargetProcessHandle;
	args.dwDesiredAccess = 0;
	args.bInheritHandle = true;
	args.dwOptions = DUPLICATE_SAME_ACCESS;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__hSourceProcessHandle_hSourceHandle_hTargetProcessHandle_dwDesiredAccess_bInheritHandle_dwOptions, "ooo|" UNPu32 "b" UNPu32 ":DuplicateHandle", &args))
		goto err;
	if unlikely(DeeNTSystem_TryGetHandle(args.raw_hSourceProcessHandle, (void **)&hSourceProcessHandle))
		goto err;
	if unlikely(DeeNTSystem_TryGetHandle(args.raw_hSourceHandle, (void **)&hSourceHandle))
		goto err;
	if unlikely(DeeNTSystem_TryGetHandle(args.raw_hTargetProcessHandle, (void **)&hTargetProcessHandle))
		goto err;
	return libwin32_DuplicateHandle_f_impl(hSourceProcessHandle, hSourceHandle, hTargetProcessHandle, args.dwDesiredAccess, args.bInheritHandle, args.dwOptions);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_DuplicateHandle, &libwin32_DuplicateHandle_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_DuplicateHandle_f_impl(HANDLE hSourceProcessHandle, HANDLE hSourceHandle, HANDLE hTargetProcessHandle, DWORD dwDesiredAccess, bool bInheritHandle, DWORD dwOptions)
/*[[[end]]]*/
{
	HANDLE hResult;
	DREF DeeObject *result;
again:
	DBG_ALIGNMENT_DISABLE();
	if (!DuplicateHandle(hSourceProcessHandle,
	                     hSourceHandle,
	                     hTargetProcessHandle,
	                     &hResult,
	                     dwDesiredAccess,
	                     bInheritHandle,
	                     dwOptions)) {
		DWORD dwError;
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(dwError)) {
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		RETURN_ERROR(dwError,
		             "Failed to duplicate handle "
		             "(hSourceProcessHandle: %p, "
		             "hSourceHandle: %p, "
		             "hTargetProcessHandle: %p, "
		             "dwDesiredAccess: %#" PRFx32 ", "
		             "bInheritHandle: %d, "
		             "dwOptions: %#" PRFx32 ")",
		             hSourceProcessHandle, hSourceHandle,
		             hTargetProcessHandle, dwDesiredAccess,
		             bInheritHandle, dwOptions);
	}
	DBG_ALIGNMENT_ENABLE();
	result = libwin32_CreateHandle(hResult);
	if unlikely(!result)
		CloseHandle(hResult);
	return result;
err:
	return NULL;
}

/*[[[deemon import("rt.gen.dexutils").gw("CreateFile", "
	lpFileName:?Dstring;
	dwDesiredAccess:nt:DWORD=FILE_GENERIC_READ=!GFILE_GENERIC_READ;
	dwShareMode:nt:DWORD
	= FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE
	= !|!GFILE_SHARE_READ!|!GFILE_SHARE_WRITE!GFILE_SHARE_DELETE;
	lpSecurityAttributes?:?GSECURITY_ATTRIBUTES=NULL;
	dwCreationDisposition:nt:DWORD=OPEN_EXISTING=!GOPEN_EXISTING;
	dwFlagsAndAttributes:nt:DWORD=FILE_ATTRIBUTE_NORMAL=!GFILE_ATTRIBUTE_NORMAL;
	hTemplateFile?:nt:HANDLE;
->" MAYBE_NONE("?GHANDLE")); ]]]*/
#define LIBWIN32_CREATEFILE_DEF          DEX_MEMBER_F("CreateFile", &libwin32_CreateFile, DEXSYM_READONLY, "(lpFileName:?Dstring,dwDesiredAccess:?Dint=!GFILE_GENERIC_READ,dwShareMode:?Dint=!|!GFILE_SHARE_READ!|!GFILE_SHARE_WRITE!GFILE_SHARE_DELETE,lpSecurityAttributes?:?GSECURITY_ATTRIBUTES,dwCreationDisposition:?Dint=!GOPEN_EXISTING,dwFlagsAndAttributes:?Dint=!GFILE_ATTRIBUTE_NORMAL,hTemplateFile?:?X3?Dint?DFile?Ewin32:HANDLE)->?GHANDLE"),
#define LIBWIN32_CREATEFILE_DEF_DOC(doc) DEX_MEMBER_F("CreateFile", &libwin32_CreateFile, DEXSYM_READONLY, "(lpFileName:?Dstring,dwDesiredAccess:?Dint=!GFILE_GENERIC_READ,dwShareMode:?Dint=!|!GFILE_SHARE_READ!|!GFILE_SHARE_WRITE!GFILE_SHARE_DELETE,lpSecurityAttributes?:?GSECURITY_ATTRIBUTES,dwCreationDisposition:?Dint=!GOPEN_EXISTING,dwFlagsAndAttributes:?Dint=!GFILE_ATTRIBUTE_NORMAL,hTemplateFile?:?X3?Dint?DFile?Ewin32:HANDLE)->?GHANDLE\n" doc),
FORCELOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL libwin32_CreateFile_f_impl(DeeObject *lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, DeeObject *lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
#ifndef DEFINED_kwlist__lpFileName_dwDesiredAccess_dwShareMode_lpSecurityAttributes_dwCreationDisposition_dwFlagsAndAttributes_hTemplateFile
#define DEFINED_kwlist__lpFileName_dwDesiredAccess_dwShareMode_lpSecurityAttributes_dwCreationDisposition_dwFlagsAndAttributes_hTemplateFile
PRIVATE DEFINE_KWLIST(kwlist__lpFileName_dwDesiredAccess_dwShareMode_lpSecurityAttributes_dwCreationDisposition_dwFlagsAndAttributes_hTemplateFile, { KEX("lpFileName", 0xff8494c3, 0xcc8ee1226b2e55fb), KEX("dwDesiredAccess", 0xa4c2866d, 0x5a4b46d72691f2f), KEX("dwShareMode", 0x98b55f1b, 0x1150ef87f0ed3c58), KEX("lpSecurityAttributes", 0xbefe38d5, 0x2a65624101bba401), KEX("dwCreationDisposition", 0x6919a034, 0x84af10b505e97941), KEX("dwFlagsAndAttributes", 0xc0280882, 0x99ece384d9eb8e11), KEX("hTemplateFile", 0xf18ab7b, 0xeb04aa0848a7765d), KEND });
#endif /* !DEFINED_kwlist__lpFileName_dwDesiredAccess_dwShareMode_lpSecurityAttributes_dwCreationDisposition_dwFlagsAndAttributes_hTemplateFile */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_CreateFile_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *lpFileName;
		DWORD dwDesiredAccess;
		DWORD dwShareMode;
		DeeObject *lpSecurityAttributes;
		DWORD dwCreationDisposition;
		DWORD dwFlagsAndAttributes;
		DeeObject *raw_hTemplateFile;
	} args;
	HANDLE hTemplateFile = INVALID_HANDLE_VALUE;
	args.dwDesiredAccess = FILE_GENERIC_READ;
	args.dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;
	args.lpSecurityAttributes = NULL;
	args.dwCreationDisposition = OPEN_EXISTING;
	args.dwFlagsAndAttributes = FILE_ATTRIBUTE_NORMAL;
	args.raw_hTemplateFile = NULL;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__lpFileName_dwDesiredAccess_dwShareMode_lpSecurityAttributes_dwCreationDisposition_dwFlagsAndAttributes_hTemplateFile, "o|" UNPu32 UNPu32 "o" UNPu32 UNPu32 "o:CreateFile", &args))
		goto err;
	if (args.raw_hTemplateFile) {
		if unlikely(DeeNTSystem_TryGetHandle(args.raw_hTemplateFile, (void **)&hTemplateFile))
			goto err;
	}
	return libwin32_CreateFile_f_impl(args.lpFileName, args.dwDesiredAccess, args.dwShareMode, args.lpSecurityAttributes, args.dwCreationDisposition, args.dwFlagsAndAttributes, hTemplateFile);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_CreateFile, &libwin32_CreateFile_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL libwin32_CreateFile_f_impl(DeeObject *lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, DeeObject *lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
/*[[[end]]]*/
{
	HANDLE hResult;
	DREF DeeObject *result;
	(void)lpSecurityAttributes; /* TODO */
	hResult = DeeNTSystem_CreateFile(lpFileName,
	                                 dwDesiredAccess,
	                                 dwShareMode,
	                                 NULL,
	                                 dwCreationDisposition,
	                                 dwFlagsAndAttributes,
	                                 hTemplateFile);
	if unlikely(hResult == INVALID_HANDLE_VALUE) {
		DWORD dwError;
		dwError = GetLastError();
		RETURN_ERROR(dwError,
		             "Failed to open file %r (dwDesiredAccess: %#" PRFx32 ", dwShareMode: %#" PRFx32 ", "
		             "dwCreationDisposition: %#" PRFx32 ", dwFlagsAndAttributes: %#" PRFx32 ", hTemplateFile: %p)",
		             lpFileName, dwDesiredAccess, dwShareMode,
		             dwCreationDisposition, dwFlagsAndAttributes,
		             hTemplateFile);
	}
	result = libwin32_CreateHandle(hResult);
	if likely(result)
		return result;
	CloseHandle(hResult);
	return NULL;
}

/*[[[deemon import("rt.gen.dexutils").gw("WriteFile",
      "hFile:nt:HANDLE"
     ",lpBuffer:?DBytes"
     ",lpOverlapped:?GOVERLAPPED=NULL"
     "->" MAYBE_NONE("?Dint")
     ); ]]]*/
#define LIBWIN32_WRITEFILE_DEF          DEX_MEMBER_F("WriteFile", &libwin32_WriteFile, DEXSYM_READONLY, "(hFile:?X3?Dint?DFile?Ewin32:HANDLE,lpBuffer:?DBytes,lpOverlapped?:?GOVERLAPPED)->?Dint"),
#define LIBWIN32_WRITEFILE_DEF_DOC(doc) DEX_MEMBER_F("WriteFile", &libwin32_WriteFile, DEXSYM_READONLY, "(hFile:?X3?Dint?DFile?Ewin32:HANDLE,lpBuffer:?DBytes,lpOverlapped?:?GOVERLAPPED)->?Dint\n" doc),
FORCELOCAL WUNUSED NONNULL((2)) DREF DeeObject *DCALL libwin32_WriteFile_f_impl(HANDLE hFile, DeeObject *lpBuffer, DeeObject *lpOverlapped);
#ifndef DEFINED_kwlist__hFile_lpBuffer_lpOverlapped
#define DEFINED_kwlist__hFile_lpBuffer_lpOverlapped
PRIVATE DEFINE_KWLIST(kwlist__hFile_lpBuffer_lpOverlapped, { KEX("hFile", 0x7bb582fb, 0x2d053fe88ad300ff), KEX("lpBuffer", 0x6c1894a0, 0xda993aa6a0fe0491), KEX("lpOverlapped", 0x4a4b3c74, 0x8ba06e2eef56092c), KEND });
#endif /* !DEFINED_kwlist__hFile_lpBuffer_lpOverlapped */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_WriteFile_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *raw_hFile;
		DeeObject *lpBuffer;
		DeeObject *lpOverlapped;
	} args;
	HANDLE hFile;
	args.lpOverlapped = NULL;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__hFile_lpBuffer_lpOverlapped, "oo|o:WriteFile", &args))
		goto err;
	if unlikely(DeeNTSystem_TryGetHandle(args.raw_hFile, (void **)&hFile))
		goto err;
	return libwin32_WriteFile_f_impl(hFile, args.lpBuffer, args.lpOverlapped);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_WriteFile, &libwin32_WriteFile_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((2)) DREF DeeObject *DCALL libwin32_WriteFile_f_impl(HANDLE hFile, DeeObject *lpBuffer, DeeObject *lpOverlapped)
/*[[[end]]]*/
{
	DREF DeeObject *result;
	DeeBuffer buffer;
	DWORD dwNumberOfBytesWritten;
	if (DeeObject_GetBuf(lpBuffer, &buffer, Dee_BUFFER_FREADONLY))
		goto err;
again:
	DBG_ALIGNMENT_DISABLE();
	(void)lpOverlapped; /* TODO */
	if (!WriteFile(hFile,
	               (LPVOID)buffer.bb_base,
	               (DWORD)buffer.bb_size,
	               &dwNumberOfBytesWritten,
	               NULL)) {
		DWORD dwError;
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(dwError)) {
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		DeeObject_PutBuf(lpBuffer, &buffer, Dee_BUFFER_FREADONLY);
		RETURN_ERROR(dwError, "Failed to write to file %p", hFile);
	}
	DBG_ALIGNMENT_ENABLE();
	result = DeeInt_NewUInt32((uint32_t)dwNumberOfBytesWritten);
	DeeObject_PutBuf(lpBuffer, &buffer, Dee_BUFFER_FREADONLY);
	return result;
err:
	return NULL;
}



/*[[[deemon import("rt.gen.dexutils").gw("ReadFile",
      "hFile:nt:HANDLE"
     ",lpBuffer:obj:buffer"
     ",lpOverlapped:?GOVERLAPPED=NULL"
     "->" MAYBE_NONE("?Dint")
     ); ]]]*/
#define LIBWIN32_READFILE_DEF          DEX_MEMBER_F("ReadFile", &libwin32_ReadFile, DEXSYM_READONLY, "(hFile:?X3?Dint?DFile?Ewin32:HANDLE,lpBuffer:?DBytes,lpOverlapped?:?GOVERLAPPED)->?Dint"),
#define LIBWIN32_READFILE_DEF_DOC(doc) DEX_MEMBER_F("ReadFile", &libwin32_ReadFile, DEXSYM_READONLY, "(hFile:?X3?Dint?DFile?Ewin32:HANDLE,lpBuffer:?DBytes,lpOverlapped?:?GOVERLAPPED)->?Dint\n" doc),
FORCELOCAL WUNUSED NONNULL((2)) DREF DeeObject *DCALL libwin32_ReadFile_f_impl(HANDLE hFile, DeeObject *lpBuffer, DeeObject *lpOverlapped);
#ifndef DEFINED_kwlist__hFile_lpBuffer_lpOverlapped
#define DEFINED_kwlist__hFile_lpBuffer_lpOverlapped
PRIVATE DEFINE_KWLIST(kwlist__hFile_lpBuffer_lpOverlapped, { KEX("hFile", 0x7bb582fb, 0x2d053fe88ad300ff), KEX("lpBuffer", 0x6c1894a0, 0xda993aa6a0fe0491), KEX("lpOverlapped", 0x4a4b3c74, 0x8ba06e2eef56092c), KEND });
#endif /* !DEFINED_kwlist__hFile_lpBuffer_lpOverlapped */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_ReadFile_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *raw_hFile;
		DeeObject *lpBuffer;
		DeeObject *lpOverlapped;
	} args;
	HANDLE hFile;
	args.lpOverlapped = NULL;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__hFile_lpBuffer_lpOverlapped, "oo|o:ReadFile", &args))
		goto err;
	if unlikely(DeeNTSystem_TryGetHandle(args.raw_hFile, (void **)&hFile))
		goto err;
	return libwin32_ReadFile_f_impl(hFile, args.lpBuffer, args.lpOverlapped);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_ReadFile, &libwin32_ReadFile_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((2)) DREF DeeObject *DCALL libwin32_ReadFile_f_impl(HANDLE hFile, DeeObject *lpBuffer, DeeObject *lpOverlapped)
/*[[[end]]]*/
{
	DREF DeeObject *result;
	DeeBuffer buffer;
	DWORD dwNumberOfBytesRead;
	if (DeeObject_GetBuf(lpBuffer, &buffer, Dee_BUFFER_FWRITABLE))
		goto err;
again:
	DBG_ALIGNMENT_DISABLE();
	(void)lpOverlapped; /* TODO */
	if (!ReadFile(hFile,
	              (LPVOID)buffer.bb_base,
	              (DWORD)buffer.bb_size,
	              &dwNumberOfBytesRead,
	              NULL)) {
		DWORD dwError;
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(dwError)) {
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		DeeObject_PutBuf(lpBuffer, &buffer, Dee_BUFFER_FWRITABLE);
		RETURN_ERROR(dwError, "Failed to read from file %p", hFile);
	}
	DBG_ALIGNMENT_ENABLE();
	result = DeeInt_NewUInt32((uint32_t)dwNumberOfBytesRead);
	DeeObject_PutBuf(lpBuffer, &buffer, Dee_BUFFER_FWRITABLE);
	return result;
err:
	return NULL;
}


/*[[[deemon import("rt.gen.dexutils").gw("CreateDirectory", "lpPathName:nt:LPCWSTR,lpSecurityAttributes:?GSECURITY_ATTRIBUTES=NULL" ERROR_OR_BOOL); ]]]*/
#define LIBWIN32_CREATEDIRECTORY_DEF          DEX_MEMBER_F("CreateDirectory", &libwin32_CreateDirectory, DEXSYM_READONLY, "(lpPathName:?Dstring,lpSecurityAttributes?:?GSECURITY_ATTRIBUTES)"),
#define LIBWIN32_CREATEDIRECTORY_DEF_DOC(doc) DEX_MEMBER_F("CreateDirectory", &libwin32_CreateDirectory, DEXSYM_READONLY, "(lpPathName:?Dstring,lpSecurityAttributes?:?GSECURITY_ATTRIBUTES)\n" doc),
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_CreateDirectory_f_impl(LPCWSTR lpPathName, DeeObject *lpSecurityAttributes);
#ifndef DEFINED_kwlist__lpPathName_lpSecurityAttributes
#define DEFINED_kwlist__lpPathName_lpSecurityAttributes
PRIVATE DEFINE_KWLIST(kwlist__lpPathName_lpSecurityAttributes, { KEX("lpPathName", 0xd75096, 0x8330c9be981d73b1), KEX("lpSecurityAttributes", 0xbefe38d5, 0x2a65624101bba401), KEND });
#endif /* !DEFINED_kwlist__lpPathName_lpSecurityAttributes */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_CreateDirectory_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		LPCWSTR lpPathName;
		DeeObject *lpSecurityAttributes;
	} args;
	args.lpSecurityAttributes = NULL;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__lpPathName_lpSecurityAttributes, "U16s|o:CreateDirectory", &args))
		goto err;
	return libwin32_CreateDirectory_f_impl(args.lpPathName, args.lpSecurityAttributes);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_CreateDirectory, &libwin32_CreateDirectory_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_CreateDirectory_f_impl(LPCWSTR lpPathName, DeeObject *lpSecurityAttributes)
/*[[[end]]]*/
{
	BOOL bResult;
again:
	DBG_ALIGNMENT_DISABLE();
	(void)lpSecurityAttributes; /* TODO */
	bResult = CreateDirectoryW(lpPathName, NULL);
	if (!bResult) {
		DWORD dwError;
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(dwError)) {
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		RETURN_ERROR_OR_FALSE(dwError,
		                      "Failed to create directory %lq",
		                      lpPathName);
	}
	DBG_ALIGNMENT_ENABLE();
	RETURN_SUCCESS_OR_TRUE;
err:
	return NULL;
}

/*[[[deemon import("rt.gen.dexutils").gw("RemoveDirectory", "lpPathName:nt:LPCWSTR" ERROR_OR_BOOL); ]]]*/
#define LIBWIN32_REMOVEDIRECTORY_DEF          DEX_MEMBER_F("RemoveDirectory", &libwin32_RemoveDirectory, DEXSYM_READONLY, "(lpPathName:?Dstring)"),
#define LIBWIN32_REMOVEDIRECTORY_DEF_DOC(doc) DEX_MEMBER_F("RemoveDirectory", &libwin32_RemoveDirectory, DEXSYM_READONLY, "(lpPathName:?Dstring)\n" doc),
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_RemoveDirectory_f_impl(LPCWSTR lpPathName);
#ifndef DEFINED_kwlist__lpPathName
#define DEFINED_kwlist__lpPathName
PRIVATE DEFINE_KWLIST(kwlist__lpPathName, { KEX("lpPathName", 0xd75096, 0x8330c9be981d73b1), KEND });
#endif /* !DEFINED_kwlist__lpPathName */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_RemoveDirectory_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		LPCWSTR lpPathName;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__lpPathName, "U16s:RemoveDirectory", &args))
		goto err;
	return libwin32_RemoveDirectory_f_impl(args.lpPathName);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_RemoveDirectory, &libwin32_RemoveDirectory_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_RemoveDirectory_f_impl(LPCWSTR lpPathName)
/*[[[end]]]*/
{
	BOOL bResult;
again:
	DBG_ALIGNMENT_DISABLE();
	bResult = RemoveDirectoryW(lpPathName);
	if (!bResult) {
		DWORD dwError;
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(dwError)) {
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		RETURN_ERROR_OR_FALSE(dwError,
		                      "Failed to remove directory %lq",
		                      lpPathName);
	}
	DBG_ALIGNMENT_ENABLE();
	RETURN_SUCCESS_OR_TRUE;
err:
	return NULL;
}

/*[[[deemon import("rt.gen.dexutils").gw("DeleteFile", "lpFileName:nt:LPCWSTR" ERROR_OR_BOOL); ]]]*/
#define LIBWIN32_DELETEFILE_DEF          DEX_MEMBER_F("DeleteFile", &libwin32_DeleteFile, DEXSYM_READONLY, "(lpFileName:?Dstring)"),
#define LIBWIN32_DELETEFILE_DEF_DOC(doc) DEX_MEMBER_F("DeleteFile", &libwin32_DeleteFile, DEXSYM_READONLY, "(lpFileName:?Dstring)\n" doc),
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_DeleteFile_f_impl(LPCWSTR lpFileName);
#ifndef DEFINED_kwlist__lpFileName
#define DEFINED_kwlist__lpFileName
PRIVATE DEFINE_KWLIST(kwlist__lpFileName, { KEX("lpFileName", 0xff8494c3, 0xcc8ee1226b2e55fb), KEND });
#endif /* !DEFINED_kwlist__lpFileName */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_DeleteFile_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		LPCWSTR lpFileName;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__lpFileName, "U16s:DeleteFile", &args))
		goto err;
	return libwin32_DeleteFile_f_impl(args.lpFileName);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_DeleteFile, &libwin32_DeleteFile_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_DeleteFile_f_impl(LPCWSTR lpFileName)
/*[[[end]]]*/
{
	BOOL bResult;
again:
	DBG_ALIGNMENT_DISABLE();
	bResult = DeleteFileW(lpFileName);
	if (!bResult) {
		DWORD dwError;
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(dwError)) {
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		RETURN_ERROR_OR_FALSE(dwError,
		                      "Failed to delete file %lq",
		                      lpFileName);
	}
	DBG_ALIGNMENT_ENABLE();
	RETURN_SUCCESS_OR_TRUE;
err:
	return NULL;
}

/*[[[deemon import("rt.gen.dexutils").gw("SetEndOfFile", "hFile:nt:HANDLE" ERROR_OR_BOOL); ]]]*/
#define LIBWIN32_SETENDOFFILE_DEF          DEX_MEMBER_F("SetEndOfFile", &libwin32_SetEndOfFile, DEXSYM_READONLY, "(hFile:?X3?Dint?DFile?Ewin32:HANDLE)"),
#define LIBWIN32_SETENDOFFILE_DEF_DOC(doc) DEX_MEMBER_F("SetEndOfFile", &libwin32_SetEndOfFile, DEXSYM_READONLY, "(hFile:?X3?Dint?DFile?Ewin32:HANDLE)\n" doc),
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_SetEndOfFile_f_impl(HANDLE hFile);
#ifndef DEFINED_kwlist__hFile
#define DEFINED_kwlist__hFile
PRIVATE DEFINE_KWLIST(kwlist__hFile, { KEX("hFile", 0x7bb582fb, 0x2d053fe88ad300ff), KEND });
#endif /* !DEFINED_kwlist__hFile */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_SetEndOfFile_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *raw_hFile;
	} args;
	HANDLE hFile;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__hFile, "o:SetEndOfFile", &args))
		goto err;
	if unlikely(DeeNTSystem_TryGetHandle(args.raw_hFile, (void **)&hFile))
		goto err;
	return libwin32_SetEndOfFile_f_impl(hFile);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_SetEndOfFile, &libwin32_SetEndOfFile_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_SetEndOfFile_f_impl(HANDLE hFile)
/*[[[end]]]*/
{
	BOOL bResult;
again:
	DBG_ALIGNMENT_DISABLE();
	bResult = SetEndOfFile(hFile);
	if (!bResult) {
		DWORD dwError;
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(dwError)) {
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		RETURN_ERROR_OR_FALSE(dwError,
		                      "Failed to truncate file %p",
		                      hFile);
	}
	DBG_ALIGNMENT_ENABLE();
	RETURN_SUCCESS_OR_TRUE;
err:
	return NULL;
}

/*[[[deemon import("rt.gen.dexutils").gw("SetFilePointer", "hFile:nt:HANDLE,lDistanceToMove:I64d,dwMoveMethod:nt:DWORD=FILE_BEGIN=!GFILE_BEGIN->?Dint"); ]]]*/
#define LIBWIN32_SETFILEPOINTER_DEF          DEX_MEMBER_F("SetFilePointer", &libwin32_SetFilePointer, DEXSYM_READONLY, "(hFile:?X3?Dint?DFile?Ewin32:HANDLE,lDistanceToMove:?Dint,dwMoveMethod:?Dint=!GFILE_BEGIN)->?Dint"),
#define LIBWIN32_SETFILEPOINTER_DEF_DOC(doc) DEX_MEMBER_F("SetFilePointer", &libwin32_SetFilePointer, DEXSYM_READONLY, "(hFile:?X3?Dint?DFile?Ewin32:HANDLE,lDistanceToMove:?Dint,dwMoveMethod:?Dint=!GFILE_BEGIN)->?Dint\n" doc),
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_SetFilePointer_f_impl(HANDLE hFile, int64_t lDistanceToMove, DWORD dwMoveMethod);
#ifndef DEFINED_kwlist__hFile_lDistanceToMove_dwMoveMethod
#define DEFINED_kwlist__hFile_lDistanceToMove_dwMoveMethod
PRIVATE DEFINE_KWLIST(kwlist__hFile_lDistanceToMove_dwMoveMethod, { KEX("hFile", 0x7bb582fb, 0x2d053fe88ad300ff), KEX("lDistanceToMove", 0xbee6d54d, 0xbe1443399eaed560), KEX("dwMoveMethod", 0x56632122, 0xdb0dce2fe61b3e08), KEND });
#endif /* !DEFINED_kwlist__hFile_lDistanceToMove_dwMoveMethod */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_SetFilePointer_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *raw_hFile;
		int64_t lDistanceToMove;
		DWORD dwMoveMethod;
	} args;
	HANDLE hFile;
	args.dwMoveMethod = FILE_BEGIN;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__hFile_lDistanceToMove_dwMoveMethod, "o" UNPd64 "|" UNPu32 ":SetFilePointer", &args))
		goto err;
	if unlikely(DeeNTSystem_TryGetHandle(args.raw_hFile, (void **)&hFile))
		goto err;
	return libwin32_SetFilePointer_f_impl(hFile, args.lDistanceToMove, args.dwMoveMethod);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_SetFilePointer, &libwin32_SetFilePointer_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_SetFilePointer_f_impl(HANDLE hFile, int64_t lDistanceToMove, DWORD dwMoveMethod)
/*[[[end]]]*/
{
	DWORD dwResult;
	LONG lDistanceToMoveLow;
	LONG lDistanceToMoveHigh;
again:
	lDistanceToMoveLow  = (LONG)(lDistanceToMove);
	lDistanceToMoveHigh = (LONG)(lDistanceToMove >> 32);
	DBG_ALIGNMENT_DISABLE();
	dwResult = SetFilePointer(hFile,
	                          lDistanceToMoveLow,
	                          &lDistanceToMoveHigh,
	                          dwMoveMethod);
	if unlikely(dwResult == INVALID_SET_FILE_POINTER) {
		DWORD dwError;
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (dwError != NO_ERROR) {
			if (DeeNTSystem_IsIntr(dwError)) {
				if (DeeThread_CheckInterrupt())
					goto err;
				goto again;
			}
			RETURN_ERROR(dwError,
			             "Failed to seek file %p (offset: %" PRFd64 ", whence: %" PRFu32 ")",
			             hFile, lDistanceToMove, dwMoveMethod);
		}
	}
	DBG_ALIGNMENT_ENABLE();
	return DeeInt_NewUInt64((uint64_t)(uint32_t)lDistanceToMoveHigh << 32 |
	                     (uint64_t)(uint32_t)dwResult);
err:
	return NULL;
}


typedef union {
	uint64_t u64;
	FILETIME ft;
} ALIGNED_FILETIME;


/*[[[deemon import("rt.gen.dexutils").gw("GetFileTime", "hFile:nt:HANDLE->" MAYBE_NONE("?T3?Dint?Dint?Dint")); ]]]*/
#define LIBWIN32_GETFILETIME_DEF          DEX_MEMBER_F("GetFileTime", &libwin32_GetFileTime, DEXSYM_READONLY, "(hFile:?X3?Dint?DFile?Ewin32:HANDLE)->?T3?Dint?Dint?Dint"),
#define LIBWIN32_GETFILETIME_DEF_DOC(doc) DEX_MEMBER_F("GetFileTime", &libwin32_GetFileTime, DEXSYM_READONLY, "(hFile:?X3?Dint?DFile?Ewin32:HANDLE)->?T3?Dint?Dint?Dint\n" doc),
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetFileTime_f_impl(HANDLE hFile);
#ifndef DEFINED_kwlist__hFile
#define DEFINED_kwlist__hFile
PRIVATE DEFINE_KWLIST(kwlist__hFile, { KEX("hFile", 0x7bb582fb, 0x2d053fe88ad300ff), KEND });
#endif /* !DEFINED_kwlist__hFile */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetFileTime_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *raw_hFile;
	} args;
	HANDLE hFile;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__hFile, "o:GetFileTime", &args))
		goto err;
	if unlikely(DeeNTSystem_TryGetHandle(args.raw_hFile, (void **)&hFile))
		goto err;
	return libwin32_GetFileTime_f_impl(hFile);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_GetFileTime, &libwin32_GetFileTime_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetFileTime_f_impl(HANDLE hFile)
/*[[[end]]]*/
{
	BOOL bResult;
	ALIGNED_FILETIME ftCreationTime;
	ALIGNED_FILETIME ftLastAccessTime;
	ALIGNED_FILETIME ftLastWriteTime;
again:
	DBG_ALIGNMENT_DISABLE();
	bResult = GetFileTime(hFile,
	                      &ftCreationTime.ft,
	                      &ftLastAccessTime.ft,
	                      &ftLastWriteTime.ft);
	if (!bResult) {
		DWORD dwError;
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(dwError)) {
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		RETURN_ERROR(dwError,
		             "Failed to lookup file times of %p",
		             hFile);
	}
	DBG_ALIGNMENT_ENABLE();
	return DeeTuple_Newf(PCKu64 PCKu64 PCKu64,
	                     ftCreationTime.u64,
	                     ftLastAccessTime.u64,
	                     ftLastWriteTime.u64);
err:
	return NULL;
}



/*[[[deemon import("rt.gen.dexutils").gw("SetFileTime", "hFile:nt:HANDLE,lpCreationTime:?Dint=NULL,lpLastAccessTime:?Dint=NULL,lpLastWriteTime:?Dint=NULL" ERROR_OR_BOOL); ]]]*/
#define LIBWIN32_SETFILETIME_DEF          DEX_MEMBER_F("SetFileTime", &libwin32_SetFileTime, DEXSYM_READONLY, "(hFile:?X3?Dint?DFile?Ewin32:HANDLE,lpCreationTime?:?Dint,lpLastAccessTime?:?Dint,lpLastWriteTime?:?Dint)"),
#define LIBWIN32_SETFILETIME_DEF_DOC(doc) DEX_MEMBER_F("SetFileTime", &libwin32_SetFileTime, DEXSYM_READONLY, "(hFile:?X3?Dint?DFile?Ewin32:HANDLE,lpCreationTime?:?Dint,lpLastAccessTime?:?Dint,lpLastWriteTime?:?Dint)\n" doc),
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_SetFileTime_f_impl(HANDLE hFile, DeeObject *lpCreationTime, DeeObject *lpLastAccessTime, DeeObject *lpLastWriteTime);
#ifndef DEFINED_kwlist__hFile_lpCreationTime_lpLastAccessTime_lpLastWriteTime
#define DEFINED_kwlist__hFile_lpCreationTime_lpLastAccessTime_lpLastWriteTime
PRIVATE DEFINE_KWLIST(kwlist__hFile_lpCreationTime_lpLastAccessTime_lpLastWriteTime, { KEX("hFile", 0x7bb582fb, 0x2d053fe88ad300ff), KEX("lpCreationTime", 0x7538a959, 0x6df3d5c5c9ffd144), KEX("lpLastAccessTime", 0x80d1d92f, 0xf138b8b041892308), KEX("lpLastWriteTime", 0x27b207c7, 0x2428124aef8f3eb7), KEND });
#endif /* !DEFINED_kwlist__hFile_lpCreationTime_lpLastAccessTime_lpLastWriteTime */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_SetFileTime_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *raw_hFile;
		DeeObject *lpCreationTime;
		DeeObject *lpLastAccessTime;
		DeeObject *lpLastWriteTime;
	} args;
	HANDLE hFile;
	args.lpCreationTime = NULL;
	args.lpLastAccessTime = NULL;
	args.lpLastWriteTime = NULL;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__hFile_lpCreationTime_lpLastAccessTime_lpLastWriteTime, "o|ooo:SetFileTime", &args))
		goto err;
	if unlikely(DeeNTSystem_TryGetHandle(args.raw_hFile, (void **)&hFile))
		goto err;
	return libwin32_SetFileTime_f_impl(hFile, args.lpCreationTime, args.lpLastAccessTime, args.lpLastWriteTime);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_SetFileTime, &libwin32_SetFileTime_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_SetFileTime_f_impl(HANDLE hFile, DeeObject *lpCreationTime, DeeObject *lpLastAccessTime, DeeObject *lpLastWriteTime)
/*[[[end]]]*/
{
	ALIGNED_FILETIME ftCreationTime;
	ALIGNED_FILETIME ftLastAccessTime;
	ALIGNED_FILETIME ftLastWriteTime;
	BOOL bResult;
	if (lpCreationTime && DeeObject_AsUInt64(lpCreationTime, &ftCreationTime.u64))
		goto err;
	if (lpLastAccessTime && DeeObject_AsUInt64(lpLastAccessTime, &ftLastAccessTime.u64))
		goto err;
	if (lpLastWriteTime && DeeObject_AsUInt64(lpLastWriteTime, &ftLastWriteTime.u64))
		goto err;
again:
	DBG_ALIGNMENT_DISABLE();
	bResult = SetFileTime(hFile,
	                      lpCreationTime ? &ftCreationTime.ft : NULL,
	                      lpLastAccessTime ? &ftLastAccessTime.ft : NULL,
	                      lpLastWriteTime ? &ftLastWriteTime.ft : NULL);
	if (!bResult) {
		DWORD dwError;
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(dwError)) {
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		RETURN_ERROR_OR_FALSE(dwError,
		                      "Failed to set file times for %p",
		                      hFile);
	}
	DBG_ALIGNMENT_ENABLE();
	RETURN_SUCCESS_OR_TRUE;
err:
	return NULL;
}



/*[[[deemon import("rt.gen.dexutils").gw("SetFileValidData", "hFile:nt:HANDLE,ValidDataLength:I64u" ERROR_OR_BOOL); ]]]*/
#define LIBWIN32_SETFILEVALIDDATA_DEF          DEX_MEMBER_F("SetFileValidData", &libwin32_SetFileValidData, DEXSYM_READONLY, "(hFile:?X3?Dint?DFile?Ewin32:HANDLE,ValidDataLength:?Dint)"),
#define LIBWIN32_SETFILEVALIDDATA_DEF_DOC(doc) DEX_MEMBER_F("SetFileValidData", &libwin32_SetFileValidData, DEXSYM_READONLY, "(hFile:?X3?Dint?DFile?Ewin32:HANDLE,ValidDataLength:?Dint)\n" doc),
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_SetFileValidData_f_impl(HANDLE hFile, uint64_t ValidDataLength);
#ifndef DEFINED_kwlist__hFile_ValidDataLength
#define DEFINED_kwlist__hFile_ValidDataLength
PRIVATE DEFINE_KWLIST(kwlist__hFile_ValidDataLength, { KEX("hFile", 0x7bb582fb, 0x2d053fe88ad300ff), KEX("ValidDataLength", 0x80fa8e12, 0x7cb9d52ed20af79), KEND });
#endif /* !DEFINED_kwlist__hFile_ValidDataLength */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_SetFileValidData_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *raw_hFile;
		uint64_t ValidDataLength;
	} args;
	HANDLE hFile;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__hFile_ValidDataLength, "o" UNPu64 ":SetFileValidData", &args))
		goto err;
	if unlikely(DeeNTSystem_TryGetHandle(args.raw_hFile, (void **)&hFile))
		goto err;
	return libwin32_SetFileValidData_f_impl(hFile, args.ValidDataLength);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_SetFileValidData, &libwin32_SetFileValidData_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_SetFileValidData_f_impl(HANDLE hFile, uint64_t ValidDataLength)
/*[[[end]]]*/
{
	BOOL bResult;
again:
	DBG_ALIGNMENT_DISABLE();
	bResult = SetFileValidData(hFile, ValidDataLength);
	if (!bResult) {
		DWORD dwError;
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(dwError)) {
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		RETURN_ERROR_OR_FALSE(dwError,
		                      "Failed to set valid file data for %p to %" PRFu64,
		                      hFile, ValidDataLength);
	}
	DBG_ALIGNMENT_ENABLE();
	RETURN_SUCCESS_OR_TRUE;
err:
	return NULL;
}


/*[[[deemon import("rt.gen.dexutils").gw("GetTempPath", "->" MAYBE_NONE("?Dstring")); ]]]*/
#define LIBWIN32_GETTEMPPATH_DEF          DEX_MEMBER_F("GetTempPath", &libwin32_GetTempPath, DEXSYM_READONLY, "->?Dstring"),
#define LIBWIN32_GETTEMPPATH_DEF_DOC(doc) DEX_MEMBER_F("GetTempPath", &libwin32_GetTempPath, DEXSYM_READONLY, "->?Dstring\n" doc),
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetTempPath_f_impl(void);
PRIVATE DEFINE_CMETHOD0(libwin32_GetTempPath, &libwin32_GetTempPath_f_impl, METHOD_FNORMAL);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetTempPath_f_impl(void)
/*[[[end]]]*/
{
	LPWSTR lpBuffer, lpNewBuffer;
	DWORD dwBufSize = PATH_MAX, dwError;
	lpBuffer        = DeeString_NewWideBuffer(dwBufSize);
	if unlikely(!lpBuffer)
		goto err;
again:
	for (;;) {
		DBG_ALIGNMENT_DISABLE();
		dwError = GetTempPathW(dwBufSize + 1, lpBuffer);
		if (!dwError) {
			dwError = GetLastError();
			DBG_ALIGNMENT_ENABLE();
			if (DeeNTSystem_IsIntr(dwError)) {
				if (DeeThread_CheckInterrupt())
					goto err_result;
				goto again;
			}
			if (dwError != NO_ERROR) {
				DeeString_FreeWideBuffer(lpBuffer);
				RETURN_ERROR(dwError, "Failed to determine TEMPPATH");
			}
		} else {
			DBG_ALIGNMENT_ENABLE();
		}
		if (dwError <= dwBufSize)
			break;
		/* Resize to fit. */
		lpNewBuffer = DeeString_ResizeWideBuffer(lpBuffer, dwError);
		if unlikely(!lpNewBuffer)
			goto err_result;
		lpBuffer  = lpNewBuffer;
		dwBufSize = dwError;
	}
	lpBuffer = DeeString_TruncateWideBuffer(lpBuffer, dwError);
	return DeeString_PackWideBuffer(lpBuffer, STRING_ERROR_FREPLAC);
err_result:
	DeeString_FreeWideBuffer(lpBuffer);
err:
	return NULL;
}

/*[[[deemon import("rt.gen.dexutils").gw("GetDllDirectory", "->" MAYBE_NONE("?Dstring")); ]]]*/
#define LIBWIN32_GETDLLDIRECTORY_DEF          DEX_MEMBER_F("GetDllDirectory", &libwin32_GetDllDirectory, DEXSYM_READONLY, "->?Dstring"),
#define LIBWIN32_GETDLLDIRECTORY_DEF_DOC(doc) DEX_MEMBER_F("GetDllDirectory", &libwin32_GetDllDirectory, DEXSYM_READONLY, "->?Dstring\n" doc),
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetDllDirectory_f_impl(void);
PRIVATE DEFINE_CMETHOD0(libwin32_GetDllDirectory, &libwin32_GetDllDirectory_f_impl, METHOD_FNORMAL);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetDllDirectory_f_impl(void)
/*[[[end]]]*/
{
	LPWSTR lpBuffer, lpNewBuffer;
	DWORD dwBufSize = PATH_MAX, dwError;
	typedef DWORD (WINAPI *LPGETDLLDIRECTORYW)(DWORD nBufferLength, LPWSTR lpBuffer);
	PRIVATE LPGETDLLDIRECTORYW pGetDllDirectoryW = NULL;
	if (pGetDllDirectoryW == NULL) {
		LPGETDLLDIRECTORYW new_pGetDllDirectoryW;
		DBG_ALIGNMENT_DISABLE();
		new_pGetDllDirectoryW = (LPGETDLLDIRECTORYW)GetProcAddress(GetModuleHandle(TEXT("KERNEL32")),
		                                                           "GetDllDirectoryW");
		DBG_ALIGNMENT_ENABLE();
		if (new_pGetDllDirectoryW == NULL)
			new_pGetDllDirectoryW = (LPGETDLLDIRECTORYW)(void *)(uintptr_t)-1;
		pGetDllDirectoryW = new_pGetDllDirectoryW;
	}
	if (pGetDllDirectoryW == (LPGETDLLDIRECTORYW)(void *)(uintptr_t)-1) {
		SetLastError(ERROR_INVALID_FUNCTION);
		RETURN_ERROR(ERROR_INVALID_FUNCTION,
		             "Unsupported function `GetDllDirectoryW()'");
	}

	lpBuffer = DeeString_NewWideBuffer(dwBufSize);
	if unlikely(!lpBuffer)
		goto err;
again:
	for (;;) {
		DBG_ALIGNMENT_DISABLE();
		dwError = (*pGetDllDirectoryW)(dwBufSize + 1, lpBuffer);
		if (!dwError) {
			dwError = GetLastError();
			DBG_ALIGNMENT_ENABLE();
			if (DeeNTSystem_IsIntr(dwError)) {
				if (DeeThread_CheckInterrupt())
					goto err_result;
				goto again;
			}
			if (dwError != NO_ERROR) {
				DeeString_FreeWideBuffer(lpBuffer);
				RETURN_ERROR(dwError, "Failed to determine the DLL Directory");
			}
		} else {
			DBG_ALIGNMENT_ENABLE();
		}
		if (dwError <= dwBufSize)
			break;
		/* Resize to fit. */
		lpNewBuffer = DeeString_ResizeWideBuffer(lpBuffer, dwError);
		if unlikely(!lpNewBuffer)
			goto err_result;
		lpBuffer  = lpNewBuffer;
		dwBufSize = dwError;
	}
	lpBuffer = DeeString_TruncateWideBuffer(lpBuffer, dwError);
	return DeeString_PackWideBuffer(lpBuffer, STRING_ERROR_FREPLAC);
err_result:
	DeeString_FreeWideBuffer(lpBuffer);
err:
	return NULL;
}

/*[[[deemon import("rt.gen.dexutils").gw("SetDllDirectory", "lpPathName:nt:LPCWSTR" ERROR_OR_BOOL); ]]]*/
#define LIBWIN32_SETDLLDIRECTORY_DEF          DEX_MEMBER_F("SetDllDirectory", &libwin32_SetDllDirectory, DEXSYM_READONLY, "(lpPathName:?Dstring)"),
#define LIBWIN32_SETDLLDIRECTORY_DEF_DOC(doc) DEX_MEMBER_F("SetDllDirectory", &libwin32_SetDllDirectory, DEXSYM_READONLY, "(lpPathName:?Dstring)\n" doc),
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_SetDllDirectory_f_impl(LPCWSTR lpPathName);
#ifndef DEFINED_kwlist__lpPathName
#define DEFINED_kwlist__lpPathName
PRIVATE DEFINE_KWLIST(kwlist__lpPathName, { KEX("lpPathName", 0xd75096, 0x8330c9be981d73b1), KEND });
#endif /* !DEFINED_kwlist__lpPathName */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_SetDllDirectory_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		LPCWSTR lpPathName;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__lpPathName, "U16s:SetDllDirectory", &args))
		goto err;
	return libwin32_SetDllDirectory_f_impl(args.lpPathName);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_SetDllDirectory, &libwin32_SetDllDirectory_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_SetDllDirectory_f_impl(LPCWSTR lpPathName)
/*[[[end]]]*/
{
	BOOL bResult;
	typedef BOOL (WINAPI *LPSETDLLDIRECTORYW)(LPCWSTR lpPathName);
	PRIVATE LPSETDLLDIRECTORYW pSetDllDirectoryW = NULL;
	if (pSetDllDirectoryW == NULL) {
		LPSETDLLDIRECTORYW new_pSetDllDirectoryW;
		DBG_ALIGNMENT_DISABLE();
		new_pSetDllDirectoryW = (LPSETDLLDIRECTORYW)GetProcAddress(GetModuleHandle(TEXT("KERNEL32")),
		                                                           "SetDllDirectoryW");
		DBG_ALIGNMENT_ENABLE();
		if (new_pSetDllDirectoryW == NULL)
			new_pSetDllDirectoryW = (LPSETDLLDIRECTORYW)(void *)(uintptr_t)-1;
		pSetDllDirectoryW = new_pSetDllDirectoryW;
	}
	if (pSetDllDirectoryW == (LPSETDLLDIRECTORYW)(void *)(uintptr_t)-1) {
		SetLastError(ERROR_INVALID_FUNCTION);
		RETURN_ERROR(ERROR_INVALID_FUNCTION,
		             "Unsupported function `SetDllDirectoryW()'");
	}
again:
	DBG_ALIGNMENT_DISABLE();
	bResult = (*pSetDllDirectoryW)(lpPathName);
	if (!bResult) {
		DWORD dwError;
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(dwError)) {
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		RETURN_ERROR_OR_FALSE(dwError,
		                      "Failed to set DLL Directory to %lq",
		                      lpPathName);
	}
	DBG_ALIGNMENT_ENABLE();
	RETURN_SUCCESS_OR_TRUE;
err:
	return NULL;
}

/*[[[deemon import("rt.gen.dexutils").gw("GetDiskFreeSpace", "lpRootPathName:nt:LPCWSTR->" MAYBE_NONE("?T4?Dint?Dint?Dint?Dint")); ]]]*/
#define LIBWIN32_GETDISKFREESPACE_DEF          DEX_MEMBER_F("GetDiskFreeSpace", &libwin32_GetDiskFreeSpace, DEXSYM_READONLY, "(lpRootPathName:?Dstring)->?T4?Dint?Dint?Dint?Dint"),
#define LIBWIN32_GETDISKFREESPACE_DEF_DOC(doc) DEX_MEMBER_F("GetDiskFreeSpace", &libwin32_GetDiskFreeSpace, DEXSYM_READONLY, "(lpRootPathName:?Dstring)->?T4?Dint?Dint?Dint?Dint\n" doc),
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetDiskFreeSpace_f_impl(LPCWSTR lpRootPathName);
#ifndef DEFINED_kwlist__lpRootPathName
#define DEFINED_kwlist__lpRootPathName
PRIVATE DEFINE_KWLIST(kwlist__lpRootPathName, { KEX("lpRootPathName", 0x7d669f09, 0x44db90b21da024d2), KEND });
#endif /* !DEFINED_kwlist__lpRootPathName */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetDiskFreeSpace_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		LPCWSTR lpRootPathName;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__lpRootPathName, "U16s:GetDiskFreeSpace", &args))
		goto err;
	return libwin32_GetDiskFreeSpace_f_impl(args.lpRootPathName);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_GetDiskFreeSpace, &libwin32_GetDiskFreeSpace_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetDiskFreeSpace_f_impl(LPCWSTR lpRootPathName)
/*[[[end]]]*/
{
	BOOL bResult;
	DWORD dwSectorsPerCluster;
	DWORD dwBytesPerSector;
	DWORD dwNumberOfFreeClusters;
	DWORD dwTotalNumberOfClusters;
again:
	DBG_ALIGNMENT_DISABLE();
	bResult = GetDiskFreeSpaceW(lpRootPathName,
	                            &dwSectorsPerCluster,
	                            &dwBytesPerSector,
	                            &dwNumberOfFreeClusters,
	                            &dwTotalNumberOfClusters);
	if (!bResult) {
		DWORD dwError;
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(dwError)) {
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		RETURN_ERROR(dwError,
		             "Failed to determine free disk space for %lq",
		             lpRootPathName);
	}
	DBG_ALIGNMENT_ENABLE();
	return DeeTuple_Newf(PCKu32 PCKu32 PCKu32 PCKu32,
	                     dwSectorsPerCluster,
	                     dwBytesPerSector,
	                     dwNumberOfFreeClusters,
	                     dwTotalNumberOfClusters);
err:
	return NULL;
}



/*[[[deemon import("rt.gen.dexutils").gw("GetDiskFreeSpaceEx", "lpDirectoryName:nt:LPCWSTR->" MAYBE_NONE("?T3?Dint?Dint?Dint")); ]]]*/
#define LIBWIN32_GETDISKFREESPACEEX_DEF          DEX_MEMBER_F("GetDiskFreeSpaceEx", &libwin32_GetDiskFreeSpaceEx, DEXSYM_READONLY, "(lpDirectoryName:?Dstring)->?T3?Dint?Dint?Dint"),
#define LIBWIN32_GETDISKFREESPACEEX_DEF_DOC(doc) DEX_MEMBER_F("GetDiskFreeSpaceEx", &libwin32_GetDiskFreeSpaceEx, DEXSYM_READONLY, "(lpDirectoryName:?Dstring)->?T3?Dint?Dint?Dint\n" doc),
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetDiskFreeSpaceEx_f_impl(LPCWSTR lpDirectoryName);
#ifndef DEFINED_kwlist__lpDirectoryName
#define DEFINED_kwlist__lpDirectoryName
PRIVATE DEFINE_KWLIST(kwlist__lpDirectoryName, { KEX("lpDirectoryName", 0xfaaed06c, 0x94529f4e5983d4e5), KEND });
#endif /* !DEFINED_kwlist__lpDirectoryName */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetDiskFreeSpaceEx_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		LPCWSTR lpDirectoryName;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__lpDirectoryName, "U16s:GetDiskFreeSpaceEx", &args))
		goto err;
	return libwin32_GetDiskFreeSpaceEx_f_impl(args.lpDirectoryName);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_GetDiskFreeSpaceEx, &libwin32_GetDiskFreeSpaceEx_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetDiskFreeSpaceEx_f_impl(LPCWSTR lpDirectoryName)
/*[[[end]]]*/
{
	BOOL bResult;
	ULARGE_INTEGER uFreeBytesAvailableToCaller;
	ULARGE_INTEGER uTotalNumberOfBytes;
	ULARGE_INTEGER uTotalNumberOfFreeBytes;
again:
	DBG_ALIGNMENT_DISABLE();
	bResult = GetDiskFreeSpaceExW(lpDirectoryName,
	                              &uFreeBytesAvailableToCaller,
	                              &uTotalNumberOfBytes,
	                              &uTotalNumberOfFreeBytes);
	if (!bResult) {
		DWORD dwError;
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(dwError)) {
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		RETURN_ERROR(dwError,
		             "Failed to determine free disk space for %lq",
		             lpDirectoryName);
	}
	DBG_ALIGNMENT_ENABLE();
	return DeeTuple_Newf(PCKu64 PCKu64 PCKu64,
	                     uFreeBytesAvailableToCaller.QuadPart,
	                     uTotalNumberOfBytes.QuadPart,
	                     uTotalNumberOfFreeBytes.QuadPart);
err:
	return NULL;
}


/*[[[deemon import("rt.gen.dexutils").gw("GetDriveType", "lpRootPathName:nt:LPCWSTR->" MAYBE_NONE("?Dint")); ]]]*/
#define LIBWIN32_GETDRIVETYPE_DEF          DEX_MEMBER_F("GetDriveType", &libwin32_GetDriveType, DEXSYM_READONLY, "(lpRootPathName:?Dstring)->?Dint"),
#define LIBWIN32_GETDRIVETYPE_DEF_DOC(doc) DEX_MEMBER_F("GetDriveType", &libwin32_GetDriveType, DEXSYM_READONLY, "(lpRootPathName:?Dstring)->?Dint\n" doc),
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetDriveType_f_impl(LPCWSTR lpRootPathName);
#ifndef DEFINED_kwlist__lpRootPathName
#define DEFINED_kwlist__lpRootPathName
PRIVATE DEFINE_KWLIST(kwlist__lpRootPathName, { KEX("lpRootPathName", 0x7d669f09, 0x44db90b21da024d2), KEND });
#endif /* !DEFINED_kwlist__lpRootPathName */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetDriveType_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		LPCWSTR lpRootPathName;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__lpRootPathName, "U16s:GetDriveType", &args))
		goto err;
	return libwin32_GetDriveType_f_impl(args.lpRootPathName);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_GetDriveType, &libwin32_GetDriveType_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetDriveType_f_impl(LPCWSTR lpRootPathName)
/*[[[end]]]*/
{
	UINT uResult;
	DWORD dwError;
again:
	DBG_ALIGNMENT_DISABLE();
	SetLastError(NO_ERROR);
	uResult = GetDriveTypeW(lpRootPathName);
	if (uResult == DRIVE_UNKNOWN) {
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (dwError != NO_ERROR) {
			if (DeeNTSystem_IsIntr(dwError)) {
				if (DeeThread_CheckInterrupt())
					goto err;
				goto again;
			}
			RETURN_ERROR(dwError,
			             "Failed to determine drive type of %lq",
			             lpRootPathName);
		}
	} else {
		DBG_ALIGNMENT_ENABLE();
	}
	return DeeInt_NewUInt(uResult);
err:
	return NULL;
}




/*[[[deemon import("rt.gen.dexutils").gw("GetModuleFileName", "hModule:nt:HANDLE->" MAYBE_NONE("?Dstring")); ]]]*/
#define LIBWIN32_GETMODULEFILENAME_DEF          DEX_MEMBER_F("GetModuleFileName", &libwin32_GetModuleFileName, DEXSYM_READONLY, "(hModule:?X3?Dint?DFile?Ewin32:HANDLE)->?Dstring"),
#define LIBWIN32_GETMODULEFILENAME_DEF_DOC(doc) DEX_MEMBER_F("GetModuleFileName", &libwin32_GetModuleFileName, DEXSYM_READONLY, "(hModule:?X3?Dint?DFile?Ewin32:HANDLE)->?Dstring\n" doc),
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetModuleFileName_f_impl(HANDLE hModule);
#ifndef DEFINED_kwlist__hModule
#define DEFINED_kwlist__hModule
PRIVATE DEFINE_KWLIST(kwlist__hModule, { KEX("hModule", 0xa2a7467f, 0x6caab631b175fa52), KEND });
#endif /* !DEFINED_kwlist__hModule */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetModuleFileName_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *raw_hModule;
	} args;
	HANDLE hModule;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__hModule, "o:GetModuleFileName", &args))
		goto err;
	if unlikely(DeeNTSystem_TryGetHandle(args.raw_hModule, (void **)&hModule))
		goto err;
	return libwin32_GetModuleFileName_f_impl(hModule);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_GetModuleFileName, &libwin32_GetModuleFileName_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetModuleFileName_f_impl(HANDLE hModule)
/*[[[end]]]*/
{
	LPWSTR lpBuffer, lpNewBuffer;
	DWORD dwBufSize = PATH_MAX, dwError;
	lpBuffer        = DeeString_NewWideBuffer(dwBufSize);
	if unlikely(!lpBuffer)
		goto err;
again:
	for (;;) {
		DBG_ALIGNMENT_DISABLE();
		dwError = GetModuleFileNameW((HMODULE)hModule, lpBuffer, dwBufSize + 1);
		if (!dwError) {
			dwError = GetLastError();
			DBG_ALIGNMENT_ENABLE();
			if (DeeNTSystem_IsIntr(dwError)) {
				if (DeeThread_CheckInterrupt())
					goto err_buffer;
				goto again;
			}
			if (DeeNTSystem_IsBufferTooSmall(dwError))
				goto do_increase_buffer;
			DeeString_FreeWideBuffer(lpBuffer);
			RETURN_ERROR(dwError,
			             "Failed to determine name of module %p",
			             hModule);
		}
		DBG_ALIGNMENT_ENABLE();
		if (dwError <= dwBufSize) {
			if (dwError < dwBufSize)
				break;
			DBG_ALIGNMENT_DISABLE();
			dwError = GetLastError();
			DBG_ALIGNMENT_ENABLE();
			if (!DeeNTSystem_IsBufferTooSmall(dwError))
				break;
		}
		/* Increase buffer size. */
do_increase_buffer:
		dwBufSize *= 2;
		lpNewBuffer = DeeString_ResizeWideBuffer(lpBuffer, dwBufSize);
		if unlikely(!lpNewBuffer)
			goto err_buffer;
		lpBuffer = lpNewBuffer;
	}
	lpBuffer = DeeString_TruncateWideBuffer(lpBuffer, dwError);
	return DeeString_PackWideBuffer(lpBuffer, STRING_ERROR_FREPLAC);
err_buffer:
	DeeString_FreeWideBuffer(lpBuffer);
err:
	return NULL;
}



/*[[[deemon import("rt.gen.dexutils").gw("GetSystemDirectory", "->" MAYBE_NONE("?Dstring")); ]]]*/
#define LIBWIN32_GETSYSTEMDIRECTORY_DEF          DEX_MEMBER_F("GetSystemDirectory", &libwin32_GetSystemDirectory, DEXSYM_READONLY, "->?Dstring"),
#define LIBWIN32_GETSYSTEMDIRECTORY_DEF_DOC(doc) DEX_MEMBER_F("GetSystemDirectory", &libwin32_GetSystemDirectory, DEXSYM_READONLY, "->?Dstring\n" doc),
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetSystemDirectory_f_impl(void);
PRIVATE DEFINE_CMETHOD0(libwin32_GetSystemDirectory, &libwin32_GetSystemDirectory_f_impl, METHOD_FNORMAL);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetSystemDirectory_f_impl(void)
/*[[[end]]]*/
{
	LPWSTR lpBuffer, lpNewBuffer;
	DWORD dwBufSize = PATH_MAX, dwError;
	lpBuffer        = DeeString_NewWideBuffer(dwBufSize);
	if unlikely(!lpBuffer)
		goto err;
again:
	for (;;) {
		DBG_ALIGNMENT_DISABLE();
		dwError = GetSystemDirectoryW(lpBuffer, dwBufSize + 1);
		if (!dwError) {
			dwError = GetLastError();
			DBG_ALIGNMENT_ENABLE();
			if (DeeNTSystem_IsIntr(dwError)) {
				if (DeeThread_CheckInterrupt())
					goto err_result;
				goto again;
			}
			if (dwError != NO_ERROR) {
				DeeString_FreeWideBuffer(lpBuffer);
				RETURN_ERROR(dwError, "Failed to determine the system directory");
			}
		} else {
			DBG_ALIGNMENT_ENABLE();
		}
		if (dwError <= dwBufSize)
			break;
		/* Resize to fit. */
		lpNewBuffer = DeeString_ResizeWideBuffer(lpBuffer, dwError);
		if unlikely(!lpNewBuffer)
			goto err_result;
		lpBuffer  = lpNewBuffer;
		dwBufSize = dwError;
	}
	lpBuffer = DeeString_TruncateWideBuffer(lpBuffer, dwError);
	return DeeString_PackWideBuffer(lpBuffer, STRING_ERROR_FREPLAC);
err_result:
	DeeString_FreeWideBuffer(lpBuffer);
err:
	return NULL;
}




/*[[[deemon import("rt.gen.dexutils").gw("GetWindowsDirectory", "->" MAYBE_NONE("?Dstring")); ]]]*/
#define LIBWIN32_GETWINDOWSDIRECTORY_DEF          DEX_MEMBER_F("GetWindowsDirectory", &libwin32_GetWindowsDirectory, DEXSYM_READONLY, "->?Dstring"),
#define LIBWIN32_GETWINDOWSDIRECTORY_DEF_DOC(doc) DEX_MEMBER_F("GetWindowsDirectory", &libwin32_GetWindowsDirectory, DEXSYM_READONLY, "->?Dstring\n" doc),
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetWindowsDirectory_f_impl(void);
PRIVATE DEFINE_CMETHOD0(libwin32_GetWindowsDirectory, &libwin32_GetWindowsDirectory_f_impl, METHOD_FNORMAL);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetWindowsDirectory_f_impl(void)
/*[[[end]]]*/
{
	LPWSTR lpBuffer, lpNewBuffer;
	DWORD dwBufSize = PATH_MAX, dwError;
	lpBuffer        = DeeString_NewWideBuffer(dwBufSize);
	if unlikely(!lpBuffer)
		goto err;
again:
	for (;;) {
		DBG_ALIGNMENT_DISABLE();
		dwError = GetWindowsDirectoryW(lpBuffer, dwBufSize + 1);
		if (!dwError) {
			dwError = GetLastError();
			DBG_ALIGNMENT_ENABLE();
			if (DeeNTSystem_IsIntr(dwError)) {
				if (DeeThread_CheckInterrupt())
					goto err_result;
				goto again;
			}
			if (dwError != NO_ERROR) {
				DeeString_FreeWideBuffer(lpBuffer);
				RETURN_ERROR(dwError, "Failed to determine the Windows directory");
			}
		} else {
			DBG_ALIGNMENT_ENABLE();
		}
		if (dwError <= dwBufSize)
			break;
		/* Resize to fit. */
		lpNewBuffer = DeeString_ResizeWideBuffer(lpBuffer, dwError);
		if unlikely(!lpNewBuffer)
			goto err_result;
		lpBuffer  = lpNewBuffer;
		dwBufSize = dwError;
	}
	lpBuffer = DeeString_TruncateWideBuffer(lpBuffer, dwError);
	return DeeString_PackWideBuffer(lpBuffer, STRING_ERROR_FREPLAC);
err_result:
	DeeString_FreeWideBuffer(lpBuffer);
err:
	return NULL;
}


/*[[[deemon import("rt.gen.dexutils").gw("GetSystemWindowsDirectory", "->" MAYBE_NONE("?Dstring")); ]]]*/
#define LIBWIN32_GETSYSTEMWINDOWSDIRECTORY_DEF          DEX_MEMBER_F("GetSystemWindowsDirectory", &libwin32_GetSystemWindowsDirectory, DEXSYM_READONLY, "->?Dstring"),
#define LIBWIN32_GETSYSTEMWINDOWSDIRECTORY_DEF_DOC(doc) DEX_MEMBER_F("GetSystemWindowsDirectory", &libwin32_GetSystemWindowsDirectory, DEXSYM_READONLY, "->?Dstring\n" doc),
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetSystemWindowsDirectory_f_impl(void);
PRIVATE DEFINE_CMETHOD0(libwin32_GetSystemWindowsDirectory, &libwin32_GetSystemWindowsDirectory_f_impl, METHOD_FNORMAL);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetSystemWindowsDirectory_f_impl(void)
/*[[[end]]]*/
{
	LPWSTR lpBuffer, lpNewBuffer;
	DWORD dwBufSize = PATH_MAX, dwError;
	lpBuffer        = DeeString_NewWideBuffer(dwBufSize);
	if unlikely(!lpBuffer)
		goto err;
again:
	for (;;) {
		DBG_ALIGNMENT_DISABLE();
		dwError = GetSystemWindowsDirectoryW(lpBuffer, dwBufSize + 1);
		if (!dwError) {
			dwError = GetLastError();
			DBG_ALIGNMENT_ENABLE();
			if (DeeNTSystem_IsIntr(dwError)) {
				if (DeeThread_CheckInterrupt())
					goto err_result;
				goto again;
			}
			if (dwError != NO_ERROR) {
				DeeString_FreeWideBuffer(lpBuffer);
				RETURN_ERROR(dwError, "Failed to determine the System-Windows directory");
			}
		} else {
			DBG_ALIGNMENT_ENABLE();
		}
		if (dwError <= dwBufSize)
			break;
		/* Resize to fit. */
		lpNewBuffer = DeeString_ResizeWideBuffer(lpBuffer, dwError);
		if unlikely(!lpNewBuffer)
			goto err_result;
		lpBuffer  = lpNewBuffer;
		dwBufSize = dwError;
	}
	lpBuffer = DeeString_TruncateWideBuffer(lpBuffer, dwError);
	return DeeString_PackWideBuffer(lpBuffer, STRING_ERROR_FREPLAC);
err_result:
	DeeString_FreeWideBuffer(lpBuffer);
err:
	return NULL;
}




/*[[[deemon import("rt.gen.dexutils").gw("GetSystemWow64Directory", "->" MAYBE_NONE("?Dstring")); ]]]*/
#define LIBWIN32_GETSYSTEMWOW64DIRECTORY_DEF          DEX_MEMBER_F("GetSystemWow64Directory", &libwin32_GetSystemWow64Directory, DEXSYM_READONLY, "->?Dstring"),
#define LIBWIN32_GETSYSTEMWOW64DIRECTORY_DEF_DOC(doc) DEX_MEMBER_F("GetSystemWow64Directory", &libwin32_GetSystemWow64Directory, DEXSYM_READONLY, "->?Dstring\n" doc),
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetSystemWow64Directory_f_impl(void);
PRIVATE DEFINE_CMETHOD0(libwin32_GetSystemWow64Directory, &libwin32_GetSystemWow64Directory_f_impl, METHOD_FNORMAL);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetSystemWow64Directory_f_impl(void)
/*[[[end]]]*/
{
	LPWSTR lpBuffer, lpNewBuffer;
	DWORD dwBufSize                                                 = PATH_MAX, dwError;
	PRIVATE PGET_SYSTEM_WOW64_DIRECTORY_W pGetSystemWow64DirectoryW = NULL;
	if (pGetSystemWow64DirectoryW == NULL) {
		PGET_SYSTEM_WOW64_DIRECTORY_W new_pGetSystemWow64DirectoryW;
		DBG_ALIGNMENT_DISABLE();
#ifndef GET_SYSTEM_WOW64_DIRECTORY_NAME_W_A
#define GET_SYSTEM_WOW64_DIRECTORY_NAME_W_A "GetSystemWow64DirectoryW"
#endif /* !GET_SYSTEM_WOW64_DIRECTORY_NAME_W_A */
		new_pGetSystemWow64DirectoryW = (PGET_SYSTEM_WOW64_DIRECTORY_W)GetProcAddress(GetModuleHandle(TEXT("KERNEL32")),
		                                                                              GET_SYSTEM_WOW64_DIRECTORY_NAME_W_A);
		DBG_ALIGNMENT_ENABLE();
		if (new_pGetSystemWow64DirectoryW == NULL)
			new_pGetSystemWow64DirectoryW = (PGET_SYSTEM_WOW64_DIRECTORY_W)(void *)(uintptr_t)-1;
		pGetSystemWow64DirectoryW = new_pGetSystemWow64DirectoryW;
	}
	if (pGetSystemWow64DirectoryW == (PGET_SYSTEM_WOW64_DIRECTORY_W)(void *)(uintptr_t)-1) {
		SetLastError(ERROR_INVALID_FUNCTION);
		RETURN_ERROR(ERROR_INVALID_FUNCTION,
		             "Unsupported function: `GetSystemWow64DirectoryW'");
	}

	lpBuffer = DeeString_NewWideBuffer(dwBufSize);
	if unlikely(!lpBuffer)
		goto err;
again:
	for (;;) {
		DBG_ALIGNMENT_DISABLE();
		dwError = (*pGetSystemWow64DirectoryW)(lpBuffer, dwBufSize + 1);
		if (!dwError) {
			dwError = GetLastError();
			DBG_ALIGNMENT_ENABLE();
			if (DeeNTSystem_IsIntr(dwError)) {
				if (DeeThread_CheckInterrupt())
					goto err_result;
				goto again;
			}
			if (dwError != NO_ERROR) {
				DeeString_FreeWideBuffer(lpBuffer);
				RETURN_ERROR(dwError, "Failed to determine the SystemWow64 directory");
			}
		} else {
			DBG_ALIGNMENT_ENABLE();
		}
		if (dwError <= dwBufSize)
			break;
		/* Resize to fit. */
		lpNewBuffer = DeeString_ResizeWideBuffer(lpBuffer, dwError);
		if unlikely(!lpNewBuffer)
			goto err_result;
		lpBuffer  = lpNewBuffer;
		dwBufSize = dwError;
	}
	lpBuffer = DeeString_TruncateWideBuffer(lpBuffer, dwError);
	return DeeString_PackWideBuffer(lpBuffer, STRING_ERROR_FREPLAC);
err_result:
	DeeString_FreeWideBuffer(lpBuffer);
err:
	return NULL;
}


/*[[[deemon (PRIVATE_DEFINE_STRING from rt.gen.string)("str_nul", "\0");]]]*/
PRIVATE DEFINE_STRING_EX(str_nul, "\0", 0x514e28b7, 0x0);
/*[[[end]]]*/

/* Split a given `str' at each instance of a NUL-character,
 * returning the sequence of resulting strings. */
PRIVATE WUNUSED DREF DeeObject *DCALL
split_nul_string(/*inherit(always)*/ DREF DeeObject *str) {
	DREF DeeObject *result;
	DeeObject *argv[1];
	if unlikely(!str)
		goto err;
	argv[0] = (DeeObject *)&str_nul;
	result  = DeeObject_CallAttrString(str, "split", 1, argv);
	Dee_Decref(str);
	return result;
err:
	return NULL;
}


/*[[[deemon import("rt.gen.dexutils").gw("GetLogicalDriveStrings", "->" MAYBE_NONE("?S?Dstring")); ]]]*/
#define LIBWIN32_GETLOGICALDRIVESTRINGS_DEF          DEX_MEMBER_F("GetLogicalDriveStrings", &libwin32_GetLogicalDriveStrings, DEXSYM_READONLY, "->?S?Dstring"),
#define LIBWIN32_GETLOGICALDRIVESTRINGS_DEF_DOC(doc) DEX_MEMBER_F("GetLogicalDriveStrings", &libwin32_GetLogicalDriveStrings, DEXSYM_READONLY, "->?S?Dstring\n" doc),
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetLogicalDriveStrings_f_impl(void);
PRIVATE DEFINE_CMETHOD0(libwin32_GetLogicalDriveStrings, &libwin32_GetLogicalDriveStrings_f_impl, METHOD_FNORMAL);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetLogicalDriveStrings_f_impl(void)
/*[[[end]]]*/
{
	LPWSTR lpBuffer, lpNewBuffer;
	DWORD dwBufSize = PATH_MAX, dwError;
	lpBuffer        = DeeString_NewWideBuffer(dwBufSize);
	if unlikely(!lpBuffer)
		goto err;
again:
	for (;;) {
		DBG_ALIGNMENT_DISABLE();
		dwError = GetLogicalDriveStringsW(dwBufSize + 1, lpBuffer);
		if (!dwError) {
			dwError = GetLastError();
			DBG_ALIGNMENT_ENABLE();
			if (DeeNTSystem_IsIntr(dwError)) {
				if (DeeThread_CheckInterrupt())
					goto err_result;
				goto again;
			}
			if (dwError != NO_ERROR) {
				DeeString_FreeWideBuffer(lpBuffer);
				RETURN_ERROR(dwError, "Failed to query logical drive strings");
			}
		} else {
			DBG_ALIGNMENT_ENABLE();
		}
		if (dwError <= dwBufSize)
			break;
		/* Resize to fit. */
		lpNewBuffer = DeeString_ResizeWideBuffer(lpBuffer, dwError);
		if unlikely(!lpNewBuffer)
			goto err_result;
		lpBuffer  = lpNewBuffer;
		dwBufSize = dwError;
	}
	/* Trim trailing NUL-characters */
	while (dwError && lpBuffer[dwError - 1] == '\0')
		--dwError;
	lpBuffer = DeeString_TruncateWideBuffer(lpBuffer, dwError);
	return split_nul_string(DeeString_PackWideBuffer(lpBuffer, STRING_ERROR_FREPLAC));
err_result:
	DeeString_FreeWideBuffer(lpBuffer);
err:
	return NULL;
}




/*[[[deemon import("rt.gen.dexutils").gw("QueryDosDevice", "lpDeviceName:nt:LPCWSTR->" MAYBE_NONE("?S?Dstring")); ]]]*/
#define LIBWIN32_QUERYDOSDEVICE_DEF          DEX_MEMBER_F("QueryDosDevice", &libwin32_QueryDosDevice, DEXSYM_READONLY, "(lpDeviceName:?Dstring)->?S?Dstring"),
#define LIBWIN32_QUERYDOSDEVICE_DEF_DOC(doc) DEX_MEMBER_F("QueryDosDevice", &libwin32_QueryDosDevice, DEXSYM_READONLY, "(lpDeviceName:?Dstring)->?S?Dstring\n" doc),
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_QueryDosDevice_f_impl(LPCWSTR lpDeviceName);
#ifndef DEFINED_kwlist__lpDeviceName
#define DEFINED_kwlist__lpDeviceName
PRIVATE DEFINE_KWLIST(kwlist__lpDeviceName, { KEX("lpDeviceName", 0xe02914a8, 0x889406e3ad17d98e), KEND });
#endif /* !DEFINED_kwlist__lpDeviceName */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_QueryDosDevice_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		LPCWSTR lpDeviceName;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__lpDeviceName, "U16s:QueryDosDevice", &args))
		goto err;
	return libwin32_QueryDosDevice_f_impl(args.lpDeviceName);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_QueryDosDevice, &libwin32_QueryDosDevice_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_QueryDosDevice_f_impl(LPCWSTR lpDeviceName)
/*[[[end]]]*/
{
	LPWSTR lpBuffer, lpNewBuffer;
	DWORD dwBufSize = PATH_MAX, dwError;
	lpBuffer        = DeeString_NewWideBuffer(dwBufSize);
	if unlikely(!lpBuffer)
		goto err;
again:
	for (;;) {
		DBG_ALIGNMENT_DISABLE();
		dwError = QueryDosDeviceW(lpDeviceName, lpBuffer, dwBufSize + 1);
		if (!dwError) {
			dwError = GetLastError();
			DBG_ALIGNMENT_ENABLE();
			if (DeeNTSystem_IsIntr(dwError)) {
				if (DeeThread_CheckInterrupt())
					goto err_buffer;
				goto again;
			}
			if (DeeNTSystem_IsBufferTooSmall(dwError))
				goto do_increase_buffer;
			DeeString_FreeWideBuffer(lpBuffer);
			RETURN_ERROR(dwError, "Failed to query the DOS devices for %lq", lpDeviceName);
		}
		DBG_ALIGNMENT_ENABLE();
		if (dwError <= dwBufSize) {
			if (dwError < dwBufSize)
				break;
			DBG_ALIGNMENT_DISABLE();
			dwError = GetLastError();
			DBG_ALIGNMENT_ENABLE();
			if (!DeeNTSystem_IsBufferTooSmall(dwError))
				break;
		}
		/* Increase buffer size. */
do_increase_buffer:
		dwBufSize *= 2;
		lpNewBuffer = DeeString_ResizeWideBuffer(lpBuffer, dwBufSize);
		if unlikely(!lpNewBuffer)
			goto err_buffer;
		lpBuffer = lpNewBuffer;
	}
	/* The last character is always a NUL */
	if (dwError)
		--dwError;
	/* Trim trailing NUL-characters */
	while (dwError && lpBuffer[dwError - 1] == '\0')
		--dwError;
	lpBuffer = DeeString_TruncateWideBuffer(lpBuffer, dwError);
	return split_nul_string(DeeString_PackWideBuffer(lpBuffer, STRING_ERROR_FREPLAC));
err_buffer:
	DeeString_FreeWideBuffer(lpBuffer);
err:
	return NULL;
}



/*[[[deemon import("rt.gen.dexutils").gw("GetFileType", "hFile:nt:HANDLE->" MAYBE_NONE("?Dint")); ]]]*/
#define LIBWIN32_GETFILETYPE_DEF          DEX_MEMBER_F("GetFileType", &libwin32_GetFileType, DEXSYM_READONLY, "(hFile:?X3?Dint?DFile?Ewin32:HANDLE)->?Dint"),
#define LIBWIN32_GETFILETYPE_DEF_DOC(doc) DEX_MEMBER_F("GetFileType", &libwin32_GetFileType, DEXSYM_READONLY, "(hFile:?X3?Dint?DFile?Ewin32:HANDLE)->?Dint\n" doc),
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetFileType_f_impl(HANDLE hFile);
#ifndef DEFINED_kwlist__hFile
#define DEFINED_kwlist__hFile
PRIVATE DEFINE_KWLIST(kwlist__hFile, { KEX("hFile", 0x7bb582fb, 0x2d053fe88ad300ff), KEND });
#endif /* !DEFINED_kwlist__hFile */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetFileType_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *raw_hFile;
	} args;
	HANDLE hFile;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__hFile, "o:GetFileType", &args))
		goto err;
	if unlikely(DeeNTSystem_TryGetHandle(args.raw_hFile, (void **)&hFile))
		goto err;
	return libwin32_GetFileType_f_impl(hFile);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_GetFileType, &libwin32_GetFileType_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetFileType_f_impl(HANDLE hFile)
/*[[[end]]]*/
{
	DWORD dwType;
again:
	DBG_ALIGNMENT_DISABLE();
	dwType = GetFileType(hFile);
	if (dwType == FILE_TYPE_UNKNOWN) {
		DWORD dwError;
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(dwError)) {
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		if (dwError != NO_ERROR)
			RETURN_ERROR(dwError, "Failed to determine the typing of handle %p", hFile);
	}
	DBG_ALIGNMENT_ENABLE();
	return DeeInt_NewUInt32((uint32_t)dwType);
err:
	return NULL;
}




/*[[[deemon import("rt.gen.dexutils").gw("GetFileSize", "hFile:nt:HANDLE->" MAYBE_NONE("?Dint")); ]]]*/
#define LIBWIN32_GETFILESIZE_DEF          DEX_MEMBER_F("GetFileSize", &libwin32_GetFileSize, DEXSYM_READONLY, "(hFile:?X3?Dint?DFile?Ewin32:HANDLE)->?Dint"),
#define LIBWIN32_GETFILESIZE_DEF_DOC(doc) DEX_MEMBER_F("GetFileSize", &libwin32_GetFileSize, DEXSYM_READONLY, "(hFile:?X3?Dint?DFile?Ewin32:HANDLE)->?Dint\n" doc),
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetFileSize_f_impl(HANDLE hFile);
#ifndef DEFINED_kwlist__hFile
#define DEFINED_kwlist__hFile
PRIVATE DEFINE_KWLIST(kwlist__hFile, { KEX("hFile", 0x7bb582fb, 0x2d053fe88ad300ff), KEND });
#endif /* !DEFINED_kwlist__hFile */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetFileSize_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *raw_hFile;
	} args;
	HANDLE hFile;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__hFile, "o:GetFileSize", &args))
		goto err;
	if unlikely(DeeNTSystem_TryGetHandle(args.raw_hFile, (void **)&hFile))
		goto err;
	return libwin32_GetFileSize_f_impl(hFile);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_GetFileSize, &libwin32_GetFileSize_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetFileSize_f_impl(HANDLE hFile)
/*[[[end]]]*/
{
	DWORD dwSizeLow, dwSizeHigh;
again:
	DBG_ALIGNMENT_DISABLE();
	dwSizeLow = GetFileSize(hFile, &dwSizeHigh);
	if (dwSizeLow == INVALID_FILE_SIZE) {
		DWORD dwError;
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(dwError)) {
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		if (dwError != NO_ERROR)
			RETURN_ERROR(dwError, "Failed to determine the size of file %p", hFile);
	}
	DBG_ALIGNMENT_ENABLE();
	return DeeInt_NewUInt64((uint64_t)dwSizeLow |
	                     (uint64_t)dwSizeHigh << 32);
err:
	return NULL;
}



/*[[[deemon import("rt.gen.dexutils").gw("GetFileAttributes", "lpFileName:nt:LPCWSTR->" MAYBE_NONE("?Dint")); ]]]*/
#define LIBWIN32_GETFILEATTRIBUTES_DEF          DEX_MEMBER_F("GetFileAttributes", &libwin32_GetFileAttributes, DEXSYM_READONLY, "(lpFileName:?Dstring)->?Dint"),
#define LIBWIN32_GETFILEATTRIBUTES_DEF_DOC(doc) DEX_MEMBER_F("GetFileAttributes", &libwin32_GetFileAttributes, DEXSYM_READONLY, "(lpFileName:?Dstring)->?Dint\n" doc),
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetFileAttributes_f_impl(LPCWSTR lpFileName);
#ifndef DEFINED_kwlist__lpFileName
#define DEFINED_kwlist__lpFileName
PRIVATE DEFINE_KWLIST(kwlist__lpFileName, { KEX("lpFileName", 0xff8494c3, 0xcc8ee1226b2e55fb), KEND });
#endif /* !DEFINED_kwlist__lpFileName */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetFileAttributes_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		LPCWSTR lpFileName;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__lpFileName, "U16s:GetFileAttributes", &args))
		goto err;
	return libwin32_GetFileAttributes_f_impl(args.lpFileName);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_GetFileAttributes, &libwin32_GetFileAttributes_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetFileAttributes_f_impl(LPCWSTR lpFileName)
/*[[[end]]]*/
{
	DWORD dwResult;
again:
	DBG_ALIGNMENT_DISABLE();
	dwResult = GetFileAttributesW(lpFileName);
	if (dwResult == INVALID_FILE_ATTRIBUTES) {
		DWORD dwError;
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(dwError)) {
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		if (dwError != NO_ERROR)
			RETURN_ERROR(dwError, "Failed to determine the attributes of %lq", lpFileName);
	}
	DBG_ALIGNMENT_ENABLE();
	return DeeInt_NewUInt32((uint32_t)dwResult);
err:
	return NULL;
}



/*[[[deemon import("rt.gen.dexutils").gw("SetFileAttributes", "lpFileName:nt:LPCWSTR,dwFileAttributes:nt:DWORD" ERROR_OR_BOOL); ]]]*/
#define LIBWIN32_SETFILEATTRIBUTES_DEF          DEX_MEMBER_F("SetFileAttributes", &libwin32_SetFileAttributes, DEXSYM_READONLY, "(lpFileName:?Dstring,dwFileAttributes:?Dint)"),
#define LIBWIN32_SETFILEATTRIBUTES_DEF_DOC(doc) DEX_MEMBER_F("SetFileAttributes", &libwin32_SetFileAttributes, DEXSYM_READONLY, "(lpFileName:?Dstring,dwFileAttributes:?Dint)\n" doc),
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_SetFileAttributes_f_impl(LPCWSTR lpFileName, DWORD dwFileAttributes);
#ifndef DEFINED_kwlist__lpFileName_dwFileAttributes
#define DEFINED_kwlist__lpFileName_dwFileAttributes
PRIVATE DEFINE_KWLIST(kwlist__lpFileName_dwFileAttributes, { KEX("lpFileName", 0xff8494c3, 0xcc8ee1226b2e55fb), KEX("dwFileAttributes", 0xe5241b8, 0x2e5dba7e6f75d939), KEND });
#endif /* !DEFINED_kwlist__lpFileName_dwFileAttributes */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_SetFileAttributes_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		LPCWSTR lpFileName;
		DWORD dwFileAttributes;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__lpFileName_dwFileAttributes, "U16s" UNPu32 ":SetFileAttributes", &args))
		goto err;
	return libwin32_SetFileAttributes_f_impl(args.lpFileName, args.dwFileAttributes);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_SetFileAttributes, &libwin32_SetFileAttributes_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_SetFileAttributes_f_impl(LPCWSTR lpFileName, DWORD dwFileAttributes)
/*[[[end]]]*/
{
	BOOL bResult;
again:
	DBG_ALIGNMENT_DISABLE();
	bResult = SetFileAttributesW(lpFileName,
	                             dwFileAttributes);
	if (!bResult) {
		DWORD dwError;
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(dwError)) {
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		RETURN_ERROR_OR_FALSE(dwError,
		                      "Failed to set attributes of file %lq to %#" PRFx32 "",
		                      lpFileName, dwFileAttributes);
	}
	DBG_ALIGNMENT_ENABLE();
	RETURN_SUCCESS_OR_TRUE;
err:
	return NULL;
}



/*[[[deemon import("rt.gen.dexutils").gw("GetCompressedFileSize", "lpFileName:nt:LPCWSTR->" MAYBE_NONE("?Dint")); ]]]*/
#define LIBWIN32_GETCOMPRESSEDFILESIZE_DEF          DEX_MEMBER_F("GetCompressedFileSize", &libwin32_GetCompressedFileSize, DEXSYM_READONLY, "(lpFileName:?Dstring)->?Dint"),
#define LIBWIN32_GETCOMPRESSEDFILESIZE_DEF_DOC(doc) DEX_MEMBER_F("GetCompressedFileSize", &libwin32_GetCompressedFileSize, DEXSYM_READONLY, "(lpFileName:?Dstring)->?Dint\n" doc),
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetCompressedFileSize_f_impl(LPCWSTR lpFileName);
#ifndef DEFINED_kwlist__lpFileName
#define DEFINED_kwlist__lpFileName
PRIVATE DEFINE_KWLIST(kwlist__lpFileName, { KEX("lpFileName", 0xff8494c3, 0xcc8ee1226b2e55fb), KEND });
#endif /* !DEFINED_kwlist__lpFileName */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetCompressedFileSize_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		LPCWSTR lpFileName;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__lpFileName, "U16s:GetCompressedFileSize", &args))
		goto err;
	return libwin32_GetCompressedFileSize_f_impl(args.lpFileName);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_GetCompressedFileSize, &libwin32_GetCompressedFileSize_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetCompressedFileSize_f_impl(LPCWSTR lpFileName)
/*[[[end]]]*/
{
	DWORD dwSizeLow, dwSizeHigh;
again:
	DBG_ALIGNMENT_DISABLE();
	dwSizeLow = GetCompressedFileSizeW(lpFileName, &dwSizeHigh);
	if (dwSizeLow == INVALID_FILE_SIZE) {
		DWORD dwError;
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(dwError)) {
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		if (dwError != NO_ERROR)
			RETURN_ERROR(dwError, "Failed to determine the compressed size of %lq", lpFileName);
	} else {
		DBG_ALIGNMENT_ENABLE();
	}
	return DeeInt_NewUInt64((uint64_t)dwSizeLow |
	                     (uint64_t)dwSizeHigh << 32);
err:
	return NULL;
}



/*[[[deemon import("rt.gen.dexutils").gw("FlushFileBuffers", "hFile:nt:HANDLE" ERROR_OR_BOOL); ]]]*/
#define LIBWIN32_FLUSHFILEBUFFERS_DEF          DEX_MEMBER_F("FlushFileBuffers", &libwin32_FlushFileBuffers, DEXSYM_READONLY, "(hFile:?X3?Dint?DFile?Ewin32:HANDLE)"),
#define LIBWIN32_FLUSHFILEBUFFERS_DEF_DOC(doc) DEX_MEMBER_F("FlushFileBuffers", &libwin32_FlushFileBuffers, DEXSYM_READONLY, "(hFile:?X3?Dint?DFile?Ewin32:HANDLE)\n" doc),
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_FlushFileBuffers_f_impl(HANDLE hFile);
#ifndef DEFINED_kwlist__hFile
#define DEFINED_kwlist__hFile
PRIVATE DEFINE_KWLIST(kwlist__hFile, { KEX("hFile", 0x7bb582fb, 0x2d053fe88ad300ff), KEND });
#endif /* !DEFINED_kwlist__hFile */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_FlushFileBuffers_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *raw_hFile;
	} args;
	HANDLE hFile;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__hFile, "o:FlushFileBuffers", &args))
		goto err;
	if unlikely(DeeNTSystem_TryGetHandle(args.raw_hFile, (void **)&hFile))
		goto err;
	return libwin32_FlushFileBuffers_f_impl(hFile);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_FlushFileBuffers, &libwin32_FlushFileBuffers_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_FlushFileBuffers_f_impl(HANDLE hFile)
/*[[[end]]]*/
{
	BOOL bResult;
again:
	DBG_ALIGNMENT_DISABLE();
	bResult = FlushFileBuffers(hFile);
	if (!bResult) {
		DWORD dwError;
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(dwError)) {
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		RETURN_ERROR_OR_FALSE(dwError,
		                      "Failed to flush buffers of file %p",
		                      hFile);
	}
	DBG_ALIGNMENT_ENABLE();
	RETURN_SUCCESS_OR_TRUE;
err:
	return NULL;
}



/*[[[deemon import("rt.gen.dexutils").gw("GetFinalPathNameByHandle", "hFile:nt:HANDLE,dwFlags:nt:DWORD=0->" MAYBE_NONE("?Dstring")); ]]]*/
#define LIBWIN32_GETFINALPATHNAMEBYHANDLE_DEF          DEX_MEMBER_F("GetFinalPathNameByHandle", &libwin32_GetFinalPathNameByHandle, DEXSYM_READONLY, "(hFile:?X3?Dint?DFile?Ewin32:HANDLE,dwFlags=!0)->?Dstring"),
#define LIBWIN32_GETFINALPATHNAMEBYHANDLE_DEF_DOC(doc) DEX_MEMBER_F("GetFinalPathNameByHandle", &libwin32_GetFinalPathNameByHandle, DEXSYM_READONLY, "(hFile:?X3?Dint?DFile?Ewin32:HANDLE,dwFlags=!0)->?Dstring\n" doc),
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetFinalPathNameByHandle_f_impl(HANDLE hFile, DWORD dwFlags);
#ifndef DEFINED_kwlist__hFile_dwFlags
#define DEFINED_kwlist__hFile_dwFlags
PRIVATE DEFINE_KWLIST(kwlist__hFile_dwFlags, { KEX("hFile", 0x7bb582fb, 0x2d053fe88ad300ff), KEX("dwFlags", 0x70f4a9da, 0xe8fc02ace3a22768), KEND });
#endif /* !DEFINED_kwlist__hFile_dwFlags */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetFinalPathNameByHandle_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *raw_hFile;
		DWORD dwFlags;
	} args;
	HANDLE hFile;
	args.dwFlags = 0;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__hFile_dwFlags, "o|" UNPu32 ":GetFinalPathNameByHandle", &args))
		goto err;
	if unlikely(DeeNTSystem_TryGetHandle(args.raw_hFile, (void **)&hFile))
		goto err;
	return libwin32_GetFinalPathNameByHandle_f_impl(hFile, args.dwFlags);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_GetFinalPathNameByHandle, &libwin32_GetFinalPathNameByHandle_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetFinalPathNameByHandle_f_impl(HANDLE hFile, DWORD dwFlags)
/*[[[end]]]*/
{
	int error;
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	error = DeeNTSystem_PrintFinalPathNameByHandle(&printer, (void *)hFile, dwFlags);
	if (error != 0) {
		if unlikely(error < 0)
			goto err;
		if (error == 1)
			goto system_error;
		/* Fallback when unsupported */
		error = DeeNTSystem_PrintFilenameOfHandle(&printer, (void *)hFile);
		if (error != 0) {
			if (error > 0)
				goto system_error;
			goto err;
		}
	}
	return unicode_printer_pack(&printer);
system_error:
	unicode_printer_fini(&printer);
	RETURN_ERROR(GetLastError(),
	             "Failed to determine the filename of handle %p",
	             hFile);
err:
	unicode_printer_fini(&printer);
	return NULL;
}



/*[[[deemon import("rt.gen.dexutils").gw("GetFilenameOfHandle", "hFile:nt:HANDLE->" MAYBE_NONE("?Dstring")); ]]]*/
#define LIBWIN32_GETFILENAMEOFHANDLE_DEF          DEX_MEMBER_F("GetFilenameOfHandle", &libwin32_GetFilenameOfHandle, DEXSYM_READONLY, "(hFile:?X3?Dint?DFile?Ewin32:HANDLE)->?Dstring"),
#define LIBWIN32_GETFILENAMEOFHANDLE_DEF_DOC(doc) DEX_MEMBER_F("GetFilenameOfHandle", &libwin32_GetFilenameOfHandle, DEXSYM_READONLY, "(hFile:?X3?Dint?DFile?Ewin32:HANDLE)->?Dstring\n" doc),
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetFilenameOfHandle_f_impl(HANDLE hFile);
#ifndef DEFINED_kwlist__hFile
#define DEFINED_kwlist__hFile
PRIVATE DEFINE_KWLIST(kwlist__hFile, { KEX("hFile", 0x7bb582fb, 0x2d053fe88ad300ff), KEND });
#endif /* !DEFINED_kwlist__hFile */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetFilenameOfHandle_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *raw_hFile;
	} args;
	HANDLE hFile;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__hFile, "o:GetFilenameOfHandle", &args))
		goto err;
	if unlikely(DeeNTSystem_TryGetHandle(args.raw_hFile, (void **)&hFile))
		goto err;
	return libwin32_GetFilenameOfHandle_f_impl(hFile);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_GetFilenameOfHandle, &libwin32_GetFilenameOfHandle_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetFilenameOfHandle_f_impl(HANDLE hFile)
/*[[[end]]]*/
{
	DREF DeeObject *result;
	result = DeeNTSystem_TryGetFilenameOfHandle((void *)hFile);
	if (result != ITER_DONE)
		return result;
	RETURN_ERROR(GetLastError(),
	             "Failed to determine the filename of handle %p",
	             hFile);
}



/*[[[deemon import("rt.gen.dexutils").gw("FormatErrorMessage", "dwError:nt:DWORD->" MAYBE_NONE("?Dstring")); ]]]*/
#define LIBWIN32_FORMATERRORMESSAGE_DEF          DEX_MEMBER_F("FormatErrorMessage", &libwin32_FormatErrorMessage, DEXSYM_READONLY, "(dwError:?Dint)->?Dstring"),
#define LIBWIN32_FORMATERRORMESSAGE_DEF_DOC(doc) DEX_MEMBER_F("FormatErrorMessage", &libwin32_FormatErrorMessage, DEXSYM_READONLY, "(dwError:?Dint)->?Dstring\n" doc),
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_FormatErrorMessage_f_impl(DWORD dwError);
#ifndef DEFINED_kwlist__dwError
#define DEFINED_kwlist__dwError
PRIVATE DEFINE_KWLIST(kwlist__dwError, { KEX("dwError", 0xc5103b02, 0xc9bfd82ba3f34b64), KEND });
#endif /* !DEFINED_kwlist__dwError */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_FormatErrorMessage_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DWORD dwError;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__dwError, UNPu32 ":FormatErrorMessage", &args))
		goto err;
	return libwin32_FormatErrorMessage_f_impl(args.dwError);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_FormatErrorMessage, &libwin32_FormatErrorMessage_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_FormatErrorMessage_f_impl(DWORD dwError)
/*[[[end]]]*/
{
	DREF DeeObject *result;
	result = DeeNTSystem_FormatErrorMessage(dwError);
	if (result != ITER_DONE)
		return result;
	RETURN_ERROR(GetLastError(),
	             "Failed to format the message for error %#lx",
	             dwError);
}



/*[[[deemon import("rt.gen.dexutils").gw("GetMappedFileName", "hProcess:nt:HANDLE,lpv:c:ptr->" MAYBE_NONE("?Dstring")); ]]]*/
#define LIBWIN32_GETMAPPEDFILENAME_DEF          DEX_MEMBER_F("GetMappedFileName", &libwin32_GetMappedFileName, DEXSYM_READONLY, "(hProcess:?X3?Dint?DFile?Ewin32:HANDLE,lpv:?Aptr?Ectypes:void)->?Dstring"),
#define LIBWIN32_GETMAPPEDFILENAME_DEF_DOC(doc) DEX_MEMBER_F("GetMappedFileName", &libwin32_GetMappedFileName, DEXSYM_READONLY, "(hProcess:?X3?Dint?DFile?Ewin32:HANDLE,lpv:?Aptr?Ectypes:void)->?Dstring\n" doc),
FORCELOCAL WUNUSED NONNULL((2)) DREF DeeObject *DCALL libwin32_GetMappedFileName_f_impl(HANDLE hProcess, void *lpv);
#ifndef DEFINED_kwlist__hProcess_lpv
#define DEFINED_kwlist__hProcess_lpv
PRIVATE DEFINE_KWLIST(kwlist__hProcess_lpv, { KEX("hProcess", 0x97c84287, 0x1d63e586eef1387e), KEX("lpv", 0xf1b74526, 0x686522cb35159588), KEND });
#endif /* !DEFINED_kwlist__hProcess_lpv */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetMappedFileName_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *raw_hProcess;
		DeeObject *raw_lpv;
	} args;
	HANDLE hProcess;
	void *lpv;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__hProcess_lpv, "oo:GetMappedFileName", &args))
		goto err;
	if unlikely(DeeNTSystem_TryGetHandle(args.raw_hProcess, (void **)&hProcess))
		goto err;
	if unlikely(DeeCTypes_GetPointer(args.raw_lpv, &lpv))
		goto err;
	return libwin32_GetMappedFileName_f_impl(hProcess, lpv);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_GetMappedFileName, &libwin32_GetMappedFileName_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((2)) DREF DeeObject *DCALL libwin32_GetMappedFileName_f_impl(HANDLE hProcess, void *lpv)
/*[[[end]]]*/
{
	int error;
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	error = DeeNTSystem_PrintMappedFileName(&printer, (void *)hProcess, lpv);
	if (error != 0) {
		if unlikely(error < 0)
			goto err;
		if (error != 1)
			SetLastError(ERROR_INVALID_FUNCTION);
		goto system_error;
	}
	return unicode_printer_pack(&printer);
system_error:
	unicode_printer_fini(&printer);
	RETURN_ERROR(GetLastError(),
	             "Failed to determine the mapped filename of address %p in process %p",
	             lpv, hProcess);
err:
	unicode_printer_fini(&printer);
	return NULL;
}



/*[[[deemon import("rt.gen.dexutils").gw("MapViewOfFile",
     "hFileMappingObject:nt:HANDLE"
    ",dwDesiredAccess:nt:DWORD=FILE_MAP_READ=!GFILE_MAP_READ"
    ",dwFileOffset:I64u=0"
    ",dwNumberOfBytesToMap:Iu=0"
    "->" MAYBE_NONE("?Aptr?Ectypes:void")); ]]]*/
#define LIBWIN32_MAPVIEWOFFILE_DEF          DEX_MEMBER_F("MapViewOfFile", &libwin32_MapViewOfFile, DEXSYM_READONLY, "(hFileMappingObject:?X3?Dint?DFile?Ewin32:HANDLE,dwDesiredAccess:?Dint=!GFILE_MAP_READ,dwFileOffset=!0,dwNumberOfBytesToMap=!0)->?Aptr?Ectypes:void"),
#define LIBWIN32_MAPVIEWOFFILE_DEF_DOC(doc) DEX_MEMBER_F("MapViewOfFile", &libwin32_MapViewOfFile, DEXSYM_READONLY, "(hFileMappingObject:?X3?Dint?DFile?Ewin32:HANDLE,dwDesiredAccess:?Dint=!GFILE_MAP_READ,dwFileOffset=!0,dwNumberOfBytesToMap=!0)->?Aptr?Ectypes:void\n" doc),
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_MapViewOfFile_f_impl(HANDLE hFileMappingObject, DWORD dwDesiredAccess, uint64_t dwFileOffset, size_t dwNumberOfBytesToMap);
#ifndef DEFINED_kwlist__hFileMappingObject_dwDesiredAccess_dwFileOffset_dwNumberOfBytesToMap
#define DEFINED_kwlist__hFileMappingObject_dwDesiredAccess_dwFileOffset_dwNumberOfBytesToMap
PRIVATE DEFINE_KWLIST(kwlist__hFileMappingObject_dwDesiredAccess_dwFileOffset_dwNumberOfBytesToMap, { KEX("hFileMappingObject", 0x46b1d30, 0x2ef7d2429f7b9520), KEX("dwDesiredAccess", 0xa4c2866d, 0x5a4b46d72691f2f), KEX("dwFileOffset", 0x84a0a4b1, 0x646b5b12aaf072db), KEX("dwNumberOfBytesToMap", 0xddc7b3cf, 0x85cd9b92e81a2a20), KEND });
#endif /* !DEFINED_kwlist__hFileMappingObject_dwDesiredAccess_dwFileOffset_dwNumberOfBytesToMap */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_MapViewOfFile_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *raw_hFileMappingObject;
		DWORD dwDesiredAccess;
		uint64_t dwFileOffset;
		size_t dwNumberOfBytesToMap;
	} args;
	HANDLE hFileMappingObject;
	args.dwDesiredAccess = FILE_MAP_READ;
	args.dwFileOffset = 0;
	args.dwNumberOfBytesToMap = 0;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__hFileMappingObject_dwDesiredAccess_dwFileOffset_dwNumberOfBytesToMap, "o|" UNPu32 UNPu64 UNPuSIZ ":MapViewOfFile", &args))
		goto err;
	if unlikely(DeeNTSystem_TryGetHandle(args.raw_hFileMappingObject, (void **)&hFileMappingObject))
		goto err;
	return libwin32_MapViewOfFile_f_impl(hFileMappingObject, args.dwDesiredAccess, args.dwFileOffset, args.dwNumberOfBytesToMap);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_MapViewOfFile, &libwin32_MapViewOfFile_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_MapViewOfFile_f_impl(HANDLE hFileMappingObject, DWORD dwDesiredAccess, uint64_t dwFileOffset, size_t dwNumberOfBytesToMap)
/*[[[end]]]*/
{
	DREF DeeObject *result;
	LPVOID pResult;
again:
	DBG_ALIGNMENT_DISABLE();
	pResult = MapViewOfFile(hFileMappingObject,
	                        dwDesiredAccess,
	                        (DWORD)(dwFileOffset >> 32),
	                        (DWORD)(dwFileOffset),
	                        dwNumberOfBytesToMap);
	if (pResult == NULL) {
		DWORD dwError;
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(dwError)) {
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		RETURN_ERROR(dwError,
		             "Failed to map a view of %p into memory "
		             "(dwDesiredAccess: %#" PRFx32 ", dwFileOffset: %" PRFu64 ", dwNumberOfBytesToMap: %" PRFuSIZ ")",
		             hFileMappingObject, dwDesiredAccess,
		             dwFileOffset, dwNumberOfBytesToMap);
	}
	DBG_ALIGNMENT_ENABLE();
	result = DeeCTypes_CreateVoidPointer(pResult);
	if unlikely(!result) {
		DWORD dwError;
		DBG_ALIGNMENT_DISABLE();
		dwError = GetLastError();
		UnmapViewOfFile(pResult);
		SetLastError(dwError);
		DBG_ALIGNMENT_ENABLE();
	}
	return result;
err:
	return NULL;
}



/*[[[deemon import("rt.gen.dexutils").gw("UnmapViewOfFile", "lpBaseAddress:c:ptr" ERROR_OR_BOOL); ]]]*/
#define LIBWIN32_UNMAPVIEWOFFILE_DEF          DEX_MEMBER_F("UnmapViewOfFile", &libwin32_UnmapViewOfFile, DEXSYM_READONLY, "(lpBaseAddress:?Aptr?Ectypes:void)"),
#define LIBWIN32_UNMAPVIEWOFFILE_DEF_DOC(doc) DEX_MEMBER_F("UnmapViewOfFile", &libwin32_UnmapViewOfFile, DEXSYM_READONLY, "(lpBaseAddress:?Aptr?Ectypes:void)\n" doc),
FORCELOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL libwin32_UnmapViewOfFile_f_impl(void *lpBaseAddress);
#ifndef DEFINED_kwlist__lpBaseAddress
#define DEFINED_kwlist__lpBaseAddress
PRIVATE DEFINE_KWLIST(kwlist__lpBaseAddress, { KEX("lpBaseAddress", 0x161d05c5, 0x641a7dcfb6aac95d), KEND });
#endif /* !DEFINED_kwlist__lpBaseAddress */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_UnmapViewOfFile_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *raw_lpBaseAddress;
	} args;
	void *lpBaseAddress;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__lpBaseAddress, "o:UnmapViewOfFile", &args))
		goto err;
	if unlikely(DeeCTypes_GetPointer(args.raw_lpBaseAddress, &lpBaseAddress))
		goto err;
	return libwin32_UnmapViewOfFile_f_impl(lpBaseAddress);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_UnmapViewOfFile, &libwin32_UnmapViewOfFile_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL libwin32_UnmapViewOfFile_f_impl(void *lpBaseAddress)
/*[[[end]]]*/
{
	BOOL bOk;
again:
	DBG_ALIGNMENT_DISABLE();
	bOk = UnmapViewOfFile((LPCVOID)lpBaseAddress);
	if (!bOk) {
		DWORD dwError;
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(dwError)) {
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		RETURN_ERROR_OR_FALSE(dwError,
		                      "failed to unmap view of file at %p",
		                      lpBaseAddress);
	}
	DBG_ALIGNMENT_ENABLE();
	RETURN_SUCCESS_OR_TRUE;
err:
	return NULL;
}



/*[[[deemon import("rt.gen.dexutils").gw("CreateFileMapping",
     "hFile:nt:HANDLE"
    ",lpFileMappingAttributes:?GSECURITY_ATTRIBUTES=NULL"
    ",flProtect:nt:DWORD=PAGE_READONLY=!GPAGE_READONLY"
    ",dwMaximumSize:I64u=0"
    ",lpName:nt:LPCWSTR=NULL"
    "->" MAYBE_NONE("?GHANDLE")); ]]]*/
#define LIBWIN32_CREATEFILEMAPPING_DEF          DEX_MEMBER_F("CreateFileMapping", &libwin32_CreateFileMapping, DEXSYM_READONLY, "(hFile:?X3?Dint?DFile?Ewin32:HANDLE,lpFileMappingAttributes?:?GSECURITY_ATTRIBUTES,flProtect:?Dint=!GPAGE_READONLY,dwMaximumSize=!0,lpName?:?Dstring)->?GHANDLE"),
#define LIBWIN32_CREATEFILEMAPPING_DEF_DOC(doc) DEX_MEMBER_F("CreateFileMapping", &libwin32_CreateFileMapping, DEXSYM_READONLY, "(hFile:?X3?Dint?DFile?Ewin32:HANDLE,lpFileMappingAttributes?:?GSECURITY_ATTRIBUTES,flProtect:?Dint=!GPAGE_READONLY,dwMaximumSize=!0,lpName?:?Dstring)->?GHANDLE\n" doc),
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_CreateFileMapping_f_impl(HANDLE hFile, DeeObject *lpFileMappingAttributes, DWORD flProtect, uint64_t dwMaximumSize, LPCWSTR lpName);
#ifndef DEFINED_kwlist__hFile_lpFileMappingAttributes_flProtect_dwMaximumSize_lpName
#define DEFINED_kwlist__hFile_lpFileMappingAttributes_flProtect_dwMaximumSize_lpName
PRIVATE DEFINE_KWLIST(kwlist__hFile_lpFileMappingAttributes_flProtect_dwMaximumSize_lpName, { KEX("hFile", 0x7bb582fb, 0x2d053fe88ad300ff), KEX("lpFileMappingAttributes", 0x6fe46276, 0x9c86d45658bc484f), KEX("flProtect", 0xdf6d0b1d, 0xcbd0ed2040d7328d), KEX("dwMaximumSize", 0x1c06865c, 0x20b5c7bece574fe5), KEX("lpName", 0xe3f0b9dd, 0x6d44dd71565f84d6), KEND });
#endif /* !DEFINED_kwlist__hFile_lpFileMappingAttributes_flProtect_dwMaximumSize_lpName */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_CreateFileMapping_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *raw_hFile;
		DeeObject *lpFileMappingAttributes;
		DWORD flProtect;
		uint64_t dwMaximumSize;
		LPCWSTR lpName;
	} args;
	HANDLE hFile;
	args.lpFileMappingAttributes = NULL;
	args.flProtect = PAGE_READONLY;
	args.dwMaximumSize = 0;
	args.lpName = NULL;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__hFile_lpFileMappingAttributes_flProtect_dwMaximumSize_lpName, "o|o" UNPu32 UNPu64 "U16s:CreateFileMapping", &args))
		goto err;
	if unlikely(DeeNTSystem_TryGetHandle(args.raw_hFile, (void **)&hFile))
		goto err;
	return libwin32_CreateFileMapping_f_impl(hFile, args.lpFileMappingAttributes, args.flProtect, args.dwMaximumSize, args.lpName);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_CreateFileMapping, &libwin32_CreateFileMapping_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_CreateFileMapping_f_impl(HANDLE hFile, DeeObject *lpFileMappingAttributes, DWORD flProtect, uint64_t dwMaximumSize, LPCWSTR lpName)
/*[[[end]]]*/
{
	DREF DeeObject *result;
	HANDLE hResult;
again:
	(void)lpFileMappingAttributes; /* TODO */
	DBG_ALIGNMENT_DISABLE();
	hResult = CreateFileMappingW(hFile,
	                             NULL,
	                             flProtect,
	                             (DWORD)(dwMaximumSize >> 32),
	                             (DWORD)(dwMaximumSize),
	                             lpName);
	if (hResult == NULL) {
		DWORD dwError;
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(dwError)) {
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		RETURN_ERROR(dwError,
		             "Failed to create file mapping for %p "
		             "(flProtect: %#" PRFx32 ", dwMaximumSize: %" PRFu64 ", lpName: %lq)",
		             hFile, flProtect, dwMaximumSize, lpName);
	}
	DBG_ALIGNMENT_ENABLE();
	result = libwin32_CreateHandle(hResult);
	if unlikely(!result) {
		DWORD dwError;
		DBG_ALIGNMENT_DISABLE();
		dwError = GetLastError();
		CloseHandle(hResult);
		SetLastError(dwError);
		DBG_ALIGNMENT_ENABLE();
	}
	return result;
err:
	return NULL;
}



/*[[[deemon import("rt.gen.dexutils").gw("GetCurrentProcess", "->?GHANDLE"); ]]]*/
#define LIBWIN32_GETCURRENTPROCESS_DEF          DEX_MEMBER_F("GetCurrentProcess", &libwin32_GetCurrentProcess, DEXSYM_READONLY, "->?GHANDLE"),
#define LIBWIN32_GETCURRENTPROCESS_DEF_DOC(doc) DEX_MEMBER_F("GetCurrentProcess", &libwin32_GetCurrentProcess, DEXSYM_READONLY, "->?GHANDLE\n" doc),
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetCurrentProcess_f_impl(void);
PRIVATE DEFINE_CMETHOD0(libwin32_GetCurrentProcess, &libwin32_GetCurrentProcess_f_impl, METHOD_FNORMAL);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetCurrentProcess_f_impl(void)
/*[[[end]]]*/
{
	HANDLE hCurrentProcess;
	DBG_ALIGNMENT_DISABLE();
	hCurrentProcess = GetCurrentProcess();
	DBG_ALIGNMENT_ENABLE();
	return libwin32_CreateHandle(hCurrentProcess);
}



/*[[[deemon import("rt.gen.dexutils").gw("GetCurrentThread", "->?GHANDLE"); ]]]*/
#define LIBWIN32_GETCURRENTTHREAD_DEF          DEX_MEMBER_F("GetCurrentThread", &libwin32_GetCurrentThread, DEXSYM_READONLY, "->?GHANDLE"),
#define LIBWIN32_GETCURRENTTHREAD_DEF_DOC(doc) DEX_MEMBER_F("GetCurrentThread", &libwin32_GetCurrentThread, DEXSYM_READONLY, "->?GHANDLE\n" doc),
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetCurrentThread_f_impl(void);
PRIVATE DEFINE_CMETHOD0(libwin32_GetCurrentThread, &libwin32_GetCurrentThread_f_impl, METHOD_FNORMAL);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetCurrentThread_f_impl(void)
/*[[[end]]]*/
{
	HANDLE hCurrentThread;
	DBG_ALIGNMENT_DISABLE();
	hCurrentThread = GetCurrentProcess();
	DBG_ALIGNMENT_ENABLE();
	return libwin32_CreateHandle(hCurrentThread);
}



/*[[[deemon import("rt.gen.dexutils").gw("GetCurrentProcessId", "->?Dint"); ]]]*/
#define LIBWIN32_GETCURRENTPROCESSID_DEF          DEX_MEMBER_F("GetCurrentProcessId", &libwin32_GetCurrentProcessId, DEXSYM_READONLY, "->?Dint"),
#define LIBWIN32_GETCURRENTPROCESSID_DEF_DOC(doc) DEX_MEMBER_F("GetCurrentProcessId", &libwin32_GetCurrentProcessId, DEXSYM_READONLY, "->?Dint\n" doc),
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetCurrentProcessId_f_impl(void);
PRIVATE DEFINE_CMETHOD0(libwin32_GetCurrentProcessId, &libwin32_GetCurrentProcessId_f_impl, METHOD_FNORMAL);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetCurrentProcessId_f_impl(void)
/*[[[end]]]*/
{
	DWORD dwCurrentProcessId;
	DBG_ALIGNMENT_DISABLE();
	dwCurrentProcessId = GetCurrentProcessId();
	DBG_ALIGNMENT_ENABLE();
	return DeeInt_NewUInt32(dwCurrentProcessId);
}



/*[[[deemon import("rt.gen.dexutils").gw("GetCurrentThreadId", "->?Dint"); ]]]*/
#define LIBWIN32_GETCURRENTTHREADID_DEF          DEX_MEMBER_F("GetCurrentThreadId", &libwin32_GetCurrentThreadId, DEXSYM_READONLY, "->?Dint"),
#define LIBWIN32_GETCURRENTTHREADID_DEF_DOC(doc) DEX_MEMBER_F("GetCurrentThreadId", &libwin32_GetCurrentThreadId, DEXSYM_READONLY, "->?Dint\n" doc),
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetCurrentThreadId_f_impl(void);
PRIVATE DEFINE_CMETHOD0(libwin32_GetCurrentThreadId, &libwin32_GetCurrentThreadId_f_impl, METHOD_FNORMAL);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetCurrentThreadId_f_impl(void)
/*[[[end]]]*/
{
	DWORD dwCurrentThreadId;
	DBG_ALIGNMENT_DISABLE();
	dwCurrentThreadId = GetCurrentThreadId();
	DBG_ALIGNMENT_ENABLE();
	return DeeInt_NewUInt32(dwCurrentThreadId);
}



/*[[[deemon import("rt.gen.dexutils").gw("GetStdHandle", "nStdHandle:I32d->?GHANDLE"); ]]]*/
#define LIBWIN32_GETSTDHANDLE_DEF          DEX_MEMBER_F("GetStdHandle", &libwin32_GetStdHandle, DEXSYM_READONLY, "(nStdHandle:?Dint)->?GHANDLE"),
#define LIBWIN32_GETSTDHANDLE_DEF_DOC(doc) DEX_MEMBER_F("GetStdHandle", &libwin32_GetStdHandle, DEXSYM_READONLY, "(nStdHandle:?Dint)->?GHANDLE\n" doc),
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetStdHandle_f_impl(int32_t nStdHandle);
#ifndef DEFINED_kwlist__nStdHandle
#define DEFINED_kwlist__nStdHandle
PRIVATE DEFINE_KWLIST(kwlist__nStdHandle, { KEX("nStdHandle", 0x89e5132, 0xe942244b90b54965), KEND });
#endif /* !DEFINED_kwlist__nStdHandle */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetStdHandle_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		int32_t nStdHandle;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__nStdHandle, UNPd32 ":GetStdHandle", &args))
		goto err;
	return libwin32_GetStdHandle_f_impl(args.nStdHandle);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_GetStdHandle, &libwin32_GetStdHandle_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetStdHandle_f_impl(int32_t nStdHandle)
/*[[[end]]]*/
{
	HANDLE hResult;
again:
	DBG_ALIGNMENT_DISABLE();
	hResult = GetStdHandle((DWORD)nStdHandle);
	if unlikely(!hResult || hResult == INVALID_HANDLE_VALUE) {
		DWORD dwError;
		dwError = GetLastError();
		SetLastError(NO_ERROR);
		if (DeeNTSystem_IsIntr(dwError)) {
			DBG_ALIGNMENT_ENABLE();
check_interrupt:
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		hResult = GetStdHandle((DWORD)nStdHandle);
		if (!hResult || hResult == INVALID_HANDLE_VALUE) {
			dwError = GetLastError();
			DBG_ALIGNMENT_ENABLE();
			if (dwError != NO_ERROR) {
				if (DeeNTSystem_IsIntr(dwError))
					goto check_interrupt;
				RETURN_ERROR(dwError,
				             "Failed to get STD handle %" PRFd32,
				             nStdHandle);
			}
			goto done;
		}
	}
	DBG_ALIGNMENT_ENABLE();
done:
	return libwin32_CreateHandle(hResult);
err:
	return NULL;
}



/*[[[deemon import("rt.gen.dexutils").gw("SetStdHandle", "nStdHandle:nt:DWORD,hHandle:nt:HANDLE" ERROR_OR_BOOL); ]]]*/
#define LIBWIN32_SETSTDHANDLE_DEF          DEX_MEMBER_F("SetStdHandle", &libwin32_SetStdHandle, DEXSYM_READONLY, "(nStdHandle:?Dint,hHandle:?X3?Dint?DFile?Ewin32:HANDLE)"),
#define LIBWIN32_SETSTDHANDLE_DEF_DOC(doc) DEX_MEMBER_F("SetStdHandle", &libwin32_SetStdHandle, DEXSYM_READONLY, "(nStdHandle:?Dint,hHandle:?X3?Dint?DFile?Ewin32:HANDLE)\n" doc),
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_SetStdHandle_f_impl(DWORD nStdHandle, HANDLE hHandle);
#ifndef DEFINED_kwlist__nStdHandle_hHandle
#define DEFINED_kwlist__nStdHandle_hHandle
PRIVATE DEFINE_KWLIST(kwlist__nStdHandle_hHandle, { KEX("nStdHandle", 0x89e5132, 0xe942244b90b54965), KEX("hHandle", 0x69103c85, 0xc1f4ea73037322b8), KEND });
#endif /* !DEFINED_kwlist__nStdHandle_hHandle */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_SetStdHandle_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DWORD nStdHandle;
		DeeObject *raw_hHandle;
	} args;
	HANDLE hHandle;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__nStdHandle_hHandle, UNPu32 "o:SetStdHandle", &args))
		goto err;
	if unlikely(DeeNTSystem_TryGetHandle(args.raw_hHandle, (void **)&hHandle))
		goto err;
	return libwin32_SetStdHandle_f_impl(args.nStdHandle, hHandle);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_SetStdHandle, &libwin32_SetStdHandle_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_SetStdHandle_f_impl(DWORD nStdHandle, HANDLE hHandle)
/*[[[end]]]*/
{
	BOOL bOk;
again:
	DBG_ALIGNMENT_DISABLE();
	bOk = SetStdHandle(nStdHandle, hHandle);
	if (!bOk) {
		DWORD dwError;
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(dwError)) {
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		RETURN_ERROR_OR_FALSE(dwError,
		                      "Failed to set STD handle %" PRFu32 " to %p",
		                      nStdHandle, hHandle);
	}
	DBG_ALIGNMENT_ENABLE();
	RETURN_SUCCESS_OR_TRUE;
err:
	return NULL;
}





/*[[[deemon import("rt.gen.dexutils").gw("CreateNamedPipe",
      "lpName:nt:LPCWSTR"
     ",dwOpenMode:nt:DWORD=PIPE_ACCESS_DUPLEX=!GPIPE_ACCESS_DUPLEX"
     ",dwPipeMode:nt:DWORD=PIPE_TYPE_BYTE | PIPE_READMODE_BYTE=!|!PIPE_TYPE_BYTE!PIPE_READMODE_BYTE"
     ",nMaxInstances:nt:DWORD=PIPE_UNLIMITED_INSTANCES=!GPIPE_UNLIMITED_INSTANCES"
     ",nOutBufferSize:nt:DWORD=65536"
     ",nInBufferSize:nt:DWORD=65536"
     ",nDefaultTimeOut:nt:DWORD=0"
     ",lpSecurityAttributes:?GSECURITY_ATTRIBUTES=NULL"
     "->" MAYBE_NONE("?GHANDLE")
     ); ]]]*/
#define LIBWIN32_CREATENAMEDPIPE_DEF          DEX_MEMBER_F("CreateNamedPipe", &libwin32_CreateNamedPipe, DEXSYM_READONLY, "(lpName:?Dstring,dwOpenMode:?Dint=!GPIPE_ACCESS_DUPLEX,dwPipeMode:?Dint=!|!PIPE_TYPE_BYTE!PIPE_READMODE_BYTE,nMaxInstances:?Dint=!GPIPE_UNLIMITED_INSTANCES,nOutBufferSize=!65536,nInBufferSize=!65536,nDefaultTimeOut=!0,lpSecurityAttributes?:?GSECURITY_ATTRIBUTES)->?GHANDLE"),
#define LIBWIN32_CREATENAMEDPIPE_DEF_DOC(doc) DEX_MEMBER_F("CreateNamedPipe", &libwin32_CreateNamedPipe, DEXSYM_READONLY, "(lpName:?Dstring,dwOpenMode:?Dint=!GPIPE_ACCESS_DUPLEX,dwPipeMode:?Dint=!|!PIPE_TYPE_BYTE!PIPE_READMODE_BYTE,nMaxInstances:?Dint=!GPIPE_UNLIMITED_INSTANCES,nOutBufferSize=!65536,nInBufferSize=!65536,nDefaultTimeOut=!0,lpSecurityAttributes?:?GSECURITY_ATTRIBUTES)->?GHANDLE\n" doc),
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_CreateNamedPipe_f_impl(LPCWSTR lpName, DWORD dwOpenMode, DWORD dwPipeMode, DWORD nMaxInstances, DWORD nOutBufferSize, DWORD nInBufferSize, DWORD nDefaultTimeOut, DeeObject *lpSecurityAttributes);
#ifndef DEFINED_kwlist__lpName_dwOpenMode_dwPipeMode_nMaxInstances_nOutBufferSize_nInBufferSize_nDefaultTimeOut_lpSecurityAttributes
#define DEFINED_kwlist__lpName_dwOpenMode_dwPipeMode_nMaxInstances_nOutBufferSize_nInBufferSize_nDefaultTimeOut_lpSecurityAttributes
PRIVATE DEFINE_KWLIST(kwlist__lpName_dwOpenMode_dwPipeMode_nMaxInstances_nOutBufferSize_nInBufferSize_nDefaultTimeOut_lpSecurityAttributes, { KEX("lpName", 0xe3f0b9dd, 0x6d44dd71565f84d6), KEX("dwOpenMode", 0xf8b7908a, 0xce312d7bb003d371), KEX("dwPipeMode", 0x9d0eecb7, 0x64b7c732130546bf), KEX("nMaxInstances", 0x5b3bc388, 0x5f8d92561749ec2c), KEX("nOutBufferSize", 0x3df60fc9, 0xb905e088cf06176c), KEX("nInBufferSize", 0xf307c4c1, 0x295bbc224966e647), KEX("nDefaultTimeOut", 0xaee36c29, 0xee367849675af3fb), KEX("lpSecurityAttributes", 0xbefe38d5, 0x2a65624101bba401), KEND });
#endif /* !DEFINED_kwlist__lpName_dwOpenMode_dwPipeMode_nMaxInstances_nOutBufferSize_nInBufferSize_nDefaultTimeOut_lpSecurityAttributes */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_CreateNamedPipe_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		LPCWSTR lpName;
		DWORD dwOpenMode;
		DWORD dwPipeMode;
		DWORD nMaxInstances;
		DWORD nOutBufferSize;
		DWORD nInBufferSize;
		DWORD nDefaultTimeOut;
		DeeObject *lpSecurityAttributes;
	} args;
	args.dwOpenMode = PIPE_ACCESS_DUPLEX;
	args.dwPipeMode = PIPE_TYPE_BYTE | PIPE_READMODE_BYTE;
	args.nMaxInstances = PIPE_UNLIMITED_INSTANCES;
	args.nOutBufferSize = 65536;
	args.nInBufferSize = 65536;
	args.nDefaultTimeOut = 0;
	args.lpSecurityAttributes = NULL;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__lpName_dwOpenMode_dwPipeMode_nMaxInstances_nOutBufferSize_nInBufferSize_nDefaultTimeOut_lpSecurityAttributes, "U16s|" UNPu32 UNPu32 UNPu32 UNPu32 UNPu32 UNPu32 "o:CreateNamedPipe", &args))
		goto err;
	return libwin32_CreateNamedPipe_f_impl(args.lpName, args.dwOpenMode, args.dwPipeMode, args.nMaxInstances, args.nOutBufferSize, args.nInBufferSize, args.nDefaultTimeOut, args.lpSecurityAttributes);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_CreateNamedPipe, &libwin32_CreateNamedPipe_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_CreateNamedPipe_f_impl(LPCWSTR lpName, DWORD dwOpenMode, DWORD dwPipeMode, DWORD nMaxInstances, DWORD nOutBufferSize, DWORD nInBufferSize, DWORD nDefaultTimeOut, DeeObject *lpSecurityAttributes)
/*[[[end]]]*/
{
	HANDLE hResult;
	DREF DeeObject *result;
again:
	DBG_ALIGNMENT_DISABLE();
	(void)lpSecurityAttributes; /* TODO */
	hResult = CreateNamedPipeW(lpName,
	                           dwOpenMode,
	                           dwPipeMode,
	                           nMaxInstances,
	                           nOutBufferSize,
	                           nInBufferSize,
	                           nDefaultTimeOut,
	                           NULL);
	if unlikely(hResult == INVALID_HANDLE_VALUE) {
		DWORD dwError;
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(dwError)) {
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		RETURN_ERROR(dwError,
		             "Failed to create named pipe %lq (dwOpenMode: %#" PRFx32 ", dwPipeMode: %#" PRFx32 ", "
		             "nMaxInstances: %#" PRFx32 ", nOutBufferSize: %#" PRFx32 ", nInBufferSize: %#" PRFx32 ", "
		             "nDefaultTimeOut: %#" PRFx32 ")",
		             lpName, dwOpenMode, dwPipeMode, nMaxInstances,
		             nOutBufferSize, nInBufferSize, nDefaultTimeOut);
	}
	DBG_ALIGNMENT_ENABLE();
	result = libwin32_CreateHandle(hResult);
	if likely(result)
		return result;
	CloseHandle(hResult);
err:
	return NULL;
}

/*[[[deemon import("rt.gen.dexutils").gw("ConnectNamedPipe",
      "hNamedPipe:nt:HANDLE"
     ",lpOverlapped:?GOVERLAPPED=NULL"
     "->" ERROR_OR_BOOL
     ); ]]]*/
#define LIBWIN32_CONNECTNAMEDPIPE_DEF          DEX_MEMBER_F("ConnectNamedPipe", &libwin32_ConnectNamedPipe, DEXSYM_READONLY, "(hNamedPipe:?X3?Dint?DFile?Ewin32:HANDLE,lpOverlapped?:?GOVERLAPPED)"),
#define LIBWIN32_CONNECTNAMEDPIPE_DEF_DOC(doc) DEX_MEMBER_F("ConnectNamedPipe", &libwin32_ConnectNamedPipe, DEXSYM_READONLY, "(hNamedPipe:?X3?Dint?DFile?Ewin32:HANDLE,lpOverlapped?:?GOVERLAPPED)\n" doc),
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_ConnectNamedPipe_f_impl(HANDLE hNamedPipe, DeeObject *lpOverlapped);
#ifndef DEFINED_kwlist__hNamedPipe_lpOverlapped
#define DEFINED_kwlist__hNamedPipe_lpOverlapped
PRIVATE DEFINE_KWLIST(kwlist__hNamedPipe_lpOverlapped, { KEX("hNamedPipe", 0x9576645a, 0x11587025d1b4194e), KEX("lpOverlapped", 0x4a4b3c74, 0x8ba06e2eef56092c), KEND });
#endif /* !DEFINED_kwlist__hNamedPipe_lpOverlapped */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_ConnectNamedPipe_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *raw_hNamedPipe;
		DeeObject *lpOverlapped;
	} args;
	HANDLE hNamedPipe;
	args.lpOverlapped = NULL;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__hNamedPipe_lpOverlapped, "o|o:ConnectNamedPipe", &args))
		goto err;
	if unlikely(DeeNTSystem_TryGetHandle(args.raw_hNamedPipe, (void **)&hNamedPipe))
		goto err;
	return libwin32_ConnectNamedPipe_f_impl(hNamedPipe, args.lpOverlapped);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_ConnectNamedPipe, &libwin32_ConnectNamedPipe_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_ConnectNamedPipe_f_impl(HANDLE hNamedPipe, DeeObject *lpOverlapped)
/*[[[end]]]*/
{
	BOOL bOk;
again:
	DBG_ALIGNMENT_DISABLE();
	(void)lpOverlapped; /* TODO */
	bOk = ConnectNamedPipe(hNamedPipe, NULL);
	if (!bOk) {
		DWORD dwError;
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(dwError)) {
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		RETURN_ERROR_OR_FALSE(dwError,
		                      "Failed to wait for connected on named pipe %p",
		                      hNamedPipe);
	}
	DBG_ALIGNMENT_ENABLE();
	RETURN_SUCCESS_OR_TRUE;
err:
	return NULL;
}

/*[[[deemon import("rt.gen.dexutils").gw("WaitNamedPipe",
      "lpNamedPipeName:nt:LPCWSTR"
     ",nTimeOut:nt:DWORD=NMPWAIT_WAIT_FOREVER=!GNMPWAIT_WAIT_FOREVER"
     "->" ERROR_OR_BOOL
     ); ]]]*/
#define LIBWIN32_WAITNAMEDPIPE_DEF          DEX_MEMBER_F("WaitNamedPipe", &libwin32_WaitNamedPipe, DEXSYM_READONLY, "(lpNamedPipeName:?Dstring,nTimeOut:?Dint=!GNMPWAIT_WAIT_FOREVER)"),
#define LIBWIN32_WAITNAMEDPIPE_DEF_DOC(doc) DEX_MEMBER_F("WaitNamedPipe", &libwin32_WaitNamedPipe, DEXSYM_READONLY, "(lpNamedPipeName:?Dstring,nTimeOut:?Dint=!GNMPWAIT_WAIT_FOREVER)\n" doc),
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_WaitNamedPipe_f_impl(LPCWSTR lpNamedPipeName, DWORD nTimeOut);
#ifndef DEFINED_kwlist__lpNamedPipeName_nTimeOut
#define DEFINED_kwlist__lpNamedPipeName_nTimeOut
PRIVATE DEFINE_KWLIST(kwlist__lpNamedPipeName_nTimeOut, { KEX("lpNamedPipeName", 0xe4d63595, 0x3a690296a1cb028b), KEX("nTimeOut", 0xe64d1cb5, 0x25d3a1e5bafc2636), KEND });
#endif /* !DEFINED_kwlist__lpNamedPipeName_nTimeOut */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_WaitNamedPipe_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		LPCWSTR lpNamedPipeName;
		DWORD nTimeOut;
	} args;
	args.nTimeOut = NMPWAIT_WAIT_FOREVER;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__lpNamedPipeName_nTimeOut, "U16s|" UNPu32 ":WaitNamedPipe", &args))
		goto err;
	return libwin32_WaitNamedPipe_f_impl(args.lpNamedPipeName, args.nTimeOut);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_WaitNamedPipe, &libwin32_WaitNamedPipe_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_WaitNamedPipe_f_impl(LPCWSTR lpNamedPipeName, DWORD nTimeOut)
/*[[[end]]]*/
{
	BOOL bOk;
again:
	DBG_ALIGNMENT_DISABLE();
	bOk = WaitNamedPipeW(lpNamedPipeName, nTimeOut);
	if (!bOk) {
		DWORD dwError;
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(dwError)) {
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		RETURN_ERROR_OR_FALSE(dwError,
		                      "Failed to wait for named pipe %lq (nTimeOut: %#" PRFx32 ")",
		                      lpNamedPipeName, nTimeOut);
	}
	DBG_ALIGNMENT_ENABLE();
	RETURN_SUCCESS_OR_TRUE;
err:
	return NULL;
}

PRIVATE BOOL DCALL libwin32_AcquirePrivilege(LPCWSTR wPrivilegeName) {
	HANDLE hAdjustPrivilegeToken;
	HANDLE hCurrentProcess;
	TOKEN_PRIVILEGES tpTokenPrivileges;
	LUID luPrivilegeId;
	DWORD dwError;
	hCurrentProcess = GetCurrentProcess();
	if unlikely(!OpenProcessToken(hCurrentProcess,
	                              TOKEN_ADJUST_PRIVILEGES,
	                              &hAdjustPrivilegeToken))
		return FALSE;
	if unlikely(!LookupPrivilegeValueW(NULL,
	                                   wPrivilegeName,
	                                   &luPrivilegeId))
		return FALSE;
	tpTokenPrivileges.PrivilegeCount           = 1;
	tpTokenPrivileges.Privileges[0].Luid       = luPrivilegeId;
	tpTokenPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	if unlikely(!AdjustTokenPrivileges(hAdjustPrivilegeToken,
	                                   FALSE, &tpTokenPrivileges,
	                                   0, NULL, NULL))
		return FALSE;
	dwError = GetLastError();
	SetLastError(0);
	if unlikely(dwError == ERROR_NOT_ALL_ASSIGNED)
		return FALSE;
	return TRUE;
}

PRIVATE WCHAR const wstr_SeDebugPrivilege[] = {
	'S', 'e', 'D', 'e', 'b', 'u', 'g', 'P', 'r', 'i', 'v', 'i', 'l', 'e', 'g', 'e', 0
};
PRIVATE BOOL DCALL libwin32_AcquireDebugPrivileges(void) {
	return libwin32_AcquirePrivilege(wstr_SeDebugPrivilege);
}


/*[[[deemon import("rt.gen.dexutils").gw("OpenProcess",
     "dwDesiredAccess:nt:DWORD"
     ",bInheritHandle:c:bool"
     ",dwProcessId:nt:DWORD"
     "->" MAYBE_NONE("?GHANDLE")); ]]]*/
#define LIBWIN32_OPENPROCESS_DEF          DEX_MEMBER_F("OpenProcess", &libwin32_OpenProcess, DEXSYM_READONLY, "(dwDesiredAccess:?Dint,bInheritHandle:?Dbool,dwProcessId:?Dint)->?GHANDLE"),
#define LIBWIN32_OPENPROCESS_DEF_DOC(doc) DEX_MEMBER_F("OpenProcess", &libwin32_OpenProcess, DEXSYM_READONLY, "(dwDesiredAccess:?Dint,bInheritHandle:?Dbool,dwProcessId:?Dint)->?GHANDLE\n" doc),
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_OpenProcess_f_impl(DWORD dwDesiredAccess, bool bInheritHandle, DWORD dwProcessId);
#ifndef DEFINED_kwlist__dwDesiredAccess_bInheritHandle_dwProcessId
#define DEFINED_kwlist__dwDesiredAccess_bInheritHandle_dwProcessId
PRIVATE DEFINE_KWLIST(kwlist__dwDesiredAccess_bInheritHandle_dwProcessId, { KEX("dwDesiredAccess", 0xa4c2866d, 0x5a4b46d72691f2f), KEX("bInheritHandle", 0x632313fa, 0x9e7213d20a3bbbb4), KEX("dwProcessId", 0x922b22c2, 0x27a50c760b86c338), KEND });
#endif /* !DEFINED_kwlist__dwDesiredAccess_bInheritHandle_dwProcessId */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_OpenProcess_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DWORD dwDesiredAccess;
		bool bInheritHandle;
		DWORD dwProcessId;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__dwDesiredAccess_bInheritHandle_dwProcessId, UNPu32 "b" UNPu32 ":OpenProcess", &args))
		goto err;
	return libwin32_OpenProcess_f_impl(args.dwDesiredAccess, args.bInheritHandle, args.dwProcessId);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_OpenProcess, &libwin32_OpenProcess_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_OpenProcess_f_impl(DWORD dwDesiredAccess, bool bInheritHandle, DWORD dwProcessId)
/*[[[end]]]*/
{
	HANDLE hResult;
	DREF DeeObject *result;
again:
	DBG_ALIGNMENT_DISABLE();
	hResult = OpenProcess(dwDesiredAccess, bInheritHandle, dwProcessId);
	if (hResult == NULL) {
		DWORD dwError;
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(dwError)) {
check_interrupt:
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		if (DeeNTSystem_IsAccessDeniedError(dwError)) {
			DBG_ALIGNMENT_DISABLE();
			libwin32_AcquireDebugPrivileges();
			hResult = OpenProcess(dwDesiredAccess, bInheritHandle, dwProcessId);
			if (hResult != NULL)
				goto got_hResult;
			dwError = GetLastError();
			DBG_ALIGNMENT_ENABLE();
			if (DeeNTSystem_IsIntr(dwError))
				goto check_interrupt;
		}
		RETURN_ERROR(dwError,
		             "Failed to open process (dwDesiredAccess: %#" PRFx32 ", bInheritHandle: %u, dwProcessId: %#" PRFx32 ")",
		             dwDesiredAccess, (unsigned int)bInheritHandle, dwProcessId);
	}
got_hResult:
	DBG_ALIGNMENT_ENABLE();
	result = libwin32_CreateHandle(hResult);
	if likely(result)
		return result;
	CloseHandle(hResult);
err:
	return NULL;
}


/*[[[deemon import("rt.gen.dexutils").gw("GetExitCodeProcess",
     "hProcess:nt:HANDLE"
     "->" MAYBE_NONE("?Dint")); ]]]*/
#define LIBWIN32_GETEXITCODEPROCESS_DEF          DEX_MEMBER_F("GetExitCodeProcess", &libwin32_GetExitCodeProcess, DEXSYM_READONLY, "(hProcess:?X3?Dint?DFile?Ewin32:HANDLE)->?Dint"),
#define LIBWIN32_GETEXITCODEPROCESS_DEF_DOC(doc) DEX_MEMBER_F("GetExitCodeProcess", &libwin32_GetExitCodeProcess, DEXSYM_READONLY, "(hProcess:?X3?Dint?DFile?Ewin32:HANDLE)->?Dint\n" doc),
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetExitCodeProcess_f_impl(HANDLE hProcess);
#ifndef DEFINED_kwlist__hProcess
#define DEFINED_kwlist__hProcess
PRIVATE DEFINE_KWLIST(kwlist__hProcess, { KEX("hProcess", 0x97c84287, 0x1d63e586eef1387e), KEND });
#endif /* !DEFINED_kwlist__hProcess */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetExitCodeProcess_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *raw_hProcess;
	} args;
	HANDLE hProcess;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__hProcess, "o:GetExitCodeProcess", &args))
		goto err;
	if unlikely(DeeNTSystem_TryGetHandle(args.raw_hProcess, (void **)&hProcess))
		goto err;
	return libwin32_GetExitCodeProcess_f_impl(hProcess);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_GetExitCodeProcess, &libwin32_GetExitCodeProcess_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetExitCodeProcess_f_impl(HANDLE hProcess)
/*[[[end]]]*/
{
	BOOL bOk;
	DWORD dwExitCode;
again:
	DBG_ALIGNMENT_DISABLE();
	bOk = GetExitCodeProcess(hProcess, &dwExitCode);
	if (!bOk) {
		DWORD dwError;
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(dwError)) {
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		RETURN_ERROR(dwError,
		             "Failed to query process exit code (hProcess: %p)",
		             hProcess);
	}
	DBG_ALIGNMENT_ENABLE();
	return DeeInt_NewUInt32(dwExitCode);
err:
	return NULL;
}


PRIVATE HMODULE libwin32_PsAPIOrKernel32Module = NULL;
PRIVATE WCHAR const wPsapi[]       = { 'P', 'S', 'A', 'P', 'I', 0 };
PRIVATE WCHAR const wPsapiDll[]    = { 'P', 's', 'a', 'p', 'i', '.', 'd', 'l', 'l', 0 };
PRIVATE WCHAR const wKernel32[]    = { 'K', 'E', 'R', 'N', 'E', 'L', '3', '2', 0 };
PRIVATE WCHAR const wKernel32Dll[] = { 'K', 'e', 'r', 'n', 'e', 'l', '3', '2', '.', 'd', 'l', 'l', 0 };

/* @return: NULL: An error was thrown */
PRIVATE HMODULE DCALL libwin32_GetPsAPIOrKernel32Module(void) {
	HMODULE hResult;
	hResult = atomic_read(&libwin32_PsAPIOrKernel32Module);
	if (hResult != NULL)
		return hResult;
	DBG_ALIGNMENT_DISABLE();
	hResult = GetModuleHandleW(wPsapi);
	if (hResult != NULL)
		goto done_set;
	hResult = LoadLibraryW(wPsapiDll);
	if (hResult != NULL)
		goto done_set;
	hResult = GetModuleHandleW(wKernel32);
	if (hResult != NULL)
		goto done_set;
	hResult = LoadLibraryW(wKernel32Dll);
	if (hResult != NULL)
		goto done_set;
	DBG_ALIGNMENT_ENABLE();
	DeeNTSystem_ThrowErrorf(NULL, GetLastError(),
	                        "Failed to load \"psapi.dll\" or \"kernel32.dll\"");
done_set:
	DBG_ALIGNMENT_ENABLE();
	atomic_write(&libwin32_PsAPIOrKernel32Module, hResult);
	return hResult;
}

/* @return: NULL: An error was thrown */
PRIVATE FARPROC DCALL libwin32_GetPsAPIProc(char const *__restrict name) {
	HMODULE hMod;
	FARPROC pResult = NULL;
	hMod = libwin32_GetPsAPIOrKernel32Module();
	if unlikely(!hMod)
		goto done;
	DBG_ALIGNMENT_DISABLE();
	pResult = GetProcAddress(hMod, name);
	DBG_ALIGNMENT_ENABLE();
	if (pResult)
		goto done;
	{
		char *namebuf;
		size_t namelen;
		DWORD dwError;
		namelen  = strlen(name);
		namebuf = (char *)Dee_Mallocac(3 + namelen + 1, sizeof(char));
		if unlikely(!namebuf)
			goto done;
		namebuf[0] = 'K';
		namebuf[1] = '3';
		namebuf[2] = '2';
		*(char *)mempcpyc(namebuf + 3, name, namelen, sizeof(char)) = '\0';
		DBG_ALIGNMENT_DISABLE();
		pResult = GetProcAddress(hMod, namebuf);
		DBG_ALIGNMENT_ENABLE();
		if (pResult) {
			Dee_Freea(namebuf);
			goto done;
		}
		DBG_ALIGNMENT_DISABLE();
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		Dee_Freea(namebuf);
		DeeNTSystem_ThrowErrorf(NULL, dwError,
		                        "Failed to locate symbol %q in \"psapi.dll\" or \"K32%#q\" in \"kernel32.dll\"",
		                        name, name);
	}
done:
	return pResult;
}

#define LOAD_PSAPI_FUNCTION(err_label, returnType, cc, name, args) \
	typedef returnType (cc *LP_psapi_##name) args;                 \
	PRIVATE LP_psapi_##name name = NULL;                           \
	if (!name) {                                                   \
		name = (LP_psapi_##name)libwin32_GetPsAPIProc(#name);      \
		if unlikely(!name)                                         \
			goto err_label;                                        \
	}


/*[[[deemon import("rt.gen.dexutils").gw("EnumProcessModules",
      "hProcess:nt:HANDLE"
     ",dwFilterFlag:nt:DWORD=LIST_MODULES_DEFAULT=!GLIST_MODULES_DEFAULT"
     "->" MAYBE_NONE("?S?GHANDLE")); ]]]*/
#define LIBWIN32_ENUMPROCESSMODULES_DEF          DEX_MEMBER_F("EnumProcessModules", &libwin32_EnumProcessModules, DEXSYM_READONLY, "(hProcess:?X3?Dint?DFile?Ewin32:HANDLE,dwFilterFlag:?Dint=!GLIST_MODULES_DEFAULT)->?S?GHANDLE"),
#define LIBWIN32_ENUMPROCESSMODULES_DEF_DOC(doc) DEX_MEMBER_F("EnumProcessModules", &libwin32_EnumProcessModules, DEXSYM_READONLY, "(hProcess:?X3?Dint?DFile?Ewin32:HANDLE,dwFilterFlag:?Dint=!GLIST_MODULES_DEFAULT)->?S?GHANDLE\n" doc),
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_EnumProcessModules_f_impl(HANDLE hProcess, DWORD dwFilterFlag);
#ifndef DEFINED_kwlist__hProcess_dwFilterFlag
#define DEFINED_kwlist__hProcess_dwFilterFlag
PRIVATE DEFINE_KWLIST(kwlist__hProcess_dwFilterFlag, { KEX("hProcess", 0x97c84287, 0x1d63e586eef1387e), KEX("dwFilterFlag", 0x733d5847, 0x534590733e33d2c9), KEND });
#endif /* !DEFINED_kwlist__hProcess_dwFilterFlag */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_EnumProcessModules_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *raw_hProcess;
		DWORD dwFilterFlag;
	} args;
	HANDLE hProcess;
	args.dwFilterFlag = LIST_MODULES_DEFAULT;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__hProcess_dwFilterFlag, "o|" UNPu32 ":EnumProcessModules", &args))
		goto err;
	if unlikely(DeeNTSystem_TryGetHandle(args.raw_hProcess, (void **)&hProcess))
		goto err;
	return libwin32_EnumProcessModules_f_impl(hProcess, args.dwFilterFlag);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_EnumProcessModules, &libwin32_EnumProcessModules_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_EnumProcessModules_f_impl(HANDLE hProcess, DWORD dwFilterFlag)
/*[[[end]]]*/
{
	BOOL bOk;
	HMODULE *phModules;
	DWORD cbNeededModules, cbAllocModules;
	LOAD_PSAPI_FUNCTION(err, BOOL, WINAPI, EnumProcessModulesEx,
	                    (HANDLE hProcess, HMODULE * lphModule,
	                     DWORD cb, LPDWORD lpcbNeeded, DWORD dwFilterFlag))
	cbAllocModules = 64;
	phModules = (HMODULE *)Dee_TryMallocc(cbAllocModules, sizeof(HMODULE));
	if unlikely(!phModules) {
		cbAllocModules = 1;
		phModules = (HMODULE *)Dee_Mallocc(cbAllocModules, sizeof(HMODULE));
		if unlikely(!phModules)
			goto err;
	}
again:
	DBG_ALIGNMENT_DISABLE();
	bOk = (*EnumProcessModulesEx)(hProcess,
	                              phModules,
	                              cbAllocModules * sizeof(HMODULE),
	                              &cbNeededModules,
	                              dwFilterFlag);
	if (!bOk) {
		DWORD dwError;
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(dwError)) {
			if (DeeThread_CheckInterrupt())
				goto err_modules;
			goto again;
		}
		Dee_Free(phModules);
		RETURN_ERROR(dwError,
		             "Failed to query process modules (hProcess: %p, dwFilterFlag: %#" PRFx32 ")",
		             hProcess, dwFilterFlag);
	}
	DBG_ALIGNMENT_ENABLE();
	cbNeededModules /= sizeof(HMODULE);
	if (cbNeededModules > cbAllocModules) {
		HMODULE *phNewModules;
		phNewModules = (HMODULE *)Dee_Reallocc(phModules, cbNeededModules, sizeof(HMODULE));
		if unlikely(!phNewModules)
			goto err_modules;
		phModules      = phNewModules;
		cbAllocModules = cbNeededModules;
		goto again;
	}
	{
		/* Pack the modules into a Tuple of HANDLE objects. */
		DWORD i;
		DREF DeeTupleObject *result;
		result = DeeTuple_NewUninitialized(cbNeededModules);
		if unlikely(!result)
			goto err_modules;
		for (i = 0; i < cbNeededModules; ++i) {
			DREF DeeObject *handle;
			handle = libwin32_CreateHandle(phModules[i]);
			if unlikely(!handle)
				goto err_modules_result;
			DeeTuple_SET(result, i, handle);
		}
		Dee_Free(phModules);
		return (DREF DeeObject *)result;
err_modules_result:
		while (i--) {
#ifdef libwin32_CreateHandle_ALWAYS_RETURNS_UNIQUE_REFERENCE
			Dee_DecrefDokill(DeeTuple_GET(result, i));
#else /* libwin32_CreateHandle_ALWAYS_RETURNS_UNIQUE_REFERENCE */
			Dee_Decref_likely(DeeTuple_GET(result, i));
#endif /* !libwin32_CreateHandle_ALWAYS_RETURNS_UNIQUE_REFERENCE */
		}
		DeeTuple_FreeUninitialized(result);
	}
err_modules:
	Dee_Free(phModules);
err:
	return NULL;
}



/*[[[deemon import("rt.gen.dexutils").gw("EnumProcesses",
     "->" MAYBE_NONE("?S?Dint")); ]]]*/
#define LIBWIN32_ENUMPROCESSES_DEF          DEX_MEMBER_F("EnumProcesses", &libwin32_EnumProcesses, DEXSYM_READONLY, "->?S?Dint"),
#define LIBWIN32_ENUMPROCESSES_DEF_DOC(doc) DEX_MEMBER_F("EnumProcesses", &libwin32_EnumProcesses, DEXSYM_READONLY, "->?S?Dint\n" doc),
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_EnumProcesses_f_impl(void);
PRIVATE DEFINE_CMETHOD0(libwin32_EnumProcesses, &libwin32_EnumProcesses_f_impl, METHOD_FNORMAL);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_EnumProcesses_f_impl(void)
/*[[[end]]]*/
{
	BOOL bOk;
	DWORD *pidProcesses;
	DWORD cbNeededProcesses, cbAllocProcesses;
	LOAD_PSAPI_FUNCTION(err, BOOL, WINAPI, EnumProcesses,
	                    (DWORD *lpidProcess, DWORD cb, LPDWORD lpcbNeeded))
	cbAllocProcesses = 128;
	pidProcesses = (DWORD *)Dee_TryMallocc(cbAllocProcesses, sizeof(DWORD));
	if unlikely(!pidProcesses) {
		cbAllocProcesses = 1;
		pidProcesses = (DWORD *)Dee_Mallocc(cbAllocProcesses, sizeof(DWORD));
		if unlikely(!pidProcesses)
			goto err;
	}
again:
	DBG_ALIGNMENT_DISABLE();
	bOk = (*EnumProcesses)(pidProcesses,
	                       cbAllocProcesses * sizeof(DWORD),
	                       &cbNeededProcesses);
	if (!bOk) {
		DWORD dwError;
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(dwError)) {
			if (DeeThread_CheckInterrupt())
				goto err_modules;
			goto again;
		}
		Dee_Free(pidProcesses);
		RETURN_ERROR(dwError, "Failed to enumerate processes");
	}
	DBG_ALIGNMENT_ENABLE();
	cbNeededProcesses /= sizeof(DWORD);
	if (cbNeededProcesses >= cbAllocProcesses) {
		DWORD *pidNewProcesses;
		cbNeededProcesses = cbAllocProcesses * 2;
		pidNewProcesses = (DWORD *)Dee_Reallocc(pidProcesses, cbNeededProcesses, sizeof(DWORD));
		if unlikely(!pidNewProcesses)
			goto err_modules;
		pidProcesses     = pidNewProcesses;
		cbAllocProcesses = cbNeededProcesses;
		goto again;
	}
	{
		/* Pack the modules into a Tuple of HANDLE objects. */
		DWORD i;
		DREF DeeTupleObject *result;
		result = DeeTuple_NewUninitialized(cbNeededProcesses);
		if unlikely(!result)
			goto err_modules;
		for (i = 0; i < cbNeededProcesses; ++i) {
			DREF DeeObject *pid_obj;
			pid_obj = DeeInt_NewUInt32(pidProcesses[i]);
			if unlikely(!pid_obj)
				goto err_pids_result;
			DeeTuple_SET(result, i, pid_obj);
		}
		Dee_Free(pidProcesses);
		return (DREF DeeObject *)result;
err_pids_result:
		Dee_Decrefv_likely(DeeTuple_ELEM(result), i);
		DeeTuple_FreeUninitialized(result);
	}
err_modules:
	Dee_Free(pidProcesses);
err:
	return NULL;
}



/*[[[deemon import("rt.gen.dexutils").gw("GetProcessImageFileName",
      "hProcess:nt:HANDLE"
     "->" MAYBE_NONE("?Dstring")); ]]]*/
#define LIBWIN32_GETPROCESSIMAGEFILENAME_DEF          DEX_MEMBER_F("GetProcessImageFileName", &libwin32_GetProcessImageFileName, DEXSYM_READONLY, "(hProcess:?X3?Dint?DFile?Ewin32:HANDLE)->?Dstring"),
#define LIBWIN32_GETPROCESSIMAGEFILENAME_DEF_DOC(doc) DEX_MEMBER_F("GetProcessImageFileName", &libwin32_GetProcessImageFileName, DEXSYM_READONLY, "(hProcess:?X3?Dint?DFile?Ewin32:HANDLE)->?Dstring\n" doc),
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetProcessImageFileName_f_impl(HANDLE hProcess);
#ifndef DEFINED_kwlist__hProcess
#define DEFINED_kwlist__hProcess
PRIVATE DEFINE_KWLIST(kwlist__hProcess, { KEX("hProcess", 0x97c84287, 0x1d63e586eef1387e), KEND });
#endif /* !DEFINED_kwlist__hProcess */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetProcessImageFileName_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *raw_hProcess;
	} args;
	HANDLE hProcess;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__hProcess, "o:GetProcessImageFileName", &args))
		goto err;
	if unlikely(DeeNTSystem_TryGetHandle(args.raw_hProcess, (void **)&hProcess))
		goto err;
	return libwin32_GetProcessImageFileName_f_impl(hProcess);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_GetProcessImageFileName, &libwin32_GetProcessImageFileName_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetProcessImageFileName_f_impl(HANDLE hProcess)
/*[[[end]]]*/
{
	LPWSTR lpBuffer, lpNewBuffer;
	DWORD dwBufSize = PATH_MAX, dwError;
	LOAD_PSAPI_FUNCTION(err, DWORD, WINAPI, GetProcessImageFileNameW,
	                    (HANDLE hProcess, LPWSTR lpImageFileName, DWORD nSize))
	lpBuffer = DeeString_NewWideBuffer(dwBufSize);
	if unlikely(!lpBuffer)
		goto err;
again:
	for (;;) {
		DBG_ALIGNMENT_DISABLE();
		dwError = (*GetProcessImageFileNameW)(hProcess, lpBuffer, dwBufSize + 1);
		if (!dwError) {
			dwError = GetLastError();
			DBG_ALIGNMENT_ENABLE();
			if (DeeNTSystem_IsIntr(dwError)) {
				if (DeeThread_CheckInterrupt())
					goto err_result;
				goto again;
			}
			DeeString_FreeWideBuffer(lpBuffer);
			RETURN_ERROR(dwError,
			             "Failed to determine the process image filename of %p",
			             hProcess);
		}
		DBG_ALIGNMENT_ENABLE();
		if (dwError <= dwBufSize)
			break;
		/* Resize to fit. */
		lpNewBuffer = DeeString_ResizeWideBuffer(lpBuffer, dwError);
		if unlikely(!lpNewBuffer)
			goto err_result;
		lpBuffer  = lpNewBuffer;
		dwBufSize = dwError;
	}
	lpBuffer = DeeString_TruncateWideBuffer(lpBuffer, dwError);
	return DeeString_PackWideBuffer(lpBuffer, STRING_ERROR_FREPLAC);
err_result:
	DeeString_FreeWideBuffer(lpBuffer);
err:
	return NULL;
}



/*[[[deemon import("rt.gen.dexutils").gw("GetModuleBaseName",
      "hProcess:nt:HANDLE"
      ",hModule:nt:HANDLE"
     "->" MAYBE_NONE("?Dstring")); ]]]*/
#define LIBWIN32_GETMODULEBASENAME_DEF          DEX_MEMBER_F("GetModuleBaseName", &libwin32_GetModuleBaseName, DEXSYM_READONLY, "(hProcess:?X3?Dint?DFile?Ewin32:HANDLE,hModule:?X3?Dint?DFile?Ewin32:HANDLE)->?Dstring"),
#define LIBWIN32_GETMODULEBASENAME_DEF_DOC(doc) DEX_MEMBER_F("GetModuleBaseName", &libwin32_GetModuleBaseName, DEXSYM_READONLY, "(hProcess:?X3?Dint?DFile?Ewin32:HANDLE,hModule:?X3?Dint?DFile?Ewin32:HANDLE)->?Dstring\n" doc),
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetModuleBaseName_f_impl(HANDLE hProcess, HANDLE hModule);
#ifndef DEFINED_kwlist__hProcess_hModule
#define DEFINED_kwlist__hProcess_hModule
PRIVATE DEFINE_KWLIST(kwlist__hProcess_hModule, { KEX("hProcess", 0x97c84287, 0x1d63e586eef1387e), KEX("hModule", 0xa2a7467f, 0x6caab631b175fa52), KEND });
#endif /* !DEFINED_kwlist__hProcess_hModule */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetModuleBaseName_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *raw_hProcess;
		DeeObject *raw_hModule;
	} args;
	HANDLE hProcess;
	HANDLE hModule;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__hProcess_hModule, "oo:GetModuleBaseName", &args))
		goto err;
	if unlikely(DeeNTSystem_TryGetHandle(args.raw_hProcess, (void **)&hProcess))
		goto err;
	if unlikely(DeeNTSystem_TryGetHandle(args.raw_hModule, (void **)&hModule))
		goto err;
	return libwin32_GetModuleBaseName_f_impl(hProcess, hModule);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_GetModuleBaseName, &libwin32_GetModuleBaseName_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetModuleBaseName_f_impl(HANDLE hProcess, HANDLE hModule)
/*[[[end]]]*/
{
	LPWSTR lpBuffer, lpNewBuffer;
	DWORD dwBufSize = PATH_MAX, dwError;
	LOAD_PSAPI_FUNCTION(err, DWORD, WINAPI, GetModuleBaseNameW,
	                    (HANDLE hProcess, HANDLE hModule, LPWSTR lpImageFileName, DWORD nSize))
	lpBuffer = DeeString_NewWideBuffer(dwBufSize);
	if unlikely(!lpBuffer)
		goto err;
again:
	for (;;) {
		DBG_ALIGNMENT_DISABLE();
		dwError = (*GetModuleBaseNameW)(hProcess, hModule, lpBuffer, dwBufSize + 1);
		if (!dwError) {
			dwError = GetLastError();
			DBG_ALIGNMENT_ENABLE();
			if (DeeNTSystem_IsIntr(dwError)) {
				if (DeeThread_CheckInterrupt())
					goto err_result;
				goto again;
			}
			DeeString_FreeWideBuffer(lpBuffer);
			RETURN_ERROR(dwError,
			             "Failed to determine the module base name of %p in process %p",
			             hModule, hProcess);
		}
		DBG_ALIGNMENT_ENABLE();
		if (dwError <= dwBufSize)
			break;
		/* Resize to fit. */
		lpNewBuffer = DeeString_ResizeWideBuffer(lpBuffer, dwError);
		if unlikely(!lpNewBuffer)
			goto err_result;
		lpBuffer  = lpNewBuffer;
		dwBufSize = dwError;
	}
	lpBuffer = DeeString_TruncateWideBuffer(lpBuffer, dwError);
	return DeeString_PackWideBuffer(lpBuffer, STRING_ERROR_FREPLAC);
err_result:
	DeeString_FreeWideBuffer(lpBuffer);
err:
	return NULL;
}



/*[[[deemon import("rt.gen.dexutils").gw("GetModuleFileNameEx",
      "hProcess:nt:HANDLE"
      ",hModule:nt:HANDLE"
     "->" MAYBE_NONE("?Dstring")); ]]]*/
#define LIBWIN32_GETMODULEFILENAMEEX_DEF          DEX_MEMBER_F("GetModuleFileNameEx", &libwin32_GetModuleFileNameEx, DEXSYM_READONLY, "(hProcess:?X3?Dint?DFile?Ewin32:HANDLE,hModule:?X3?Dint?DFile?Ewin32:HANDLE)->?Dstring"),
#define LIBWIN32_GETMODULEFILENAMEEX_DEF_DOC(doc) DEX_MEMBER_F("GetModuleFileNameEx", &libwin32_GetModuleFileNameEx, DEXSYM_READONLY, "(hProcess:?X3?Dint?DFile?Ewin32:HANDLE,hModule:?X3?Dint?DFile?Ewin32:HANDLE)->?Dstring\n" doc),
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetModuleFileNameEx_f_impl(HANDLE hProcess, HANDLE hModule);
#ifndef DEFINED_kwlist__hProcess_hModule
#define DEFINED_kwlist__hProcess_hModule
PRIVATE DEFINE_KWLIST(kwlist__hProcess_hModule, { KEX("hProcess", 0x97c84287, 0x1d63e586eef1387e), KEX("hModule", 0xa2a7467f, 0x6caab631b175fa52), KEND });
#endif /* !DEFINED_kwlist__hProcess_hModule */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetModuleFileNameEx_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *raw_hProcess;
		DeeObject *raw_hModule;
	} args;
	HANDLE hProcess;
	HANDLE hModule;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__hProcess_hModule, "oo:GetModuleFileNameEx", &args))
		goto err;
	if unlikely(DeeNTSystem_TryGetHandle(args.raw_hProcess, (void **)&hProcess))
		goto err;
	if unlikely(DeeNTSystem_TryGetHandle(args.raw_hModule, (void **)&hModule))
		goto err;
	return libwin32_GetModuleFileNameEx_f_impl(hProcess, hModule);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_GetModuleFileNameEx, &libwin32_GetModuleFileNameEx_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetModuleFileNameEx_f_impl(HANDLE hProcess, HANDLE hModule)
/*[[[end]]]*/
{
	LPWSTR lpBuffer, lpNewBuffer;
	DWORD dwBufSize = PATH_MAX, dwError;
	LOAD_PSAPI_FUNCTION(err, DWORD, WINAPI, GetModuleFileNameExW,
	                    (HANDLE hProcess, HANDLE hModule, LPWSTR lpImageFileName, DWORD nSize))
	lpBuffer = DeeString_NewWideBuffer(dwBufSize);
	if unlikely(!lpBuffer)
		goto err;
again:
	for (;;) {
		DBG_ALIGNMENT_DISABLE();
		dwError = (*GetModuleFileNameExW)(hProcess, hModule, lpBuffer, dwBufSize + 1);
		if (!dwError) {
			dwError = GetLastError();
			DBG_ALIGNMENT_ENABLE();
			if (DeeNTSystem_IsIntr(dwError)) {
				if (DeeThread_CheckInterrupt())
					goto err_result;
				goto again;
			}
			DeeString_FreeWideBuffer(lpBuffer);
			RETURN_ERROR(dwError,
			             "Failed to determine the module filename of %p in process %p",
			             hModule, hProcess);
		}
		DBG_ALIGNMENT_ENABLE();
		if (dwError <= dwBufSize)
			break;
		/* Resize to fit. */
		lpNewBuffer = DeeString_ResizeWideBuffer(lpBuffer, dwError);
		if unlikely(!lpNewBuffer)
			goto err_result;
		lpBuffer  = lpNewBuffer;
		dwBufSize = dwError;
	}
	lpBuffer = DeeString_TruncateWideBuffer(lpBuffer, dwError);
	return DeeString_PackWideBuffer(lpBuffer, STRING_ERROR_FREPLAC);
err_result:
	DeeString_FreeWideBuffer(lpBuffer);
err:
	return NULL;
}


/*[[[deemon import("rt.gen.dexutils").gw("TerminateProcess",
      "hProcess:nt:HANDLE"
      ",uExitCode:nt:UINT"
     "->" ERROR_OR_BOOL); ]]]*/
#define LIBWIN32_TERMINATEPROCESS_DEF          DEX_MEMBER_F("TerminateProcess", &libwin32_TerminateProcess, DEXSYM_READONLY, "(hProcess:?X3?Dint?DFile?Ewin32:HANDLE,uExitCode:?Dint)"),
#define LIBWIN32_TERMINATEPROCESS_DEF_DOC(doc) DEX_MEMBER_F("TerminateProcess", &libwin32_TerminateProcess, DEXSYM_READONLY, "(hProcess:?X3?Dint?DFile?Ewin32:HANDLE,uExitCode:?Dint)\n" doc),
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_TerminateProcess_f_impl(HANDLE hProcess, UINT uExitCode);
#ifndef DEFINED_kwlist__hProcess_uExitCode
#define DEFINED_kwlist__hProcess_uExitCode
PRIVATE DEFINE_KWLIST(kwlist__hProcess_uExitCode, { KEX("hProcess", 0x97c84287, 0x1d63e586eef1387e), KEX("uExitCode", 0x965bc487, 0xd69502f6ab8371b1), KEND });
#endif /* !DEFINED_kwlist__hProcess_uExitCode */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_TerminateProcess_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *raw_hProcess;
		UINT uExitCode;
	} args;
	HANDLE hProcess;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__hProcess_uExitCode, "o" UNPu32 ":TerminateProcess", &args))
		goto err;
	if unlikely(DeeNTSystem_TryGetHandle(args.raw_hProcess, (void **)&hProcess))
		goto err;
	return libwin32_TerminateProcess_f_impl(hProcess, args.uExitCode);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_TerminateProcess, &libwin32_TerminateProcess_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_TerminateProcess_f_impl(HANDLE hProcess, UINT uExitCode)
/*[[[end]]]*/
{
	BOOL bOk;
again:
	DBG_ALIGNMENT_DISABLE();
	bOk = TerminateProcess(hProcess, uExitCode);
	if (!bOk) {
		DWORD dwError;
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(dwError)) {
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		RETURN_ERROR_OR_FALSE(dwError,
		                      "Failed to terminate process %p (uExitCode: %" PRFu32 ")",
		                      hProcess, uExitCode);
	}
	DBG_ALIGNMENT_ENABLE();
	RETURN_SUCCESS_OR_TRUE;
err:
	return NULL;
}


/*[[[deemon import("rt.gen.dexutils").gw("TerminateThread",
      "hThread:nt:HANDLE"
      ",dwExitCode:nt:DWORD"
     "->" ERROR_OR_BOOL); ]]]*/
#define LIBWIN32_TERMINATETHREAD_DEF          DEX_MEMBER_F("TerminateThread", &libwin32_TerminateThread, DEXSYM_READONLY, "(hThread:?X3?Dint?DFile?Ewin32:HANDLE,dwExitCode:?Dint)"),
#define LIBWIN32_TERMINATETHREAD_DEF_DOC(doc) DEX_MEMBER_F("TerminateThread", &libwin32_TerminateThread, DEXSYM_READONLY, "(hThread:?X3?Dint?DFile?Ewin32:HANDLE,dwExitCode:?Dint)\n" doc),
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_TerminateThread_f_impl(HANDLE hThread, DWORD dwExitCode);
#ifndef DEFINED_kwlist__hThread_dwExitCode
#define DEFINED_kwlist__hThread_dwExitCode
PRIVATE DEFINE_KWLIST(kwlist__hThread_dwExitCode, { KEX("hThread", 0xcdcf6035, 0xfb163b09905ac0ff), KEX("dwExitCode", 0x5729c92b, 0xc17df058f3ba4297), KEND });
#endif /* !DEFINED_kwlist__hThread_dwExitCode */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_TerminateThread_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *raw_hThread;
		DWORD dwExitCode;
	} args;
	HANDLE hThread;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__hThread_dwExitCode, "o" UNPu32 ":TerminateThread", &args))
		goto err;
	if unlikely(DeeNTSystem_TryGetHandle(args.raw_hThread, (void **)&hThread))
		goto err;
	return libwin32_TerminateThread_f_impl(hThread, args.dwExitCode);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_TerminateThread, &libwin32_TerminateThread_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_TerminateThread_f_impl(HANDLE hThread, DWORD dwExitCode)
/*[[[end]]]*/
{
	BOOL bOk;
again:
	DBG_ALIGNMENT_DISABLE();
	bOk = TerminateThread(hThread, dwExitCode);
	if (!bOk) {
		DWORD dwError;
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(dwError)) {
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		RETURN_ERROR_OR_FALSE(dwError,
		                      "Failed to terminate thread %p (dwExitCode: %" PRFu32 ")",
		                      hThread, dwExitCode);
	}
	DBG_ALIGNMENT_ENABLE();
	RETURN_SUCCESS_OR_TRUE;
err:
	return NULL;
}


/*[[[deemon import("rt.gen.dexutils").gw("SuspendThread",
      "hThread:nt:HANDLE"
     "->" MAYBE_NONE("?Dint")); ]]]*/
#define LIBWIN32_SUSPENDTHREAD_DEF          DEX_MEMBER_F("SuspendThread", &libwin32_SuspendThread, DEXSYM_READONLY, "(hThread:?X3?Dint?DFile?Ewin32:HANDLE)->?Dint"),
#define LIBWIN32_SUSPENDTHREAD_DEF_DOC(doc) DEX_MEMBER_F("SuspendThread", &libwin32_SuspendThread, DEXSYM_READONLY, "(hThread:?X3?Dint?DFile?Ewin32:HANDLE)->?Dint\n" doc),
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_SuspendThread_f_impl(HANDLE hThread);
#ifndef DEFINED_kwlist__hThread
#define DEFINED_kwlist__hThread
PRIVATE DEFINE_KWLIST(kwlist__hThread, { KEX("hThread", 0xcdcf6035, 0xfb163b09905ac0ff), KEND });
#endif /* !DEFINED_kwlist__hThread */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_SuspendThread_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *raw_hThread;
	} args;
	HANDLE hThread;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__hThread, "o:SuspendThread", &args))
		goto err;
	if unlikely(DeeNTSystem_TryGetHandle(args.raw_hThread, (void **)&hThread))
		goto err;
	return libwin32_SuspendThread_f_impl(hThread);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_SuspendThread, &libwin32_SuspendThread_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_SuspendThread_f_impl(HANDLE hThread)
/*[[[end]]]*/
{
	DWORD dwResult;
again:
	DBG_ALIGNMENT_DISABLE();
	dwResult = SuspendThread(hThread);
	if (dwResult == (DWORD)-1) {
		DWORD dwError;
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(dwError)) {
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		RETURN_ERROR(dwError,
		             "Failed to suspend thread %p",
		             hThread);
	}
	DBG_ALIGNMENT_ENABLE();
	return DeeInt_NewUInt32(dwResult);
err:
	return NULL;
}


/*[[[deemon import("rt.gen.dexutils").gw("ResumeThread",
      "hThread:nt:HANDLE"
     "->" MAYBE_NONE("?Dint")); ]]]*/
#define LIBWIN32_RESUMETHREAD_DEF          DEX_MEMBER_F("ResumeThread", &libwin32_ResumeThread, DEXSYM_READONLY, "(hThread:?X3?Dint?DFile?Ewin32:HANDLE)->?Dint"),
#define LIBWIN32_RESUMETHREAD_DEF_DOC(doc) DEX_MEMBER_F("ResumeThread", &libwin32_ResumeThread, DEXSYM_READONLY, "(hThread:?X3?Dint?DFile?Ewin32:HANDLE)->?Dint\n" doc),
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_ResumeThread_f_impl(HANDLE hThread);
#ifndef DEFINED_kwlist__hThread
#define DEFINED_kwlist__hThread
PRIVATE DEFINE_KWLIST(kwlist__hThread, { KEX("hThread", 0xcdcf6035, 0xfb163b09905ac0ff), KEND });
#endif /* !DEFINED_kwlist__hThread */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_ResumeThread_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *raw_hThread;
	} args;
	HANDLE hThread;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__hThread, "o:ResumeThread", &args))
		goto err;
	if unlikely(DeeNTSystem_TryGetHandle(args.raw_hThread, (void **)&hThread))
		goto err;
	return libwin32_ResumeThread_f_impl(hThread);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_ResumeThread, &libwin32_ResumeThread_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_ResumeThread_f_impl(HANDLE hThread)
/*[[[end]]]*/
{
	DWORD dwResult;
again:
	DBG_ALIGNMENT_DISABLE();
	dwResult = ResumeThread(hThread);
	if (dwResult == (DWORD)-1) {
		DWORD dwError;
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(dwError)) {
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		RETURN_ERROR(dwError,
		             "Failed to resume thread %p",
		             hThread);
	}
	DBG_ALIGNMENT_ENABLE();
	return DeeInt_NewUInt32(dwResult);
err:
	return NULL;
}


PRIVATE HMODULE libwin32_NtDllModule = NULL;
PRIVATE WCHAR const wNtDll[]    = { 'N', 'T', 'D', 'L', 'L', 0 };
PRIVATE WCHAR const wNtDllDll[] = { 'N', 't', 'D', 'l', 'l', '.', 'd', 'l', 'l', 0 };

/* @return: NULL: An error was thrown */
PRIVATE HMODULE DCALL libwin32_GetNtDllModule(void) {
	HMODULE hResult;
	hResult = atomic_read(&libwin32_NtDllModule);
	if (hResult != NULL)
		return hResult;
	DBG_ALIGNMENT_DISABLE();
	hResult = GetModuleHandleW(wNtDll);
	if (hResult != NULL)
		goto done_set;
	hResult = LoadLibraryW(wNtDllDll);
	if (hResult != NULL)
		goto done_set;
	DBG_ALIGNMENT_ENABLE();
	DeeNTSystem_ThrowErrorf(NULL, GetLastError(),
	                        "Failed to load \"NtDll.dll\"");
done_set:
	DBG_ALIGNMENT_ENABLE();
	atomic_write(&libwin32_NtDllModule, hResult);
	return hResult;
}

/* @return: NULL: An error was thrown */
PRIVATE FARPROC DCALL libwin32_GetNtDllProc(char const *__restrict name) {
	HMODULE hMod;
	FARPROC pResult = NULL;
	hMod = libwin32_GetNtDllModule();
	if unlikely(!hMod)
		goto done;
	DBG_ALIGNMENT_DISABLE();
	pResult = GetProcAddress(hMod, name);
	DBG_ALIGNMENT_ENABLE();
	if (pResult)
		goto done;
	{
		DWORD dwError;
		DBG_ALIGNMENT_DISABLE();
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		DeeNTSystem_ThrowErrorf(NULL, dwError,
		                        "Failed to locate symbol %q in \"NtDll.dll\"",
		                        name);
	}
done:
	return pResult;
}

#define LOAD_NTDLL_FUNCTION(err_label, returnType, cc, name, args) \
	typedef returnType (cc *LP_psapi_##name) args;                 \
	PRIVATE LP_psapi_##name name = NULL;                           \
	if (!name) {                                                   \
		name = (LP_psapi_##name)libwin32_GetNtDllProc(#name);      \
		if unlikely(!name)                                         \
			goto err_label;                                        \
	}
#ifndef NTAPI
#define NTAPI WINAPI
#endif /* !NTAPI */

typedef ULONG (NTAPI *LPRTLNTSTATUSTODOSERROR)(NTSTATUS Status);
PRIVATE LPRTLNTSTATUSTODOSERROR pdyn_RtlNtStatusToDosError = NULL;
PRIVATE LPRTLNTSTATUSTODOSERROR DCALL get_RtlNtStatusToDosError(void) {
	if (!pdyn_RtlNtStatusToDosError)
		pdyn_RtlNtStatusToDosError = (LPRTLNTSTATUSTODOSERROR)libwin32_GetNtDllProc("RtlNtStatusToDosError");
	return pdyn_RtlNtStatusToDosError;
}

#define LOAD_NTDLL_RTLNTSTATUSTODOSERROR(err_label)      \
	LPRTLNTSTATUSTODOSERROR RtlNtStatusToDosError;       \
	RtlNtStatusToDosError = get_RtlNtStatusToDosError(); \
	if unlikely(!RtlNtStatusToDosError)                  \
		goto err_label;


/*[[[deemon import("rt.gen.dexutils").gw("NtQueryInformationProcess",
       "ProcessHandle:nt:HANDLE"
      ",ProcessInformationClass:u"
      ",ProcessInformationLength:c:size_t=64"
     "->" MAYBE_NONE("?DBytes")); ]]]*/
#define LIBWIN32_NTQUERYINFORMATIONPROCESS_DEF          DEX_MEMBER_F("NtQueryInformationProcess", &libwin32_NtQueryInformationProcess, DEXSYM_READONLY, "(ProcessHandle:?X3?Dint?DFile?Ewin32:HANDLE,ProcessInformationClass:?Dint,ProcessInformationLength=!64)->?DBytes"),
#define LIBWIN32_NTQUERYINFORMATIONPROCESS_DEF_DOC(doc) DEX_MEMBER_F("NtQueryInformationProcess", &libwin32_NtQueryInformationProcess, DEXSYM_READONLY, "(ProcessHandle:?X3?Dint?DFile?Ewin32:HANDLE,ProcessInformationClass:?Dint,ProcessInformationLength=!64)->?DBytes\n" doc),
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_NtQueryInformationProcess_f_impl(HANDLE ProcessHandle, unsigned int ProcessInformationClass, size_t ProcessInformationLength);
#ifndef DEFINED_kwlist__ProcessHandle_ProcessInformationClass_ProcessInformationLength
#define DEFINED_kwlist__ProcessHandle_ProcessInformationClass_ProcessInformationLength
PRIVATE DEFINE_KWLIST(kwlist__ProcessHandle_ProcessInformationClass_ProcessInformationLength, { KEX("ProcessHandle", 0xe9d53f39, 0x498bdf36d182b8b7), KEX("ProcessInformationClass", 0x63f8f187, 0x24278c35a154af4d), KEX("ProcessInformationLength", 0x1b3919a2, 0xc0e52c1be0a4d491), KEND });
#endif /* !DEFINED_kwlist__ProcessHandle_ProcessInformationClass_ProcessInformationLength */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_NtQueryInformationProcess_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *raw_ProcessHandle;
		unsigned int ProcessInformationClass;
		size_t ProcessInformationLength;
	} args;
	HANDLE ProcessHandle;
	args.ProcessInformationLength = 64;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__ProcessHandle_ProcessInformationClass_ProcessInformationLength, "ou|" UNPuSIZ ":NtQueryInformationProcess", &args))
		goto err;
	if unlikely(DeeNTSystem_TryGetHandle(args.raw_ProcessHandle, (void **)&ProcessHandle))
		goto err;
	return libwin32_NtQueryInformationProcess_f_impl(ProcessHandle, args.ProcessInformationClass, args.ProcessInformationLength);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_NtQueryInformationProcess, &libwin32_NtQueryInformationProcess_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_NtQueryInformationProcess_f_impl(HANDLE ProcessHandle, unsigned int ProcessInformationClass, size_t ProcessInformationLength)
/*[[[end]]]*/
{
	NTSTATUS nsResult;
	ULONG ulReturnLength;
	DREF DeeBytesObject *result;
	LOAD_NTDLL_FUNCTION(err, NTSTATUS, NTAPI, NtQueryInformationProcess,
	                    (HANDLE ProcessHandle, /*PROCESSINFOCLASS*/ int ProcessInformationClass,
	                     PVOID ProcessInformation, ULONG ProcessInformationLength,
	                     PULONG ReturnLength))
	result = (DREF DeeBytesObject *)DeeBytes_NewBufferUninitialized(ProcessInformationLength);
	if unlikely(!result)
		goto err;
again:
	DBG_ALIGNMENT_DISABLE();
	ulReturnLength = 0;
	nsResult = (*NtQueryInformationProcess)(ProcessHandle,
	                                        (/*PROCESSINFOCLASS*/ int)ProcessInformationClass,
	                                        DeeBytes_DATA(result), (ULONG)DeeBytes_SIZE(result),
	                                        &ulReturnLength);
	DBG_ALIGNMENT_ENABLE();
	if (NT_ERROR(nsResult)) {
		DWORD dwError;
		LOAD_NTDLL_RTLNTSTATUSTODOSERROR(err_r);
		DBG_ALIGNMENT_DISABLE();
		dwError = (*RtlNtStatusToDosError)(nsResult);
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(dwError)) {
			if (DeeThread_CheckInterrupt())
				goto err_r;
			goto again;
		}
		if (DeeNTSystem_IsBufferTooSmall(dwError)) {
			if (ulReturnLength <= DeeBytes_SIZE(result))
				ulReturnLength = (ULONG)DeeBytes_SIZE(result) * 2;
			goto resize_buffer;
		}
		if (ulReturnLength == 0 || ulReturnLength >= 0x01000000) {
			Dee_Decref_likely(result);
			RETURN_ERROR(dwError,
			             "Failed to query information for process %p (ProcessInformationClass: %u)",
			             ProcessHandle, ProcessInformationClass);
		}
	}
	if (ulReturnLength > DeeBytes_SIZE(result)) {
		DREF DeeBytesObject *new_result;
resize_buffer:
		new_result = (DREF DeeBytesObject *)DeeBytes_ResizeBuffer((DeeObject *)result,
		                                                          ulReturnLength);
		if unlikely(!new_result)
			goto err_r;
		result = new_result;
		goto again;
	} else if (ulReturnLength < DeeBytes_SIZE(result)) {
		result = (DREF DeeBytesObject *)DeeBytes_TruncateBuffer((DeeObject *)result,
		                                                        ulReturnLength);
	}
	return (DREF DeeObject *)result;
err_r:
	Dee_Decref_likely(result);
err:
	return NULL;
}


/*[[[deemon import("rt.gen.dexutils").gw("NtWow64QueryInformationProcess64",
       "ProcessHandle:nt:HANDLE"
      ",ProcessInformationClass:u"
      ",ProcessInformationLength:c:size_t=64"
     "->" MAYBE_NONE("?DBytes")); ]]]*/
#define LIBWIN32_NTWOW64QUERYINFORMATIONPROCESS64_DEF          DEX_MEMBER_F("NtWow64QueryInformationProcess64", &libwin32_NtWow64QueryInformationProcess64, DEXSYM_READONLY, "(ProcessHandle:?X3?Dint?DFile?Ewin32:HANDLE,ProcessInformationClass:?Dint,ProcessInformationLength=!64)->?DBytes"),
#define LIBWIN32_NTWOW64QUERYINFORMATIONPROCESS64_DEF_DOC(doc) DEX_MEMBER_F("NtWow64QueryInformationProcess64", &libwin32_NtWow64QueryInformationProcess64, DEXSYM_READONLY, "(ProcessHandle:?X3?Dint?DFile?Ewin32:HANDLE,ProcessInformationClass:?Dint,ProcessInformationLength=!64)->?DBytes\n" doc),
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_NtWow64QueryInformationProcess64_f_impl(HANDLE ProcessHandle, unsigned int ProcessInformationClass, size_t ProcessInformationLength);
#ifndef DEFINED_kwlist__ProcessHandle_ProcessInformationClass_ProcessInformationLength
#define DEFINED_kwlist__ProcessHandle_ProcessInformationClass_ProcessInformationLength
PRIVATE DEFINE_KWLIST(kwlist__ProcessHandle_ProcessInformationClass_ProcessInformationLength, { KEX("ProcessHandle", 0xe9d53f39, 0x498bdf36d182b8b7), KEX("ProcessInformationClass", 0x63f8f187, 0x24278c35a154af4d), KEX("ProcessInformationLength", 0x1b3919a2, 0xc0e52c1be0a4d491), KEND });
#endif /* !DEFINED_kwlist__ProcessHandle_ProcessInformationClass_ProcessInformationLength */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_NtWow64QueryInformationProcess64_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *raw_ProcessHandle;
		unsigned int ProcessInformationClass;
		size_t ProcessInformationLength;
	} args;
	HANDLE ProcessHandle;
	args.ProcessInformationLength = 64;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__ProcessHandle_ProcessInformationClass_ProcessInformationLength, "ou|" UNPuSIZ ":NtWow64QueryInformationProcess64", &args))
		goto err;
	if unlikely(DeeNTSystem_TryGetHandle(args.raw_ProcessHandle, (void **)&ProcessHandle))
		goto err;
	return libwin32_NtWow64QueryInformationProcess64_f_impl(ProcessHandle, args.ProcessInformationClass, args.ProcessInformationLength);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_NtWow64QueryInformationProcess64, &libwin32_NtWow64QueryInformationProcess64_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_NtWow64QueryInformationProcess64_f_impl(HANDLE ProcessHandle, unsigned int ProcessInformationClass, size_t ProcessInformationLength)
/*[[[end]]]*/
{
	NTSTATUS nsResult;
	ULONG ulReturnLength;
	DREF DeeBytesObject *result;
	LOAD_NTDLL_FUNCTION(err, NTSTATUS, NTAPI, NtWow64QueryInformationProcess64,
	                    (HANDLE ProcessHandle, /*PROCESSINFOCLASS*/ int ProcessInformationClass,
	                     PVOID ProcessInformation, ULONG ProcessInformationLength,
	                     PULONG ReturnLength))
	result = (DREF DeeBytesObject *)DeeBytes_NewBufferUninitialized(ProcessInformationLength);
	if unlikely(!result)
		goto err;
again:
	DBG_ALIGNMENT_DISABLE();
	ulReturnLength = 0;
	nsResult = (*NtWow64QueryInformationProcess64)(ProcessHandle,
	                                               (/*PROCESSINFOCLASS*/ int)ProcessInformationClass,
	                                               DeeBytes_DATA(result), (ULONG)DeeBytes_SIZE(result),
	                                               &ulReturnLength);
	DBG_ALIGNMENT_ENABLE();
	if (NT_ERROR(nsResult)) {
		DWORD dwError;
		LOAD_NTDLL_RTLNTSTATUSTODOSERROR(err_r);
		DBG_ALIGNMENT_DISABLE();
		dwError = (*RtlNtStatusToDosError)(nsResult);
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(dwError)) {
			if (DeeThread_CheckInterrupt())
				goto err_r;
			goto again;
		}
		if (DeeNTSystem_IsBufferTooSmall(dwError)) {
			if (ulReturnLength <= DeeBytes_SIZE(result))
				ulReturnLength = (ULONG)DeeBytes_SIZE(result) * 2;
			goto resize_buffer;
		}
		if (ulReturnLength == 0 || ulReturnLength >= 0x01000000) {
			Dee_Decref_likely(result);
			RETURN_ERROR(dwError,
			             "Failed to query wow64 information for process %p (ProcessInformationClass: %u)",
			             ProcessHandle, ProcessInformationClass);
		}
	}
	if (ulReturnLength > DeeBytes_SIZE(result)) {
		DREF DeeBytesObject *new_result;
resize_buffer:
		new_result = (DREF DeeBytesObject *)DeeBytes_ResizeBuffer((DeeObject *)result,
		                                                          ulReturnLength);
		if unlikely(!new_result)
			goto err_r;
		result = new_result;
		goto again;
	} else if (ulReturnLength < DeeBytes_SIZE(result)) {
		result = (DREF DeeBytesObject *)DeeBytes_TruncateBuffer((DeeObject *)result,
		                                                        ulReturnLength);
	}
	return (DREF DeeObject *)result;
err_r:
	Dee_Decref_likely(result);
err:
	return NULL;
}


/*[[[deemon import("rt.gen.dexutils").gw("ReadProcessMemory",
       "hProcess:nt:HANDLE"
      ",lpBaseAddress:c:uintptr_t"
      ",nSize:nt:SIZE_T"
     "->" MAYBE_NONE("?DBytes")); ]]]*/
#define LIBWIN32_READPROCESSMEMORY_DEF          DEX_MEMBER_F("ReadProcessMemory", &libwin32_ReadProcessMemory, DEXSYM_READONLY, "(hProcess:?X3?Dint?DFile?Ewin32:HANDLE,lpBaseAddress:?Dint,nSize:?Dint)->?DBytes"),
#define LIBWIN32_READPROCESSMEMORY_DEF_DOC(doc) DEX_MEMBER_F("ReadProcessMemory", &libwin32_ReadProcessMemory, DEXSYM_READONLY, "(hProcess:?X3?Dint?DFile?Ewin32:HANDLE,lpBaseAddress:?Dint,nSize:?Dint)->?DBytes\n" doc),
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_ReadProcessMemory_f_impl(HANDLE hProcess, uintptr_t lpBaseAddress, SIZE_T nSize);
#ifndef DEFINED_kwlist__hProcess_lpBaseAddress_nSize
#define DEFINED_kwlist__hProcess_lpBaseAddress_nSize
PRIVATE DEFINE_KWLIST(kwlist__hProcess_lpBaseAddress_nSize, { KEX("hProcess", 0x97c84287, 0x1d63e586eef1387e), KEX("lpBaseAddress", 0x161d05c5, 0x641a7dcfb6aac95d), KEX("nSize", 0xf9a4ff0b, 0x2eb43fdd0241156b), KEND });
#endif /* !DEFINED_kwlist__hProcess_lpBaseAddress_nSize */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_ReadProcessMemory_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *raw_hProcess;
		uintptr_t lpBaseAddress;
		SIZE_T nSize;
	} args;
	HANDLE hProcess;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__hProcess_lpBaseAddress_nSize, "o" UNPuPTR UNPuSIZ ":ReadProcessMemory", &args))
		goto err;
	if unlikely(DeeNTSystem_TryGetHandle(args.raw_hProcess, (void **)&hProcess))
		goto err;
	return libwin32_ReadProcessMemory_f_impl(hProcess, args.lpBaseAddress, args.nSize);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_ReadProcessMemory, &libwin32_ReadProcessMemory_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_ReadProcessMemory_f_impl(HANDLE hProcess, uintptr_t lpBaseAddress, SIZE_T nSize)
/*[[[end]]]*/
{
	SIZE_T szNumberOfBytesRead;
	DREF DeeBytesObject *result;
	BOOL bOk;
	result = (DREF DeeBytesObject *)DeeBytes_NewBufferUninitialized(nSize);
	if unlikely(!result)
		goto err;
again:
	DBG_ALIGNMENT_DISABLE();
	bOk = ReadProcessMemory(hProcess, (LPCVOID)lpBaseAddress,
	                        DeeBytes_DATA(result), nSize,
	                        &szNumberOfBytesRead);
	if (!bOk) {
		DWORD dwError;
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(dwError)) {
			if (DeeThread_CheckInterrupt())
				goto err_r;
			goto again;
		}
		Dee_Decref_likely(result);
		RETURN_ERROR_OR_FALSE(dwError,
		                      "Failed to read process %p memory at (nSize: %" PRFuSIZ ")",
		                      hProcess, (void *)lpBaseAddress, nSize);
	}
	DBG_ALIGNMENT_ENABLE();
	if unlikely(szNumberOfBytesRead < nSize) {
		result = (DREF DeeBytesObject *)DeeBytes_ResizeBuffer((DREF DeeObject *)result,
		                                                      szNumberOfBytesRead);
	}
	return (DREF DeeObject *)result;
err_r:
	Dee_Decref_likely(result);
err:
	return NULL;
}


/*[[[deemon import("rt.gen.dexutils").gw("NtWow64ReadVirtualMemory64",
       "hProcess:nt:HANDLE"
      ",lpBaseAddress:c:uint64_t"
      ",nSize:nt:ULONG64"
     "->" MAYBE_NONE("?DBytes")); ]]]*/
#define LIBWIN32_NTWOW64READVIRTUALMEMORY64_DEF          DEX_MEMBER_F("NtWow64ReadVirtualMemory64", &libwin32_NtWow64ReadVirtualMemory64, DEXSYM_READONLY, "(hProcess:?X3?Dint?DFile?Ewin32:HANDLE,lpBaseAddress:?Dint,nSize:?Dint)->?DBytes"),
#define LIBWIN32_NTWOW64READVIRTUALMEMORY64_DEF_DOC(doc) DEX_MEMBER_F("NtWow64ReadVirtualMemory64", &libwin32_NtWow64ReadVirtualMemory64, DEXSYM_READONLY, "(hProcess:?X3?Dint?DFile?Ewin32:HANDLE,lpBaseAddress:?Dint,nSize:?Dint)->?DBytes\n" doc),
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_NtWow64ReadVirtualMemory64_f_impl(HANDLE hProcess, uint64_t lpBaseAddress, ULONG64 nSize);
#ifndef DEFINED_kwlist__hProcess_lpBaseAddress_nSize
#define DEFINED_kwlist__hProcess_lpBaseAddress_nSize
PRIVATE DEFINE_KWLIST(kwlist__hProcess_lpBaseAddress_nSize, { KEX("hProcess", 0x97c84287, 0x1d63e586eef1387e), KEX("lpBaseAddress", 0x161d05c5, 0x641a7dcfb6aac95d), KEX("nSize", 0xf9a4ff0b, 0x2eb43fdd0241156b), KEND });
#endif /* !DEFINED_kwlist__hProcess_lpBaseAddress_nSize */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_NtWow64ReadVirtualMemory64_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *raw_hProcess;
		uint64_t lpBaseAddress;
		ULONG64 nSize;
	} args;
	HANDLE hProcess;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__hProcess_lpBaseAddress_nSize, "o" UNPu64 UNPu64 ":NtWow64ReadVirtualMemory64", &args))
		goto err;
	if unlikely(DeeNTSystem_TryGetHandle(args.raw_hProcess, (void **)&hProcess))
		goto err;
	return libwin32_NtWow64ReadVirtualMemory64_f_impl(hProcess, args.lpBaseAddress, args.nSize);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_NtWow64ReadVirtualMemory64, &libwin32_NtWow64ReadVirtualMemory64_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_NtWow64ReadVirtualMemory64_f_impl(HANDLE hProcess, uint64_t lpBaseAddress, ULONG64 nSize)
/*[[[end]]]*/
{
#if __SIZEOF_POINTER__ >= 8
	return libwin32_ReadProcessMemory_f_impl(hProcess, (uintptr_t)lpBaseAddress, (SIZE_T)nSize);
#else /* __SIZEOF_POINTER__ >= 8 */
	ULONG64 szNumberOfBytesRead;
	DREF DeeBytesObject *result;
	NTSTATUS nsResult;
	LOAD_NTDLL_FUNCTION(err, NTSTATUS, NTAPI, NtWow64ReadVirtualMemory64,
	                    (HANDLE ProcessHandle, PVOID64 BaseAddress,
	                     PVOID Buffer, ULONG64 Size, PULONG64 NumberOfBytesRead))
	result = (DREF DeeBytesObject *)DeeBytes_NewBufferUninitialized((size_t)nSize);
	if unlikely(!result)
		goto err;
again:
	DBG_ALIGNMENT_DISABLE();
	nsResult = (*NtWow64ReadVirtualMemory64)(hProcess, (PVOID64)lpBaseAddress,
	                                         DeeBytes_DATA(result), nSize,
	                                         &szNumberOfBytesRead);
	if (NT_ERROR(nsResult)) {
		DWORD dwError;
		LOAD_NTDLL_RTLNTSTATUSTODOSERROR(err_r);
		DBG_ALIGNMENT_DISABLE();
		dwError = (*RtlNtStatusToDosError)(nsResult);
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(dwError)) {
			if (DeeThread_CheckInterrupt())
				goto err_r;
			goto again;
		}
		Dee_Decref_likely(result);
		RETURN_ERROR_OR_FALSE(dwError,
		                      "Failed to read wow64 process %p memory at (nSize: %" PRFuSIZ ")",
		                      hProcess, (void *)lpBaseAddress, nSize);
	}
	DBG_ALIGNMENT_ENABLE();
	if unlikely(szNumberOfBytesRead < nSize) {
		result = (DREF DeeBytesObject *)DeeBytes_ResizeBuffer((DREF DeeObject *)result,
		                                                      (size_t)szNumberOfBytesRead);
	}
	return (DREF DeeObject *)result;
err_r:
	Dee_Decref_likely(result);
err:
	return NULL;
#endif /* __SIZEOF_POINTER__ < 8 */
}


/*[[[deemon import("rt.gen.dexutils").gw("WriteProcessMemory",
       "hProcess:nt:HANDLE"
      ",lpBaseAddress:c:uintptr_t"
      ",lpBuffer:obj:buffer"
     "->" MAYBE_NONE("?Dint")); ]]]*/
#define LIBWIN32_WRITEPROCESSMEMORY_DEF          DEX_MEMBER_F("WriteProcessMemory", &libwin32_WriteProcessMemory, DEXSYM_READONLY, "(hProcess:?X3?Dint?DFile?Ewin32:HANDLE,lpBaseAddress:?Dint,lpBuffer:?DBytes)->?Dint"),
#define LIBWIN32_WRITEPROCESSMEMORY_DEF_DOC(doc) DEX_MEMBER_F("WriteProcessMemory", &libwin32_WriteProcessMemory, DEXSYM_READONLY, "(hProcess:?X3?Dint?DFile?Ewin32:HANDLE,lpBaseAddress:?Dint,lpBuffer:?DBytes)->?Dint\n" doc),
FORCELOCAL WUNUSED NONNULL((3)) DREF DeeObject *DCALL libwin32_WriteProcessMemory_f_impl(HANDLE hProcess, uintptr_t lpBaseAddress, DeeObject *lpBuffer);
#ifndef DEFINED_kwlist__hProcess_lpBaseAddress_lpBuffer
#define DEFINED_kwlist__hProcess_lpBaseAddress_lpBuffer
PRIVATE DEFINE_KWLIST(kwlist__hProcess_lpBaseAddress_lpBuffer, { KEX("hProcess", 0x97c84287, 0x1d63e586eef1387e), KEX("lpBaseAddress", 0x161d05c5, 0x641a7dcfb6aac95d), KEX("lpBuffer", 0x6c1894a0, 0xda993aa6a0fe0491), KEND });
#endif /* !DEFINED_kwlist__hProcess_lpBaseAddress_lpBuffer */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_WriteProcessMemory_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *raw_hProcess;
		uintptr_t lpBaseAddress;
		DeeObject *lpBuffer;
	} args;
	HANDLE hProcess;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__hProcess_lpBaseAddress_lpBuffer, "o" UNPuPTR "o:WriteProcessMemory", &args))
		goto err;
	if unlikely(DeeNTSystem_TryGetHandle(args.raw_hProcess, (void **)&hProcess))
		goto err;
	return libwin32_WriteProcessMemory_f_impl(hProcess, args.lpBaseAddress, args.lpBuffer);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_WriteProcessMemory, &libwin32_WriteProcessMemory_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((3)) DREF DeeObject *DCALL libwin32_WriteProcessMemory_f_impl(HANDLE hProcess, uintptr_t lpBaseAddress, DeeObject *lpBuffer)
/*[[[end]]]*/
{
	SIZE_T szNumberOfBytesWritten;
	BOOL bOk;
	DeeBuffer buf;
	if (DeeObject_GetBuf(lpBuffer, &buf, Dee_BUFFER_FREADONLY))
		goto err;
again:
	DBG_ALIGNMENT_DISABLE();
	bOk = WriteProcessMemory(hProcess, (LPVOID)lpBaseAddress,
	                         buf.bb_base, buf.bb_size,
	                         &szNumberOfBytesWritten);
	if (!bOk) {
		DWORD dwError;
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(dwError)) {
			if (DeeThread_CheckInterrupt())
				goto err_buf;
			goto again;
		}
		DeeObject_PutBuf(lpBuffer, &buf, Dee_BUFFER_FREADONLY);
		RETURN_ERROR_OR_FALSE(dwError,
		                      "Failed to write process %p memory at (nSize: %" PRFuSIZ ")",
		                      hProcess, (void *)lpBaseAddress, buf.bb_size);
	}
	DBG_ALIGNMENT_ENABLE();
	DeeObject_PutBuf(lpBuffer, &buf, Dee_BUFFER_FREADONLY);
	return DeeInt_NewSize(szNumberOfBytesWritten);
err_buf:
	DeeObject_PutBuf(lpBuffer, &buf, Dee_BUFFER_FREADONLY);
err:
	return NULL;
}


/*[[[deemon import("rt.gen.dexutils").gw("NtWow64WriteVirtualMemory64",
       "hProcess:nt:HANDLE"
      ",lpBaseAddress:c:uint64_t"
      ",lpBuffer:obj:buffer"
     "->" MAYBE_NONE("?Dint")); ]]]*/
#define LIBWIN32_NTWOW64WRITEVIRTUALMEMORY64_DEF          DEX_MEMBER_F("NtWow64WriteVirtualMemory64", &libwin32_NtWow64WriteVirtualMemory64, DEXSYM_READONLY, "(hProcess:?X3?Dint?DFile?Ewin32:HANDLE,lpBaseAddress:?Dint,lpBuffer:?DBytes)->?Dint"),
#define LIBWIN32_NTWOW64WRITEVIRTUALMEMORY64_DEF_DOC(doc) DEX_MEMBER_F("NtWow64WriteVirtualMemory64", &libwin32_NtWow64WriteVirtualMemory64, DEXSYM_READONLY, "(hProcess:?X3?Dint?DFile?Ewin32:HANDLE,lpBaseAddress:?Dint,lpBuffer:?DBytes)->?Dint\n" doc),
FORCELOCAL WUNUSED NONNULL((3)) DREF DeeObject *DCALL libwin32_NtWow64WriteVirtualMemory64_f_impl(HANDLE hProcess, uint64_t lpBaseAddress, DeeObject *lpBuffer);
#ifndef DEFINED_kwlist__hProcess_lpBaseAddress_lpBuffer
#define DEFINED_kwlist__hProcess_lpBaseAddress_lpBuffer
PRIVATE DEFINE_KWLIST(kwlist__hProcess_lpBaseAddress_lpBuffer, { KEX("hProcess", 0x97c84287, 0x1d63e586eef1387e), KEX("lpBaseAddress", 0x161d05c5, 0x641a7dcfb6aac95d), KEX("lpBuffer", 0x6c1894a0, 0xda993aa6a0fe0491), KEND });
#endif /* !DEFINED_kwlist__hProcess_lpBaseAddress_lpBuffer */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_NtWow64WriteVirtualMemory64_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *raw_hProcess;
		uint64_t lpBaseAddress;
		DeeObject *lpBuffer;
	} args;
	HANDLE hProcess;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__hProcess_lpBaseAddress_lpBuffer, "o" UNPu64 "o:NtWow64WriteVirtualMemory64", &args))
		goto err;
	if unlikely(DeeNTSystem_TryGetHandle(args.raw_hProcess, (void **)&hProcess))
		goto err;
	return libwin32_NtWow64WriteVirtualMemory64_f_impl(hProcess, args.lpBaseAddress, args.lpBuffer);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_NtWow64WriteVirtualMemory64, &libwin32_NtWow64WriteVirtualMemory64_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((3)) DREF DeeObject *DCALL libwin32_NtWow64WriteVirtualMemory64_f_impl(HANDLE hProcess, uint64_t lpBaseAddress, DeeObject *lpBuffer)
/*[[[end]]]*/
{
#if __SIZEOF_POINTER__ >= 8
	return libwin32_WriteProcessMemory_f_impl(hProcess, (uintptr_t)lpBaseAddress, lpBuffer);
#else /* __SIZEOF_POINTER__ >= 8 */
	ULONG64 szNumberOfBytesWritten;
	DeeBuffer buf;
	NTSTATUS nsResult;
	LOAD_NTDLL_FUNCTION(err, NTSTATUS, NTAPI, NtWow64WriteVirtualMemory64,
	                    (HANDLE ProcessHandle, PVOID64 BaseAddress,
	                     PVOID Buffer, ULONG64 Size, PULONG64 NumberOfBytesWritten))
	if (DeeObject_GetBuf(lpBuffer, &buf, Dee_BUFFER_FREADONLY))
		goto err;
again:
	DBG_ALIGNMENT_DISABLE();
	nsResult = (*NtWow64WriteVirtualMemory64)(hProcess, (PVOID64)lpBaseAddress,
	                                          buf.bb_base, buf.bb_size,
	                                          &szNumberOfBytesWritten);
	if (NT_ERROR(nsResult)) {
		DWORD dwError;
		LOAD_NTDLL_RTLNTSTATUSTODOSERROR(err_buf);
		DBG_ALIGNMENT_DISABLE();
		dwError = (*RtlNtStatusToDosError)(nsResult);
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(dwError)) {
			if (DeeThread_CheckInterrupt())
				goto err_buf;
			goto again;
		}
		DeeObject_PutBuf(lpBuffer, &buf, Dee_BUFFER_FREADONLY);
		RETURN_ERROR_OR_FALSE(dwError,
		                      "Failed to write wow64 process %.16" PRFX64 " memory at (nSize: %" PRFuSIZ ")",
		                      hProcess, lpBaseAddress, buf.bb_size);
	}
	DBG_ALIGNMENT_ENABLE();
	DeeObject_PutBuf(lpBuffer, &buf, Dee_BUFFER_FREADONLY);
	return DeeInt_NewUInt64(szNumberOfBytesWritten);
err_buf:
	DeeObject_PutBuf(lpBuffer, &buf, Dee_BUFFER_FREADONLY);
err:
	return NULL;
#endif /* __SIZEOF_POINTER__ < 8 */
}


/*[[[deemon import("rt.gen.dexutils").gw("WaitForSingleObject",
      "hHandle:nt:HANDLE"
     ",dwMilliseconds:nt:DWORD=INFINITE=!GINFINITE"
     "->" MAYBE_NONE("?Dint")); ]]]*/
#define LIBWIN32_WAITFORSINGLEOBJECT_DEF          DEX_MEMBER_F("WaitForSingleObject", &libwin32_WaitForSingleObject, DEXSYM_READONLY, "(hHandle:?X3?Dint?DFile?Ewin32:HANDLE,dwMilliseconds:?Dint=!GINFINITE)->?Dint"),
#define LIBWIN32_WAITFORSINGLEOBJECT_DEF_DOC(doc) DEX_MEMBER_F("WaitForSingleObject", &libwin32_WaitForSingleObject, DEXSYM_READONLY, "(hHandle:?X3?Dint?DFile?Ewin32:HANDLE,dwMilliseconds:?Dint=!GINFINITE)->?Dint\n" doc),
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_WaitForSingleObject_f_impl(HANDLE hHandle, DWORD dwMilliseconds);
#ifndef DEFINED_kwlist__hHandle_dwMilliseconds
#define DEFINED_kwlist__hHandle_dwMilliseconds
PRIVATE DEFINE_KWLIST(kwlist__hHandle_dwMilliseconds, { KEX("hHandle", 0x69103c85, 0xc1f4ea73037322b8), KEX("dwMilliseconds", 0x320be8cc, 0xce92e1c3794c9fc6), KEND });
#endif /* !DEFINED_kwlist__hHandle_dwMilliseconds */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_WaitForSingleObject_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *raw_hHandle;
		DWORD dwMilliseconds;
	} args;
	HANDLE hHandle;
	args.dwMilliseconds = INFINITE;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__hHandle_dwMilliseconds, "o|" UNPu32 ":WaitForSingleObject", &args))
		goto err;
	if unlikely(DeeNTSystem_TryGetHandle(args.raw_hHandle, (void **)&hHandle))
		goto err;
	return libwin32_WaitForSingleObject_f_impl(hHandle, args.dwMilliseconds);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_WaitForSingleObject, &libwin32_WaitForSingleObject_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_WaitForSingleObject_f_impl(HANDLE hHandle, DWORD dwMilliseconds)
/*[[[end]]]*/
{
	DWORD dwResult;
again:
	DBG_ALIGNMENT_DISABLE();
	dwResult = WaitForSingleObjectEx(hHandle, dwMilliseconds, true);
	if (dwResult == WAIT_IO_COMPLETION)
		goto check_interrupt;
	if (dwResult == WAIT_FAILED) {
		DWORD dwError;
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(dwError)) {
check_interrupt:
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		RETURN_ERROR(dwError,
		             "Failed to wait for handle %p (dwMilliseconds: %#" PRFx32 ")",
		             hHandle, dwMilliseconds);
	}
	DBG_ALIGNMENT_ENABLE();
	return DeeInt_NewUInt32(dwResult);
err:
	return NULL;
}


/*[[[deemon import("rt.gen.dexutils").gw("WaitForMultipleObjects",
      "lpHandles:?S?X3?Dint?DFile?Ewin32:HANDLE"
     ",bWaitAll:c:bool"
     ",dwMilliseconds:nt:DWORD=INFINITE=!GINFINITE"
     "->" MAYBE_NONE("?Dint")); ]]]*/
#define LIBWIN32_WAITFORMULTIPLEOBJECTS_DEF          DEX_MEMBER_F("WaitForMultipleObjects", &libwin32_WaitForMultipleObjects, DEXSYM_READONLY, "(lpHandles:?S?X3?Dint?DFile?Ewin32:HANDLE,bWaitAll:?Dbool,dwMilliseconds:?Dint=!GINFINITE)->?Dint"),
#define LIBWIN32_WAITFORMULTIPLEOBJECTS_DEF_DOC(doc) DEX_MEMBER_F("WaitForMultipleObjects", &libwin32_WaitForMultipleObjects, DEXSYM_READONLY, "(lpHandles:?S?X3?Dint?DFile?Ewin32:HANDLE,bWaitAll:?Dbool,dwMilliseconds:?Dint=!GINFINITE)->?Dint\n" doc),
FORCELOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL libwin32_WaitForMultipleObjects_f_impl(DeeObject *lpHandles, bool bWaitAll, DWORD dwMilliseconds);
#ifndef DEFINED_kwlist__lpHandles_bWaitAll_dwMilliseconds
#define DEFINED_kwlist__lpHandles_bWaitAll_dwMilliseconds
PRIVATE DEFINE_KWLIST(kwlist__lpHandles_bWaitAll_dwMilliseconds, { KEX("lpHandles", 0x1194c901, 0x670370c3b5f1e0d8), KEX("bWaitAll", 0x8e79ac01, 0x67dfc22f01d7ea59), KEX("dwMilliseconds", 0x320be8cc, 0xce92e1c3794c9fc6), KEND });
#endif /* !DEFINED_kwlist__lpHandles_bWaitAll_dwMilliseconds */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_WaitForMultipleObjects_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *lpHandles;
		bool bWaitAll;
		DWORD dwMilliseconds;
	} args;
	args.dwMilliseconds = INFINITE;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__lpHandles_bWaitAll_dwMilliseconds, "ob|" UNPu32 ":WaitForMultipleObjects", &args))
		goto err;
	return libwin32_WaitForMultipleObjects_f_impl(args.lpHandles, args.bWaitAll, args.dwMilliseconds);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_WaitForMultipleObjects, &libwin32_WaitForMultipleObjects_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL libwin32_WaitForMultipleObjects_f_impl(DeeObject *lpHandles, bool bWaitAll, DWORD dwMilliseconds)
/*[[[end]]]*/
{
	DWORD dwResult;
	union {
		void **v;
		DREF DeeObject **o;
	} pHandles;
	size_t i, nCount;
	pHandles.o = DeeSeq_AsHeapVector(lpHandles, &nCount);
	if unlikely(!pHandles.o)
		goto err;
	for (i = 0; i < nCount; ++i) {
		void *hHandle;
		if unlikely(DeeNTSystem_TryGetHandle(pHandles.o[i], &hHandle))
			goto err_handles_i;
		Dee_Decref(pHandles.o[i]);
		pHandles.v[i] = hHandle;
	}
again:
	DBG_ALIGNMENT_DISABLE();
	dwResult = WaitForMultipleObjectsEx((DWORD)nCount,
	                                    (HANDLE const *)pHandles.v,
	                                    bWaitAll,
	                                    dwMilliseconds,
	                                    true);
	if (dwResult == WAIT_IO_COMPLETION)
		goto check_interrupt;
	if (dwResult == WAIT_FAILED) {
		DWORD dwError;
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(dwError)) {
check_interrupt:
			if (DeeThread_CheckInterrupt())
				goto err_handles;
			goto again;
		}
		Dee_Free(pHandles.v);
		RETURN_ERROR(dwError,
		             "Failed to wait for %" PRFuSIZ " handles (bWaitAll: %u, dwMilliseconds: %#" PRFx32 ")",
		             nCount, (unsigned int)bWaitAll, dwMilliseconds);
	}
	DBG_ALIGNMENT_ENABLE();
	Dee_Free(pHandles.v);
	return DeeInt_NewUInt32(dwResult);
err_handles_i:
	Dee_Decrefv(pHandles.o + i, nCount - i);
err_handles:
	Dee_Free(pHandles.v);
err:
	return NULL;
}


/*[[[deemon import("rt.gen.dexutils").gw("SignalObjectAndWait",
      "hObjectToSignal:nt:HANDLE"
     ",hObjectToWaitOn:nt:HANDLE"
     ",dwMilliseconds:nt:DWORD=INFINITE=!GINFINITE"
     "->" MAYBE_NONE("?Dint")); ]]]*/
#define LIBWIN32_SIGNALOBJECTANDWAIT_DEF          DEX_MEMBER_F("SignalObjectAndWait", &libwin32_SignalObjectAndWait, DEXSYM_READONLY, "(hObjectToSignal:?X3?Dint?DFile?Ewin32:HANDLE,hObjectToWaitOn:?X3?Dint?DFile?Ewin32:HANDLE,dwMilliseconds:?Dint=!GINFINITE)->?Dint"),
#define LIBWIN32_SIGNALOBJECTANDWAIT_DEF_DOC(doc) DEX_MEMBER_F("SignalObjectAndWait", &libwin32_SignalObjectAndWait, DEXSYM_READONLY, "(hObjectToSignal:?X3?Dint?DFile?Ewin32:HANDLE,hObjectToWaitOn:?X3?Dint?DFile?Ewin32:HANDLE,dwMilliseconds:?Dint=!GINFINITE)->?Dint\n" doc),
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_SignalObjectAndWait_f_impl(HANDLE hObjectToSignal, HANDLE hObjectToWaitOn, DWORD dwMilliseconds);
#ifndef DEFINED_kwlist__hObjectToSignal_hObjectToWaitOn_dwMilliseconds
#define DEFINED_kwlist__hObjectToSignal_hObjectToWaitOn_dwMilliseconds
PRIVATE DEFINE_KWLIST(kwlist__hObjectToSignal_hObjectToWaitOn_dwMilliseconds, { KEX("hObjectToSignal", 0xf3af01ba, 0xa8aca225120e5b2f), KEX("hObjectToWaitOn", 0xc7552b44, 0xe34c945a9fa32e7e), KEX("dwMilliseconds", 0x320be8cc, 0xce92e1c3794c9fc6), KEND });
#endif /* !DEFINED_kwlist__hObjectToSignal_hObjectToWaitOn_dwMilliseconds */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_SignalObjectAndWait_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *raw_hObjectToSignal;
		DeeObject *raw_hObjectToWaitOn;
		DWORD dwMilliseconds;
	} args;
	HANDLE hObjectToSignal;
	HANDLE hObjectToWaitOn;
	args.dwMilliseconds = INFINITE;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__hObjectToSignal_hObjectToWaitOn_dwMilliseconds, "oo|" UNPu32 ":SignalObjectAndWait", &args))
		goto err;
	if unlikely(DeeNTSystem_TryGetHandle(args.raw_hObjectToSignal, (void **)&hObjectToSignal))
		goto err;
	if unlikely(DeeNTSystem_TryGetHandle(args.raw_hObjectToWaitOn, (void **)&hObjectToWaitOn))
		goto err;
	return libwin32_SignalObjectAndWait_f_impl(hObjectToSignal, hObjectToWaitOn, args.dwMilliseconds);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_SignalObjectAndWait, &libwin32_SignalObjectAndWait_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_SignalObjectAndWait_f_impl(HANDLE hObjectToSignal, HANDLE hObjectToWaitOn, DWORD dwMilliseconds)
/*[[[end]]]*/
{
	DWORD dwResult;
again:
	DBG_ALIGNMENT_DISABLE();
	dwResult = SignalObjectAndWait(hObjectToSignal,
	                               hObjectToWaitOn,
	                               dwMilliseconds,
	                               true);
	if (dwResult == WAIT_IO_COMPLETION)
		goto check_interrupt;
	if (dwResult == WAIT_FAILED) {
		DWORD dwError;
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(dwError)) {
check_interrupt:
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		RETURN_ERROR(dwError,
		             "Failed to signal object %p and wait for %p (dwMilliseconds: %#" PRFx32 ")",
		             hObjectToSignal, hObjectToWaitOn, dwMilliseconds);
	}
	DBG_ALIGNMENT_ENABLE();
	return DeeInt_NewUInt32(dwResult);
err:
	return NULL;
}


/*[[[deemon import("rt.gen.dexutils").gw("Sleep",
     "dwMilliseconds:nt:DWORD"); ]]]*/
#define LIBWIN32_SLEEP_DEF          DEX_MEMBER_F("Sleep", &libwin32_Sleep, DEXSYM_READONLY, "(dwMilliseconds:?Dint)"),
#define LIBWIN32_SLEEP_DEF_DOC(doc) DEX_MEMBER_F("Sleep", &libwin32_Sleep, DEXSYM_READONLY, "(dwMilliseconds:?Dint)\n" doc),
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_Sleep_f_impl(DWORD dwMilliseconds);
#ifndef DEFINED_kwlist__dwMilliseconds
#define DEFINED_kwlist__dwMilliseconds
PRIVATE DEFINE_KWLIST(kwlist__dwMilliseconds, { KEX("dwMilliseconds", 0x320be8cc, 0xce92e1c3794c9fc6), KEND });
#endif /* !DEFINED_kwlist__dwMilliseconds */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_Sleep_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DWORD dwMilliseconds;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__dwMilliseconds, UNPu32 ":Sleep", &args))
		goto err;
	return libwin32_Sleep_f_impl(args.dwMilliseconds);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_Sleep, &libwin32_Sleep_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_Sleep_f_impl(DWORD dwMilliseconds)
/*[[[end]]]*/
{
	DWORD dwResult;
again:
	DBG_ALIGNMENT_DISABLE();
	dwResult = SleepEx(dwMilliseconds, true);
	if (dwResult == WAIT_IO_COMPLETION) {
		if (DeeThread_CheckInterrupt())
			goto err;
		goto again;
	}
	DBG_ALIGNMENT_ENABLE();
	return_none;
err:
	return NULL;
}


/*[[[deemon import("rt.gen.dexutils").gw("CreateEvent",
     "lpEventAttributes:?GSECURITY_ATTRIBUTES=NULL"
     ",bManualReset:c:bool=false"
     ",bInitialState:c:bool=false"
     ",lpName:nt:LPCWSTR=NULL"
     "->" MAYBE_NONE("?GHANDLE")); ]]]*/
#define LIBWIN32_CREATEEVENT_DEF          DEX_MEMBER_F("CreateEvent", &libwin32_CreateEvent, DEXSYM_READONLY, "(lpEventAttributes?:?GSECURITY_ATTRIBUTES,bManualReset=!f,bInitialState=!f,lpName?:?Dstring)->?GHANDLE"),
#define LIBWIN32_CREATEEVENT_DEF_DOC(doc) DEX_MEMBER_F("CreateEvent", &libwin32_CreateEvent, DEXSYM_READONLY, "(lpEventAttributes?:?GSECURITY_ATTRIBUTES,bManualReset=!f,bInitialState=!f,lpName?:?Dstring)->?GHANDLE\n" doc),
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_CreateEvent_f_impl(DeeObject *lpEventAttributes, bool bManualReset, bool bInitialState, LPCWSTR lpName);
#ifndef DEFINED_kwlist__lpEventAttributes_bManualReset_bInitialState_lpName
#define DEFINED_kwlist__lpEventAttributes_bManualReset_bInitialState_lpName
PRIVATE DEFINE_KWLIST(kwlist__lpEventAttributes_bManualReset_bInitialState_lpName, { KEX("lpEventAttributes", 0x48c844a7, 0x21e9950d8a034a75), KEX("bManualReset", 0x8284a854, 0xe02d1cfc7c0f513f), KEX("bInitialState", 0xe5b1f8bb, 0x8e8bb7ead921b0fe), KEX("lpName", 0xe3f0b9dd, 0x6d44dd71565f84d6), KEND });
#endif /* !DEFINED_kwlist__lpEventAttributes_bManualReset_bInitialState_lpName */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_CreateEvent_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *lpEventAttributes;
		bool bManualReset;
		bool bInitialState;
		LPCWSTR lpName;
	} args;
	args.lpEventAttributes = NULL;
	args.bManualReset = false;
	args.bInitialState = false;
	args.lpName = NULL;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__lpEventAttributes_bManualReset_bInitialState_lpName, "|obbU16s:CreateEvent", &args))
		goto err;
	return libwin32_CreateEvent_f_impl(args.lpEventAttributes, args.bManualReset, args.bInitialState, args.lpName);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_CreateEvent, &libwin32_CreateEvent_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_CreateEvent_f_impl(DeeObject *lpEventAttributes, bool bManualReset, bool bInitialState, LPCWSTR lpName)
/*[[[end]]]*/
{
	HANDLE hResult;
	DREF DeeObject *result;
again:
	DBG_ALIGNMENT_DISABLE();
	(void)lpEventAttributes; /* TODO */
	hResult = CreateEventW(NULL, bManualReset, bInitialState, lpName);
	if (hResult == NULL) {
		DWORD dwError;
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(dwError)) {
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		RETURN_ERROR(dwError,
		             "Failed to create event (bManualReset: %u, bInitialState: %u, lpName: %lq)",
		             (unsigned int)bManualReset, (unsigned int)bInitialState, lpName);
	}
	DBG_ALIGNMENT_ENABLE();
	result = libwin32_CreateHandle(hResult);
	if likely(result)
		return result;
	CloseHandle(hResult);
err:
	return NULL;
}


/*[[[deemon import("rt.gen.dexutils").gw("OpenEvent",
     "dwDesiredAccess:nt:DWORD"
     ",bInheritHandle:c:bool"
     ",lpName:nt:LPCWSTR"
     "->" MAYBE_NONE("?GHANDLE")); ]]]*/
#define LIBWIN32_OPENEVENT_DEF          DEX_MEMBER_F("OpenEvent", &libwin32_OpenEvent, DEXSYM_READONLY, "(dwDesiredAccess:?Dint,bInheritHandle:?Dbool,lpName:?Dstring)->?GHANDLE"),
#define LIBWIN32_OPENEVENT_DEF_DOC(doc) DEX_MEMBER_F("OpenEvent", &libwin32_OpenEvent, DEXSYM_READONLY, "(dwDesiredAccess:?Dint,bInheritHandle:?Dbool,lpName:?Dstring)->?GHANDLE\n" doc),
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_OpenEvent_f_impl(DWORD dwDesiredAccess, bool bInheritHandle, LPCWSTR lpName);
#ifndef DEFINED_kwlist__dwDesiredAccess_bInheritHandle_lpName
#define DEFINED_kwlist__dwDesiredAccess_bInheritHandle_lpName
PRIVATE DEFINE_KWLIST(kwlist__dwDesiredAccess_bInheritHandle_lpName, { KEX("dwDesiredAccess", 0xa4c2866d, 0x5a4b46d72691f2f), KEX("bInheritHandle", 0x632313fa, 0x9e7213d20a3bbbb4), KEX("lpName", 0xe3f0b9dd, 0x6d44dd71565f84d6), KEND });
#endif /* !DEFINED_kwlist__dwDesiredAccess_bInheritHandle_lpName */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_OpenEvent_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DWORD dwDesiredAccess;
		bool bInheritHandle;
		LPCWSTR lpName;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__dwDesiredAccess_bInheritHandle_lpName, UNPu32 "bU16s:OpenEvent", &args))
		goto err;
	return libwin32_OpenEvent_f_impl(args.dwDesiredAccess, args.bInheritHandle, args.lpName);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_OpenEvent, &libwin32_OpenEvent_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_OpenEvent_f_impl(DWORD dwDesiredAccess, bool bInheritHandle, LPCWSTR lpName)
/*[[[end]]]*/
{
	HANDLE hResult;
	DREF DeeObject *result;
again:
	DBG_ALIGNMENT_DISABLE();
	hResult = OpenMutexW(dwDesiredAccess, bInheritHandle, lpName);
	if (hResult == NULL) {
		DWORD dwError;
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(dwError)) {
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		RETURN_ERROR(dwError,
		             "Failed to open event (dwDesiredAccess: %#" PRFx32 ", bInheritHandle: %u, lpName: %lq)",
		             dwDesiredAccess, (unsigned int)bInheritHandle, lpName);
	}
	DBG_ALIGNMENT_ENABLE();
	result = libwin32_CreateHandle(hResult);
	if likely(result)
		return result;
	CloseHandle(hResult);
err:
	return NULL;
}


/*[[[deemon import("rt.gen.dexutils").gw("ResetEvent",
      "hEvent:nt:HANDLE"
     "->" ERROR_OR_BOOL
     ); ]]]*/
#define LIBWIN32_RESETEVENT_DEF          DEX_MEMBER_F("ResetEvent", &libwin32_ResetEvent, DEXSYM_READONLY, "(hEvent:?X3?Dint?DFile?Ewin32:HANDLE)"),
#define LIBWIN32_RESETEVENT_DEF_DOC(doc) DEX_MEMBER_F("ResetEvent", &libwin32_ResetEvent, DEXSYM_READONLY, "(hEvent:?X3?Dint?DFile?Ewin32:HANDLE)\n" doc),
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_ResetEvent_f_impl(HANDLE hEvent);
#ifndef DEFINED_kwlist__hEvent
#define DEFINED_kwlist__hEvent
PRIVATE DEFINE_KWLIST(kwlist__hEvent, { KEX("hEvent", 0x5b8c4808, 0xcbf24b1abbd2dd7d), KEND });
#endif /* !DEFINED_kwlist__hEvent */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_ResetEvent_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *raw_hEvent;
	} args;
	HANDLE hEvent;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__hEvent, "o:ResetEvent", &args))
		goto err;
	if unlikely(DeeNTSystem_TryGetHandle(args.raw_hEvent, (void **)&hEvent))
		goto err;
	return libwin32_ResetEvent_f_impl(hEvent);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_ResetEvent, &libwin32_ResetEvent_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_ResetEvent_f_impl(HANDLE hEvent)
/*[[[end]]]*/
{
	BOOL bOk;
again:
	DBG_ALIGNMENT_DISABLE();
	bOk = ResetEvent(hEvent);
	if (!bOk) {
		DWORD dwError;
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(dwError)) {
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		RETURN_ERROR_OR_FALSE(dwError,
		                      "Failed to reset event %p",
		                      hEvent);
	}
	DBG_ALIGNMENT_ENABLE();
	RETURN_SUCCESS_OR_TRUE;
err:
	return NULL;
}


/*[[[deemon import("rt.gen.dexutils").gw("SetEvent",
      "hEvent:nt:HANDLE"
     "->" ERROR_OR_BOOL
     ); ]]]*/
#define LIBWIN32_SETEVENT_DEF          DEX_MEMBER_F("SetEvent", &libwin32_SetEvent, DEXSYM_READONLY, "(hEvent:?X3?Dint?DFile?Ewin32:HANDLE)"),
#define LIBWIN32_SETEVENT_DEF_DOC(doc) DEX_MEMBER_F("SetEvent", &libwin32_SetEvent, DEXSYM_READONLY, "(hEvent:?X3?Dint?DFile?Ewin32:HANDLE)\n" doc),
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_SetEvent_f_impl(HANDLE hEvent);
#ifndef DEFINED_kwlist__hEvent
#define DEFINED_kwlist__hEvent
PRIVATE DEFINE_KWLIST(kwlist__hEvent, { KEX("hEvent", 0x5b8c4808, 0xcbf24b1abbd2dd7d), KEND });
#endif /* !DEFINED_kwlist__hEvent */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_SetEvent_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *raw_hEvent;
	} args;
	HANDLE hEvent;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__hEvent, "o:SetEvent", &args))
		goto err;
	if unlikely(DeeNTSystem_TryGetHandle(args.raw_hEvent, (void **)&hEvent))
		goto err;
	return libwin32_SetEvent_f_impl(hEvent);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_SetEvent, &libwin32_SetEvent_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_SetEvent_f_impl(HANDLE hEvent)
/*[[[end]]]*/
{
	BOOL bOk;
again:
	DBG_ALIGNMENT_DISABLE();
	bOk = SetEvent(hEvent);
	if (!bOk) {
		DWORD dwError;
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(dwError)) {
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		RETURN_ERROR_OR_FALSE(dwError,
		                      "Failed to set event %p",
		                      hEvent);
	}
	DBG_ALIGNMENT_ENABLE();
	RETURN_SUCCESS_OR_TRUE;
err:
	return NULL;
}


/*[[[deemon import("rt.gen.dexutils").gw("CreateMutex",
     "lpMutexAttributes:?GSECURITY_ATTRIBUTES=NULL"
     ",bInitialOwner:c:bool=false"
     ",lpName:nt:LPCWSTR=NULL"
     "->" MAYBE_NONE("?GHANDLE")); ]]]*/
#define LIBWIN32_CREATEMUTEX_DEF          DEX_MEMBER_F("CreateMutex", &libwin32_CreateMutex, DEXSYM_READONLY, "(lpMutexAttributes?:?GSECURITY_ATTRIBUTES,bInitialOwner=!f,lpName?:?Dstring)->?GHANDLE"),
#define LIBWIN32_CREATEMUTEX_DEF_DOC(doc) DEX_MEMBER_F("CreateMutex", &libwin32_CreateMutex, DEXSYM_READONLY, "(lpMutexAttributes?:?GSECURITY_ATTRIBUTES,bInitialOwner=!f,lpName?:?Dstring)->?GHANDLE\n" doc),
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_CreateMutex_f_impl(DeeObject *lpMutexAttributes, bool bInitialOwner, LPCWSTR lpName);
#ifndef DEFINED_kwlist__lpMutexAttributes_bInitialOwner_lpName
#define DEFINED_kwlist__lpMutexAttributes_bInitialOwner_lpName
PRIVATE DEFINE_KWLIST(kwlist__lpMutexAttributes_bInitialOwner_lpName, { KEX("lpMutexAttributes", 0x64b462c9, 0xfd8837a2c4ac2274), KEX("bInitialOwner", 0xe74e22ce, 0x95fbc1180ff72ac8), KEX("lpName", 0xe3f0b9dd, 0x6d44dd71565f84d6), KEND });
#endif /* !DEFINED_kwlist__lpMutexAttributes_bInitialOwner_lpName */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_CreateMutex_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *lpMutexAttributes;
		bool bInitialOwner;
		LPCWSTR lpName;
	} args;
	args.lpMutexAttributes = NULL;
	args.bInitialOwner = false;
	args.lpName = NULL;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__lpMutexAttributes_bInitialOwner_lpName, "|obU16s:CreateMutex", &args))
		goto err;
	return libwin32_CreateMutex_f_impl(args.lpMutexAttributes, args.bInitialOwner, args.lpName);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_CreateMutex, &libwin32_CreateMutex_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_CreateMutex_f_impl(DeeObject *lpMutexAttributes, bool bInitialOwner, LPCWSTR lpName)
/*[[[end]]]*/
{
	HANDLE hResult;
	DREF DeeObject *result;
again:
	DBG_ALIGNMENT_DISABLE();
	(void)lpMutexAttributes; /* TODO */
	hResult = CreateMutexW(NULL, bInitialOwner, lpName);
	if (hResult == NULL) {
		DWORD dwError;
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(dwError)) {
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		RETURN_ERROR(dwError,
		             "Failed to create event (bInitialOwner: %u, lpName: %lq)",
		             (unsigned int)bInitialOwner, lpName);
	}
	DBG_ALIGNMENT_ENABLE();
	result = libwin32_CreateHandle(hResult);
	if likely(result)
		return result;
	CloseHandle(hResult);
err:
	return NULL;
}


/*[[[deemon import("rt.gen.dexutils").gw("OpenMutex",
     "dwDesiredAccess:nt:DWORD"
     ",bInheritHandle:c:bool"
     ",lpName:nt:LPCWSTR"
     "->" MAYBE_NONE("?GHANDLE")); ]]]*/
#define LIBWIN32_OPENMUTEX_DEF          DEX_MEMBER_F("OpenMutex", &libwin32_OpenMutex, DEXSYM_READONLY, "(dwDesiredAccess:?Dint,bInheritHandle:?Dbool,lpName:?Dstring)->?GHANDLE"),
#define LIBWIN32_OPENMUTEX_DEF_DOC(doc) DEX_MEMBER_F("OpenMutex", &libwin32_OpenMutex, DEXSYM_READONLY, "(dwDesiredAccess:?Dint,bInheritHandle:?Dbool,lpName:?Dstring)->?GHANDLE\n" doc),
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_OpenMutex_f_impl(DWORD dwDesiredAccess, bool bInheritHandle, LPCWSTR lpName);
#ifndef DEFINED_kwlist__dwDesiredAccess_bInheritHandle_lpName
#define DEFINED_kwlist__dwDesiredAccess_bInheritHandle_lpName
PRIVATE DEFINE_KWLIST(kwlist__dwDesiredAccess_bInheritHandle_lpName, { KEX("dwDesiredAccess", 0xa4c2866d, 0x5a4b46d72691f2f), KEX("bInheritHandle", 0x632313fa, 0x9e7213d20a3bbbb4), KEX("lpName", 0xe3f0b9dd, 0x6d44dd71565f84d6), KEND });
#endif /* !DEFINED_kwlist__dwDesiredAccess_bInheritHandle_lpName */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_OpenMutex_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DWORD dwDesiredAccess;
		bool bInheritHandle;
		LPCWSTR lpName;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__dwDesiredAccess_bInheritHandle_lpName, UNPu32 "bU16s:OpenMutex", &args))
		goto err;
	return libwin32_OpenMutex_f_impl(args.dwDesiredAccess, args.bInheritHandle, args.lpName);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_OpenMutex, &libwin32_OpenMutex_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_OpenMutex_f_impl(DWORD dwDesiredAccess, bool bInheritHandle, LPCWSTR lpName)
/*[[[end]]]*/
{
	HANDLE hResult;
	DREF DeeObject *result;
again:
	DBG_ALIGNMENT_DISABLE();
	hResult = OpenMutexW(dwDesiredAccess, bInheritHandle, lpName);
	if (hResult == NULL) {
		DWORD dwError;
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(dwError)) {
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		RETURN_ERROR(dwError,
		             "Failed to open mutex (dwDesiredAccess: %#" PRFx32 ", bInheritHandle: %u, lpName: %lq)",
		             dwDesiredAccess, (unsigned int)bInheritHandle, lpName);
	}
	DBG_ALIGNMENT_ENABLE();
	result = libwin32_CreateHandle(hResult);
	if likely(result)
		return result;
	CloseHandle(hResult);
err:
	return NULL;
}

/*[[[deemon import("rt.gen.dexutils").gw("ReleaseMutex",
      "hMutex:nt:HANDLE"
     "->" ERROR_OR_BOOL
     ); ]]]*/
#define LIBWIN32_RELEASEMUTEX_DEF          DEX_MEMBER_F("ReleaseMutex", &libwin32_ReleaseMutex, DEXSYM_READONLY, "(hMutex:?X3?Dint?DFile?Ewin32:HANDLE)"),
#define LIBWIN32_RELEASEMUTEX_DEF_DOC(doc) DEX_MEMBER_F("ReleaseMutex", &libwin32_ReleaseMutex, DEXSYM_READONLY, "(hMutex:?X3?Dint?DFile?Ewin32:HANDLE)\n" doc),
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_ReleaseMutex_f_impl(HANDLE hMutex);
#ifndef DEFINED_kwlist__hMutex
#define DEFINED_kwlist__hMutex
PRIVATE DEFINE_KWLIST(kwlist__hMutex, { KEX("hMutex", 0xb220eb59, 0x8cb14daab9cd0f46), KEND });
#endif /* !DEFINED_kwlist__hMutex */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_ReleaseMutex_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *raw_hMutex;
	} args;
	HANDLE hMutex;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__hMutex, "o:ReleaseMutex", &args))
		goto err;
	if unlikely(DeeNTSystem_TryGetHandle(args.raw_hMutex, (void **)&hMutex))
		goto err;
	return libwin32_ReleaseMutex_f_impl(hMutex);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_ReleaseMutex, &libwin32_ReleaseMutex_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_ReleaseMutex_f_impl(HANDLE hMutex)
/*[[[end]]]*/
{
	BOOL bOk;
again:
	DBG_ALIGNMENT_DISABLE();
	bOk = ReleaseMutex(hMutex);
	if (!bOk) {
		DWORD dwError;
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(dwError)) {
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		RETURN_ERROR_OR_FALSE(dwError,
		                      "Failed to release mutex %p",
		                      hMutex);
	}
	DBG_ALIGNMENT_ENABLE();
	RETURN_SUCCESS_OR_TRUE;
err:
	return NULL;
}


/*[[[deemon import("rt.gen.dexutils").gw("CreateSemaphore",
     "lpSemaphoreAttributes:?GSECURITY_ATTRIBUTES=NULL"
     ",lInitialCount:nt:LONG=0"
     ",lMaximumCount:nt:LONG=0x10000"
     ",lpName:nt:LPCWSTR=NULL"
     "->" MAYBE_NONE("?GHANDLE")); ]]]*/
#define LIBWIN32_CREATESEMAPHORE_DEF          DEX_MEMBER_F("CreateSemaphore", &libwin32_CreateSemaphore, DEXSYM_READONLY, "(lpSemaphoreAttributes?:?GSECURITY_ATTRIBUTES,lInitialCount=!0,lMaximumCount=!0x10000,lpName?:?Dstring)->?GHANDLE"),
#define LIBWIN32_CREATESEMAPHORE_DEF_DOC(doc) DEX_MEMBER_F("CreateSemaphore", &libwin32_CreateSemaphore, DEXSYM_READONLY, "(lpSemaphoreAttributes?:?GSECURITY_ATTRIBUTES,lInitialCount=!0,lMaximumCount=!0x10000,lpName?:?Dstring)->?GHANDLE\n" doc),
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_CreateSemaphore_f_impl(DeeObject *lpSemaphoreAttributes, LONG lInitialCount, LONG lMaximumCount, LPCWSTR lpName);
#ifndef DEFINED_kwlist__lpSemaphoreAttributes_lInitialCount_lMaximumCount_lpName
#define DEFINED_kwlist__lpSemaphoreAttributes_lInitialCount_lMaximumCount_lpName
PRIVATE DEFINE_KWLIST(kwlist__lpSemaphoreAttributes_lInitialCount_lMaximumCount_lpName, { KEX("lpSemaphoreAttributes", 0x239d72d5, 0xd2a262c921fff10b), KEX("lInitialCount", 0xda143079, 0xe307fd93d3df8105), KEX("lMaximumCount", 0xcf42e9b6, 0x2aa4f64d4875258f), KEX("lpName", 0xe3f0b9dd, 0x6d44dd71565f84d6), KEND });
#endif /* !DEFINED_kwlist__lpSemaphoreAttributes_lInitialCount_lMaximumCount_lpName */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_CreateSemaphore_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *lpSemaphoreAttributes;
		LONG lInitialCount;
		LONG lMaximumCount;
		LPCWSTR lpName;
	} args;
	args.lpSemaphoreAttributes = NULL;
	args.lInitialCount = 0;
	args.lMaximumCount = 0x10000;
	args.lpName = NULL;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__lpSemaphoreAttributes_lInitialCount_lMaximumCount_lpName, "|o" UNPd32 UNPd32 "U16s:CreateSemaphore", &args))
		goto err;
	return libwin32_CreateSemaphore_f_impl(args.lpSemaphoreAttributes, args.lInitialCount, args.lMaximumCount, args.lpName);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_CreateSemaphore, &libwin32_CreateSemaphore_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_CreateSemaphore_f_impl(DeeObject *lpSemaphoreAttributes, LONG lInitialCount, LONG lMaximumCount, LPCWSTR lpName)
/*[[[end]]]*/
{
	HANDLE hResult;
	DREF DeeObject *result;
again:
	DBG_ALIGNMENT_DISABLE();
	(void)lpSemaphoreAttributes; /* TODO */
	hResult = CreateSemaphoreW(NULL,
	                           lInitialCount,
	                           lMaximumCount,
	                           lpName);
	if (hResult == NULL) {
		DWORD dwError;
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(dwError)) {
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		RETURN_ERROR(dwError,
		             "Failed to create semaphore (lInitialCount: %" PRFu32 ", lMaximumCount: %" PRFu32 ", lpName: %lq)",
		             lInitialCount, lMaximumCount, lpName);
	}
	DBG_ALIGNMENT_ENABLE();
	result = libwin32_CreateHandle(hResult);
	if likely(result)
		return result;
	CloseHandle(hResult);
err:
	return NULL;
}


/*[[[deemon import("rt.gen.dexutils").gw("OpenSemaphore",
     "dwDesiredAccess:nt:DWORD"
     ",bInheritHandle:c:bool"
     ",lpName:nt:LPCWSTR"
     "->" MAYBE_NONE("?GHANDLE")); ]]]*/
#define LIBWIN32_OPENSEMAPHORE_DEF          DEX_MEMBER_F("OpenSemaphore", &libwin32_OpenSemaphore, DEXSYM_READONLY, "(dwDesiredAccess:?Dint,bInheritHandle:?Dbool,lpName:?Dstring)->?GHANDLE"),
#define LIBWIN32_OPENSEMAPHORE_DEF_DOC(doc) DEX_MEMBER_F("OpenSemaphore", &libwin32_OpenSemaphore, DEXSYM_READONLY, "(dwDesiredAccess:?Dint,bInheritHandle:?Dbool,lpName:?Dstring)->?GHANDLE\n" doc),
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_OpenSemaphore_f_impl(DWORD dwDesiredAccess, bool bInheritHandle, LPCWSTR lpName);
#ifndef DEFINED_kwlist__dwDesiredAccess_bInheritHandle_lpName
#define DEFINED_kwlist__dwDesiredAccess_bInheritHandle_lpName
PRIVATE DEFINE_KWLIST(kwlist__dwDesiredAccess_bInheritHandle_lpName, { KEX("dwDesiredAccess", 0xa4c2866d, 0x5a4b46d72691f2f), KEX("bInheritHandle", 0x632313fa, 0x9e7213d20a3bbbb4), KEX("lpName", 0xe3f0b9dd, 0x6d44dd71565f84d6), KEND });
#endif /* !DEFINED_kwlist__dwDesiredAccess_bInheritHandle_lpName */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_OpenSemaphore_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DWORD dwDesiredAccess;
		bool bInheritHandle;
		LPCWSTR lpName;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__dwDesiredAccess_bInheritHandle_lpName, UNPu32 "bU16s:OpenSemaphore", &args))
		goto err;
	return libwin32_OpenSemaphore_f_impl(args.dwDesiredAccess, args.bInheritHandle, args.lpName);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_OpenSemaphore, &libwin32_OpenSemaphore_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_OpenSemaphore_f_impl(DWORD dwDesiredAccess, bool bInheritHandle, LPCWSTR lpName)
/*[[[end]]]*/
{
	HANDLE hResult;
	DREF DeeObject *result;
again:
	DBG_ALIGNMENT_DISABLE();
	hResult = OpenSemaphoreW(dwDesiredAccess, bInheritHandle, lpName);
	if (hResult == NULL) {
		DWORD dwError;
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(dwError)) {
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		RETURN_ERROR(dwError,
		             "Failed to open semaphore (dwDesiredAccess: %#" PRFx32 ", bInheritHandle: %u, lpName: %lq)",
		             dwDesiredAccess, (unsigned int)bInheritHandle, lpName);
	}
	DBG_ALIGNMENT_ENABLE();
	result = libwin32_CreateHandle(hResult);
	if likely(result)
		return result;
	CloseHandle(hResult);
err:
	return NULL;
}

/*[[[deemon import("rt.gen.dexutils").gw("ReleaseSemaphore",
       "hSemaphore:nt:HANDLE"
      ",lReleaseCount:nt:LONG=1"
     "->" MAYBE_NONE("?Dint")
     ); ]]]*/
#define LIBWIN32_RELEASESEMAPHORE_DEF          DEX_MEMBER_F("ReleaseSemaphore", &libwin32_ReleaseSemaphore, DEXSYM_READONLY, "(hSemaphore:?X3?Dint?DFile?Ewin32:HANDLE,lReleaseCount=!1)->?Dint"),
#define LIBWIN32_RELEASESEMAPHORE_DEF_DOC(doc) DEX_MEMBER_F("ReleaseSemaphore", &libwin32_ReleaseSemaphore, DEXSYM_READONLY, "(hSemaphore:?X3?Dint?DFile?Ewin32:HANDLE,lReleaseCount=!1)->?Dint\n" doc),
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_ReleaseSemaphore_f_impl(HANDLE hSemaphore, LONG lReleaseCount);
#ifndef DEFINED_kwlist__hSemaphore_lReleaseCount
#define DEFINED_kwlist__hSemaphore_lReleaseCount
PRIVATE DEFINE_KWLIST(kwlist__hSemaphore_lReleaseCount, { KEX("hSemaphore", 0xbbfa5408, 0x2705db157a75a16a), KEX("lReleaseCount", 0x9d4745b1, 0xb38edeb0f0e9f5ed), KEND });
#endif /* !DEFINED_kwlist__hSemaphore_lReleaseCount */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_ReleaseSemaphore_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *raw_hSemaphore;
		LONG lReleaseCount;
	} args;
	HANDLE hSemaphore;
	args.lReleaseCount = 1;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__hSemaphore_lReleaseCount, "o|" UNPd32 ":ReleaseSemaphore", &args))
		goto err;
	if unlikely(DeeNTSystem_TryGetHandle(args.raw_hSemaphore, (void **)&hSemaphore))
		goto err;
	return libwin32_ReleaseSemaphore_f_impl(hSemaphore, args.lReleaseCount);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_ReleaseSemaphore, &libwin32_ReleaseSemaphore_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_ReleaseSemaphore_f_impl(HANDLE hSemaphore, LONG lReleaseCount)
/*[[[end]]]*/
{
	BOOL bOk;
	LONG lPreviousCount;
again:
	DBG_ALIGNMENT_DISABLE();
	bOk = ReleaseSemaphore(hSemaphore, lReleaseCount, &lPreviousCount);
	if (!bOk) {
		DWORD dwError;
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(dwError)) {
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		RETURN_ERROR(dwError,
		             "Failed to release semaphore %p",
		             hSemaphore);
	}
	DBG_ALIGNMENT_ENABLE();
	return DeeInt_NewUInt32(lPreviousCount);
err:
	return NULL;
}


/*[[[deemon import("rt.gen.dexutils").gw("CreateWaitableTimer",
     "lpTimerAttributes:?GSECURITY_ATTRIBUTES=NULL"
     ",bManualReset:c:bool=false"
     ",lpTimerName:nt:LPCWSTR=NULL"
     "->" MAYBE_NONE("?GHANDLE")); ]]]*/
#define LIBWIN32_CREATEWAITABLETIMER_DEF          DEX_MEMBER_F("CreateWaitableTimer", &libwin32_CreateWaitableTimer, DEXSYM_READONLY, "(lpTimerAttributes?:?GSECURITY_ATTRIBUTES,bManualReset=!f,lpTimerName?:?Dstring)->?GHANDLE"),
#define LIBWIN32_CREATEWAITABLETIMER_DEF_DOC(doc) DEX_MEMBER_F("CreateWaitableTimer", &libwin32_CreateWaitableTimer, DEXSYM_READONLY, "(lpTimerAttributes?:?GSECURITY_ATTRIBUTES,bManualReset=!f,lpTimerName?:?Dstring)->?GHANDLE\n" doc),
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_CreateWaitableTimer_f_impl(DeeObject *lpTimerAttributes, bool bManualReset, LPCWSTR lpTimerName);
#ifndef DEFINED_kwlist__lpTimerAttributes_bManualReset_lpTimerName
#define DEFINED_kwlist__lpTimerAttributes_bManualReset_lpTimerName
PRIVATE DEFINE_KWLIST(kwlist__lpTimerAttributes_bManualReset_lpTimerName, { KEX("lpTimerAttributes", 0x9ed0c388, 0xd46c1f1730c623a4), KEX("bManualReset", 0x8284a854, 0xe02d1cfc7c0f513f), KEX("lpTimerName", 0x200ed5a2, 0xd68e63741f652663), KEND });
#endif /* !DEFINED_kwlist__lpTimerAttributes_bManualReset_lpTimerName */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_CreateWaitableTimer_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *lpTimerAttributes;
		bool bManualReset;
		LPCWSTR lpTimerName;
	} args;
	args.lpTimerAttributes = NULL;
	args.bManualReset = false;
	args.lpTimerName = NULL;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__lpTimerAttributes_bManualReset_lpTimerName, "|obU16s:CreateWaitableTimer", &args))
		goto err;
	return libwin32_CreateWaitableTimer_f_impl(args.lpTimerAttributes, args.bManualReset, args.lpTimerName);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_CreateWaitableTimer, &libwin32_CreateWaitableTimer_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_CreateWaitableTimer_f_impl(DeeObject *lpTimerAttributes, bool bManualReset, LPCWSTR lpTimerName)
/*[[[end]]]*/
{
	HANDLE hResult;
	DREF DeeObject *result;
again:
	DBG_ALIGNMENT_DISABLE();
	(void)lpTimerAttributes; /* TODO */
	hResult = CreateWaitableTimerW(NULL,
	                               bManualReset,
	                               lpTimerName);
	if (hResult == NULL) {
		DWORD dwError;
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(dwError)) {
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		RETURN_ERROR(dwError,
		             "Failed to create waitable timer (bManualReset: %u, lpTimerName: %lq)",
		             (unsigned int)bManualReset, lpTimerName);
	}
	DBG_ALIGNMENT_ENABLE();
	result = libwin32_CreateHandle(hResult);
	if likely(result)
		return result;
	CloseHandle(hResult);
err:
	return NULL;
}


/*[[[deemon import("rt.gen.dexutils").gw("OpenWaitableTimer",
     "dwDesiredAccess:nt:DWORD"
     ",bInheritHandle:c:bool"
     ",lpName:nt:LPCWSTR"
     "->" MAYBE_NONE("?GHANDLE")); ]]]*/
#define LIBWIN32_OPENWAITABLETIMER_DEF          DEX_MEMBER_F("OpenWaitableTimer", &libwin32_OpenWaitableTimer, DEXSYM_READONLY, "(dwDesiredAccess:?Dint,bInheritHandle:?Dbool,lpName:?Dstring)->?GHANDLE"),
#define LIBWIN32_OPENWAITABLETIMER_DEF_DOC(doc) DEX_MEMBER_F("OpenWaitableTimer", &libwin32_OpenWaitableTimer, DEXSYM_READONLY, "(dwDesiredAccess:?Dint,bInheritHandle:?Dbool,lpName:?Dstring)->?GHANDLE\n" doc),
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_OpenWaitableTimer_f_impl(DWORD dwDesiredAccess, bool bInheritHandle, LPCWSTR lpName);
#ifndef DEFINED_kwlist__dwDesiredAccess_bInheritHandle_lpName
#define DEFINED_kwlist__dwDesiredAccess_bInheritHandle_lpName
PRIVATE DEFINE_KWLIST(kwlist__dwDesiredAccess_bInheritHandle_lpName, { KEX("dwDesiredAccess", 0xa4c2866d, 0x5a4b46d72691f2f), KEX("bInheritHandle", 0x632313fa, 0x9e7213d20a3bbbb4), KEX("lpName", 0xe3f0b9dd, 0x6d44dd71565f84d6), KEND });
#endif /* !DEFINED_kwlist__dwDesiredAccess_bInheritHandle_lpName */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_OpenWaitableTimer_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DWORD dwDesiredAccess;
		bool bInheritHandle;
		LPCWSTR lpName;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__dwDesiredAccess_bInheritHandle_lpName, UNPu32 "bU16s:OpenWaitableTimer", &args))
		goto err;
	return libwin32_OpenWaitableTimer_f_impl(args.dwDesiredAccess, args.bInheritHandle, args.lpName);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_OpenWaitableTimer, &libwin32_OpenWaitableTimer_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_OpenWaitableTimer_f_impl(DWORD dwDesiredAccess, bool bInheritHandle, LPCWSTR lpName)
/*[[[end]]]*/
{
	HANDLE hResult;
	DREF DeeObject *result;
again:
	DBG_ALIGNMENT_DISABLE();
	hResult = OpenWaitableTimerW(dwDesiredAccess, bInheritHandle, lpName);
	if (hResult == NULL) {
		DWORD dwError;
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(dwError)) {
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		RETURN_ERROR(dwError,
		             "Failed to open waitable timer (dwDesiredAccess: %#" PRFx32 ", bInheritHandle: %u, lpName: %lq)",
		             dwDesiredAccess, (unsigned int)bInheritHandle, lpName);
	}
	DBG_ALIGNMENT_ENABLE();
	result = libwin32_CreateHandle(hResult);
	if likely(result)
		return result;
	CloseHandle(hResult);
err:
	return NULL;
}


/*[[[deemon import("rt.gen.dexutils").gw("CancelWaitableTimer",
      "hTimer:nt:HANDLE"
     "->" ERROR_OR_BOOL
     ); ]]]*/
#define LIBWIN32_CANCELWAITABLETIMER_DEF          DEX_MEMBER_F("CancelWaitableTimer", &libwin32_CancelWaitableTimer, DEXSYM_READONLY, "(hTimer:?X3?Dint?DFile?Ewin32:HANDLE)"),
#define LIBWIN32_CANCELWAITABLETIMER_DEF_DOC(doc) DEX_MEMBER_F("CancelWaitableTimer", &libwin32_CancelWaitableTimer, DEXSYM_READONLY, "(hTimer:?X3?Dint?DFile?Ewin32:HANDLE)\n" doc),
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_CancelWaitableTimer_f_impl(HANDLE hTimer);
#ifndef DEFINED_kwlist__hTimer
#define DEFINED_kwlist__hTimer
PRIVATE DEFINE_KWLIST(kwlist__hTimer, { KEX("hTimer", 0x959d10e9, 0xebf100e7729ade7b), KEND });
#endif /* !DEFINED_kwlist__hTimer */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_CancelWaitableTimer_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *raw_hTimer;
	} args;
	HANDLE hTimer;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__hTimer, "o:CancelWaitableTimer", &args))
		goto err;
	if unlikely(DeeNTSystem_TryGetHandle(args.raw_hTimer, (void **)&hTimer))
		goto err;
	return libwin32_CancelWaitableTimer_f_impl(hTimer);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_CancelWaitableTimer, &libwin32_CancelWaitableTimer_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_CancelWaitableTimer_f_impl(HANDLE hTimer)
/*[[[end]]]*/
{
	BOOL bOk;
again:
	DBG_ALIGNMENT_DISABLE();
	bOk = CancelWaitableTimer(hTimer);
	if (!bOk) {
		DWORD dwError;
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(dwError)) {
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		RETURN_ERROR_OR_FALSE(dwError,
		                      "Failed to cancel timer %p",
		                      hTimer);
	}
	DBG_ALIGNMENT_ENABLE();
	RETURN_SUCCESS_OR_TRUE;
err:
	return NULL;
}


/*[[[deemon import("rt.gen.dexutils").gw("FixUncPath", "path:?Dstring->?Dstring"); ]]]*/
#define LIBWIN32_FIXUNCPATH_DEF          DEX_MEMBER_F("FixUncPath", &libwin32_FixUncPath, DEXSYM_READONLY, "(path:?Dstring)->?Dstring"),
#define LIBWIN32_FIXUNCPATH_DEF_DOC(doc) DEX_MEMBER_F("FixUncPath", &libwin32_FixUncPath, DEXSYM_READONLY, "(path:?Dstring)->?Dstring\n" doc),
FORCELOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL libwin32_FixUncPath_f_impl(DeeObject *path);
#ifndef DEFINED_kwlist__path
#define DEFINED_kwlist__path
PRIVATE DEFINE_KWLIST(kwlist__path, { KEX("path", 0x1ab74e01, 0xc2dd5992f362b3c4), KEND });
#endif /* !DEFINED_kwlist__path */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_FixUncPath_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *path;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__path, "o:FixUncPath", &args))
		goto err;
	return libwin32_FixUncPath_f_impl(args.path);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libwin32_FixUncPath, &libwin32_FixUncPath_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL libwin32_FixUncPath_f_impl(DeeObject *path)
/*[[[end]]]*/
{
	if (DeeObject_AssertTypeExact(path, &DeeString_Type))
		goto err;
	return DeeNTSystem_FixUncPath(path);
err:
	return NULL;
}










DEX_BEGIN

DEX_MEMBER_F_NODOC("HANDLE", &DeeHandle_Type, DEXSYM_READONLY),
DEX_MEMBER_F_NODOC("INVALID_HANDLE_VALUE", &Dee_INVALID_HANDLE_VALUE, DEXSYM_READONLY),
/* TODO: Wrapper types for `SECURITY_ATTRIBUTES' and `OVERLAPPED' */
/* TODO: Wrapper types for `WIN32_FIND_DATA' */

/* Deemon-specific helpers */
LIBWIN32_FIXUNCPATH_DEF_DOC("#t{:Interrupt}"
                            "Used ot fix a given path to become a valid UNC long-path.\n"
                            "The main purpose of doing this is to break the 260-character "
                            /**/ "limit of regular paths under windows and extend it to a "
                            /**/ "maximum of around ${2**16}.\n"
                            "Note that all system APIs directly exposed by deemon already "
                            /**/ "include handling to automatically apply this fix when you "
                            /**/ "pass a path that exceeds non-UNC limits, so using this "
                            /**/ "function shouldn't be necessary in most cases.")
LIBWIN32_GETHANDLE_DEF_DOC("Return the underlying system handle that is bound to the given object"
                           "#L-{"
                           /**/ "When ?N is given, always return ${HANDLE(0)}|"
                           /**/ "When an :int is given, return the result of ${get_osfhandle(ob)}|"
                           /**/ "When the given object has an attribute $" Dee_fd_osfhandle_GETSET ", return ${ob." Dee_fd_osfhandle_GETSET "}|"
                           /**/ "When the given object has an attribute $" Dee_fd_fileno_GETSET ", return ${get_osfhandle(ob." Dee_fd_osfhandle_GETSET ")}|"
                           /**/ "When another ?GHANDLE object is given, re-return that object"
                           "}"
                           "Note that these sames steps are also performed by all other "
                           /**/ "functions taking ?GHANDLE input arguments, as well as "
                           /**/ "all functions in the ?Mposix module accepting file descriptors "
                           /**/ "when compiled for a windows host (e.g. ?Eposix:openat will "
                           /**/ "accept a handle opened by ?GCreateFile).")

/* Error-related functions. */
LIBWIN32_GETLASTERROR_DEF
LIBWIN32_SETLASTERROR_DEF
LIBWIN32_FORMATERRORMESSAGE_DEF_DOC("Return a human-readable error message associated with "
                                    "the given @dwError (as returned by ?GGetLastError)")

/* Handle-related functions. */
LIBWIN32_CLOSEHANDLE_DEF
LIBWIN32_CREATEFILE_DEF
LIBWIN32_DUPLICATEHANDLE_DEF
LIBWIN32_READFILE_DEF_DOC("Returns #CdwNumberOfBytesRead")
LIBWIN32_WRITEFILE_DEF_DOC("Returns #CdwNumberOfBytesWritten")
LIBWIN32_SETENDOFFILE_DEF
LIBWIN32_SETFILEPOINTER_DEF_DOC("Returns the new stream position")
LIBWIN32_GETFILETIME_DEF_DOC("Returns a tuple #C{(CreationTime, LastAccessTime, LastWriteTime)} for the given @hFile")
LIBWIN32_SETFILETIME_DEF
LIBWIN32_SETFILEVALIDDATA_DEF
LIBWIN32_GETDISKFREESPACE_DEF_DOC("Returns a tuple #C{(uSectorsPerCluster, uBytesPerSector, uNumberOfFreeClusters, uTotalNumberOfClusters)}")
LIBWIN32_GETDISKFREESPACEEX_DEF_DOC("Returns a tuple #C{(uFreeBytesAvailableToCaller, uTotalNumberOfBytes, uTotalNumberOfFreeBytes)}")
LIBWIN32_GETTEMPPATH_DEF_DOC("Returns a string containing a temporary path")
LIBWIN32_GETDLLDIRECTORY_DEF_DOC("Returns a string describing the windows DLL directory")
LIBWIN32_SETDLLDIRECTORY_DEF_DOC("Set the windows DLL directory, as used when loading dynamic libraries, and as returned by ?GGetDllDirectory")
LIBWIN32_GETFILETYPE_DEF_DOC("Return one of #C{FILE_TYPE_*}")
LIBWIN32_GETFILESIZE_DEF_DOC("Return the size of the given @hFile")
LIBWIN32_GETDRIVETYPE_DEF_DOC("Returns the type of drive of @lpRootPathName (one of #C{DRIVE_*})")
LIBWIN32_GETFILEATTRIBUTES_DEF_DOC("Returns attributes for @lpFileName (set of #C{FILE_ATTRIBUTE_*})")
LIBWIN32_SETFILEATTRIBUTES_DEF_DOC("Set attributes for @lpFileName to @dwFileAttributes (set of #C{FILE_ATTRIBUTE_*})")
LIBWIN32_GETCOMPRESSEDFILESIZE_DEF
LIBWIN32_FLUSHFILEBUFFERS_DEF
LIBWIN32_GETFINALPATHNAMEBYHANDLE_DEF
LIBWIN32_GETFILENAMEOFHANDLE_DEF_DOC("Convenience wrapper for ?GGetFinalPathNameByHandle that also supports the "
                                     /**/ "${GetMappedFileName(MapViewOfFile(CreateFileMapping(hFile)))} workaround "
                                     /**/ "that is required on Windows XP\n"
                                     "Note that similar functionality is also provided by ?Eposix:frealpath")
LIBWIN32_WAITFORSINGLEOBJECT_DEF
LIBWIN32_WAITFORMULTIPLEOBJECTS_DEF
LIBWIN32_SIGNALOBJECTANDWAIT_DEF

/* STD handle control */
LIBWIN32_GETSTDHANDLE_DEF
LIBWIN32_SETSTDHANDLE_DEF

/* Memory functions */
LIBWIN32_MAPVIEWOFFILE_DEF
LIBWIN32_UNMAPVIEWOFFILE_DEF
LIBWIN32_CREATEFILEMAPPING_DEF

/* Process-/thread-control */
LIBWIN32_GETCURRENTPROCESS_DEF
LIBWIN32_GETCURRENTTHREAD_DEF
LIBWIN32_GETCURRENTPROCESSID_DEF
LIBWIN32_GETCURRENTTHREADID_DEF
LIBWIN32_OPENPROCESS_DEF
LIBWIN32_GETEXITCODEPROCESS_DEF
LIBWIN32_ENUMPROCESSMODULES_DEF
LIBWIN32_ENUMPROCESSES_DEF
LIBWIN32_GETPROCESSIMAGEFILENAME_DEF
LIBWIN32_GETMODULEBASENAME_DEF
LIBWIN32_GETMODULEFILENAMEEX_DEF
LIBWIN32_TERMINATEPROCESS_DEF
LIBWIN32_TERMINATETHREAD_DEF
LIBWIN32_SUSPENDTHREAD_DEF
LIBWIN32_RESUMETHREAD_DEF
LIBWIN32_NTQUERYINFORMATIONPROCESS_DEF
LIBWIN32_NTWOW64QUERYINFORMATIONPROCESS64_DEF
LIBWIN32_READPROCESSMEMORY_DEF
LIBWIN32_NTWOW64READVIRTUALMEMORY64_DEF
LIBWIN32_WRITEPROCESSMEMORY_DEF
LIBWIN32_NTWOW64WRITEVIRTUALMEMORY64_DEF

/* Filesystem functions */
LIBWIN32_REMOVEDIRECTORY_DEF
LIBWIN32_CREATEDIRECTORY_DEF
LIBWIN32_DELETEFILE_DEF
LIBWIN32_GETSYSTEMDIRECTORY_DEF_DOC("Returns the windows system directory ($r\"C:\\Windows\\system32\")")
LIBWIN32_GETWINDOWSDIRECTORY_DEF_DOC("Returns the windows directory ($r\"C:\\Windows\")")
LIBWIN32_GETSYSTEMWINDOWSDIRECTORY_DEF_DOC("Returns the system windows directory ($r\"C:\\Windows\")")
LIBWIN32_GETSYSTEMWOW64DIRECTORY_DEF_DOC("Returns the windows SysWOW64 directory ($r\"C:\\Windows\\SysWOW64\")")
LIBWIN32_GETLOGICALDRIVESTRINGS_DEF_DOC("Returns a list of known system drives ($({ r\"C:\\\", r\"D:\\\", r\"E:\\\" }))")
LIBWIN32_QUERYDOSDEVICE_DEF_DOC("Returns a list of DOS devices mounted under the given drive (which should be one of ?GGetLogicalDriveStrings)")

/* DLL functions */
LIBWIN32_GETMODULEFILENAME_DEF
LIBWIN32_GETMAPPEDFILENAME_DEF

/* Named pipe API. */
LIBWIN32_CREATENAMEDPIPE_DEF
LIBWIN32_CONNECTNAMEDPIPE_DEF
LIBWIN32_WAITNAMEDPIPE_DEF

/* Mutex. */
LIBWIN32_CREATEMUTEX_DEF
LIBWIN32_OPENMUTEX_DEF
LIBWIN32_RELEASEMUTEX_DEF

/* Event. */
LIBWIN32_CREATEEVENT_DEF
LIBWIN32_OPENEVENT_DEF
LIBWIN32_RESETEVENT_DEF
LIBWIN32_SETEVENT_DEF

/* Semaphore. */
LIBWIN32_CREATESEMAPHORE_DEF
LIBWIN32_OPENSEMAPHORE_DEF
LIBWIN32_RELEASESEMAPHORE_DEF

/* Waitable Timer. */
LIBWIN32_CREATEWAITABLETIMER_DEF
LIBWIN32_OPENWAITABLETIMER_DEF
LIBWIN32_CANCELWAITABLETIMER_DEF
/* TODO: SetWaitableTimer */

/* Misc. */
LIBWIN32_SLEEP_DEF

/* Include Windows constants. */
LIBWIN32_CONSTANTS_DEFS

DEX_END(NULL, NULL, NULL);

DECL_END

#else /* CONFIG_HOST_WINDOWS */
#include <deemon/dex.h>

DECL_BEGIN

DEX_BEGIN
/* --- Nothing here --- */
DEX_END(NULL, NULL, NULL);

DECL_END
#endif /* CONFIG_HOST_WINDOWS */

#endif /* !GUARD_DEX_WIN32_LIBWIN32_C */
