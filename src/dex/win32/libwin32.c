/* Copyright (c) 2018-2023 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2023 Griefer@Work                           *
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
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/bytes.h>
#include <deemon/dex.h>
#include <deemon/error.h>
#include <deemon/file.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/objmethod.h>
#include <deemon/seq.h>
#include <deemon/system-features.h> /* memcpyc(), ... */
#include <deemon/system.h>
#include <deemon/thread.h>
#include <deemon/tuple.h>
#include <deemon/util/atomic.h>

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
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_hHandle,
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


PRIVATE struct Dee_type_math handle_math = {
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

PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
handle_print(DeeHandleObject *__restrict self,
             dformatprinter printer, void *arg) {
	return DeeFormat_Printf(printer, arg, "<handle %p>", self->ho_handle);
}

PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
handle_printrepr(DeeHandleObject *__restrict self,
                 dformatprinter printer, void *arg) {
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

#define DEFINE_HANDLE_COMPARE(name, op)                           \
	PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL         \
	name(DeeHandleObject *self, DeeObject *some_object) {         \
		HANDLE hOtherHandle;                                      \
		if (DeeNTSystem_TryGetHandle(some_object, &hOtherHandle)) \
			goto err;                                             \
		return_bool(self->ho_handle op hOtherHandle);             \
	err:                                                          \
		return NULL;                                              \
	}
DEFINE_HANDLE_COMPARE(handle_eq, ==)
DEFINE_HANDLE_COMPARE(handle_ne, !=)
DEFINE_HANDLE_COMPARE(handle_lo, <)
DEFINE_HANDLE_COMPARE(handle_le, <=)
DEFINE_HANDLE_COMPARE(handle_gr, >)
DEFINE_HANDLE_COMPARE(handle_ge, >=)
#undef DEFINE_HANDLE_COMPARE

PRIVATE struct type_cmp handle_cmp = {
	/* .tp_hash = */ (dhash_t (DCALL *)(DeeObject *__restrict))&handle_hash,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&handle_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&handle_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&handle_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&handle_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&handle_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&handle_ge,
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
				/* .tp_ctor        = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor   = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor   = */ (dfunptr_t)NULL,
				/* .tp_any_ctor    = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(DeeHandleObject),
				/* .tp_any_ctor_kw = */ (dfunptr_t)&handle_init_kw,
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
		/* .tp_print     = */ (dssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&handle_print,
		/* .tp_printrepr = */ (dssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&handle_printrepr
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ &handle_math,
	/* .tp_cmp           = */ &handle_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
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
PRIVATE DREF DeeObject *DCALL
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
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetHandle_f_impl(HANDLE hHandle);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetHandle_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_GETHANDLE_DEF { "GetHandle", (DeeObject *)&libwin32_GetHandle, MODSYM_FNORMAL, DOC("(hHandle:?X3?Dint?DFile?Ewin32:HANDLE)->?GHANDLE") },
#define LIBWIN32_GETHANDLE_DEF_DOC(doc) { "GetHandle", (DeeObject *)&libwin32_GetHandle, MODSYM_FNORMAL, DOC("(hHandle:?X3?Dint?DFile?Ewin32:HANDLE)->?GHANDLE\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_GetHandle, &libwin32_GetHandle_f);
#ifndef LIBWIN32_KWDS_HHANDLE_DEFINED
#define LIBWIN32_KWDS_HHANDLE_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_hHandle, { K(hHandle), KEND });
#endif /* !LIBWIN32_KWDS_HHANDLE_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetHandle_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	HANDLE hhHandle;
	DeeObject *hHandle;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_hHandle, "o:GetHandle", &hHandle))
		goto err;
	if (DeeNTSystem_TryGetHandle(hHandle, (void **)&hhHandle))
		goto err;
	return libwin32_GetHandle_f_impl(hhHandle);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetHandle_f_impl(HANDLE hHandle)
/*[[[end]]]*/
{
	return libwin32_CreateHandle(hHandle);
}

/*[[[deemon import("rt.gen.dexutils").gw("GetLastError", "->?Dint"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetLastError_f_impl(void);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetLastError_f(size_t argc, DeeObject *const *argv);
#define LIBWIN32_GETLASTERROR_DEF { "GetLastError", (DeeObject *)&libwin32_GetLastError, MODSYM_FNORMAL, DOC("->?Dint") },
#define LIBWIN32_GETLASTERROR_DEF_DOC(doc) { "GetLastError", (DeeObject *)&libwin32_GetLastError, MODSYM_FNORMAL, DOC("->?Dint\n" doc) },
PRIVATE DEFINE_CMETHOD(libwin32_GetLastError, libwin32_GetLastError_f);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetLastError_f(size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":GetLastError"))
		goto err;
	return libwin32_GetLastError_f_impl();
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetLastError_f_impl(void)
/*[[[end]]]*/
{
	return DeeInt_NewU32((uint32_t)GetLastError());
}

/*[[[deemon import("rt.gen.dexutils").gw("SetLastError", "dwErrCode:nt:DWORD"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_SetLastError_f_impl(DWORD dwErrCode);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_SetLastError_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_SETLASTERROR_DEF { "SetLastError", (DeeObject *)&libwin32_SetLastError, MODSYM_FNORMAL, DOC("(dwErrCode:?Dint)") },
#define LIBWIN32_SETLASTERROR_DEF_DOC(doc) { "SetLastError", (DeeObject *)&libwin32_SetLastError, MODSYM_FNORMAL, DOC("(dwErrCode:?Dint)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_SetLastError, &libwin32_SetLastError_f);
#ifndef LIBWIN32_KWDS_DWERRCODE_DEFINED
#define LIBWIN32_KWDS_DWERRCODE_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_dwErrCode, { K(dwErrCode), KEND });
#endif /* !LIBWIN32_KWDS_DWERRCODE_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_SetLastError_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DWORD dwErrCode;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_dwErrCode, UNPu32 ":SetLastError", &dwErrCode))
		goto err;
	return libwin32_SetLastError_f_impl(dwErrCode);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_SetLastError_f_impl(DWORD dwErrCode)
/*[[[end]]]*/
{
	SetLastError(dwErrCode);
	return_none;
}

/*[[[deemon import("rt.gen.dexutils").gw("CloseHandle", "hObject:nt:HANDLE" ERROR_OR_BOOL); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_CloseHandle_f_impl(HANDLE hObject);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_CloseHandle_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_CLOSEHANDLE_DEF { "CloseHandle", (DeeObject *)&libwin32_CloseHandle, MODSYM_FNORMAL, DOC("(hObject:?X3?Dint?DFile?Ewin32:HANDLE)") },
#define LIBWIN32_CLOSEHANDLE_DEF_DOC(doc) { "CloseHandle", (DeeObject *)&libwin32_CloseHandle, MODSYM_FNORMAL, DOC("(hObject:?X3?Dint?DFile?Ewin32:HANDLE)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_CloseHandle, &libwin32_CloseHandle_f);
#ifndef LIBWIN32_KWDS_HOBJECT_DEFINED
#define LIBWIN32_KWDS_HOBJECT_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_hObject, { K(hObject), KEND });
#endif /* !LIBWIN32_KWDS_HOBJECT_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_CloseHandle_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	HANDLE hhObject;
	DeeObject *hObject;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_hObject, "o:CloseHandle", &hObject))
		goto err;
	if (DeeNTSystem_TryGetHandle(hObject, (void **)&hhObject))
		goto err;
	return libwin32_CloseHandle_f_impl(hhObject);
err:
	return NULL;
}
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
     ",dwOptions:nt:DWORD=DUPLICATE_SAME_ACCESS"
     "->" MAYBE_NONE("?GHANDLE")
     ); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_DuplicateHandle_f_impl(HANDLE hSourceProcessHandle, HANDLE hSourceHandle, HANDLE hTargetProcessHandle, DWORD dwDesiredAccess, bool bInheritHandle, DWORD dwOptions);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_DuplicateHandle_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_DUPLICATEHANDLE_DEF { "DuplicateHandle", (DeeObject *)&libwin32_DuplicateHandle, MODSYM_FNORMAL, DOC("(hSourceProcessHandle:?X3?Dint?DFile?Ewin32:HANDLE,hSourceHandle:?X3?Dint?DFile?Ewin32:HANDLE,hTargetProcessHandle:?X3?Dint?DFile?Ewin32:HANDLE,dwDesiredAccess:?Dint=!0,bInheritHandle:?Dbool=!t,dwOptions:?Dint=!GDUPLICATE_SAME_ACCESS)->?GHANDLE") },
#define LIBWIN32_DUPLICATEHANDLE_DEF_DOC(doc) { "DuplicateHandle", (DeeObject *)&libwin32_DuplicateHandle, MODSYM_FNORMAL, DOC("(hSourceProcessHandle:?X3?Dint?DFile?Ewin32:HANDLE,hSourceHandle:?X3?Dint?DFile?Ewin32:HANDLE,hTargetProcessHandle:?X3?Dint?DFile?Ewin32:HANDLE,dwDesiredAccess:?Dint=!0,bInheritHandle:?Dbool=!t,dwOptions:?Dint=!GDUPLICATE_SAME_ACCESS)->?GHANDLE\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_DuplicateHandle, &libwin32_DuplicateHandle_f);
#ifndef LIBWIN32_KWDS_HSOURCEPROCESSHANDLE_HSOURCEHANDLE_HTARGETPROCESSHANDLE_DWDESIREDACCESS_BINHERITHANDLE_DWOPTIONS_DEFINED
#define LIBWIN32_KWDS_HSOURCEPROCESSHANDLE_HSOURCEHANDLE_HTARGETPROCESSHANDLE_DWDESIREDACCESS_BINHERITHANDLE_DWOPTIONS_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_hSourceProcessHandle_hSourceHandle_hTargetProcessHandle_dwDesiredAccess_bInheritHandle_dwOptions, { K(hSourceProcessHandle), K(hSourceHandle), K(hTargetProcessHandle), K(dwDesiredAccess), K(bInheritHandle), K(dwOptions), KEND });
#endif /* !LIBWIN32_KWDS_HSOURCEPROCESSHANDLE_HSOURCEHANDLE_HTARGETPROCESSHANDLE_DWDESIREDACCESS_BINHERITHANDLE_DWOPTIONS_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_DuplicateHandle_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	HANDLE hhSourceProcessHandle;
	DeeObject *hSourceProcessHandle;
	HANDLE hhSourceHandle;
	DeeObject *hSourceHandle;
	HANDLE hhTargetProcessHandle;
	DeeObject *hTargetProcessHandle;
	DWORD dwDesiredAccess = 0;
	bool bInheritHandle = true;
	DWORD dwOptions = DUPLICATE_SAME_ACCESS;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_hSourceProcessHandle_hSourceHandle_hTargetProcessHandle_dwDesiredAccess_bInheritHandle_dwOptions, "ooo|" UNPu32 "b" UNPu32 ":DuplicateHandle", &hSourceProcessHandle, &hSourceHandle, &hTargetProcessHandle, &dwDesiredAccess, &bInheritHandle, &dwOptions))
		goto err;
	if (DeeNTSystem_TryGetHandle(hSourceProcessHandle, (void **)&hhSourceProcessHandle))
		goto err;
	if (DeeNTSystem_TryGetHandle(hSourceHandle, (void **)&hhSourceHandle))
		goto err;
	if (DeeNTSystem_TryGetHandle(hTargetProcessHandle, (void **)&hhTargetProcessHandle))
		goto err;
	return libwin32_DuplicateHandle_f_impl(hhSourceProcessHandle, hhSourceHandle, hhTargetProcessHandle, dwDesiredAccess, bInheritHandle, dwOptions);
err:
	return NULL;
}
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

/*[[[deemon import("rt.gen.dexutils").gw("CreateFile",
      "lpFileName:o"
     ",dwDesiredAccess:nt:DWORD=FILE_GENERIC_READ"
     ",dwShareMode:nt:DWORD=FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE"
     ",lpSecurityAttributes:?GSECURITY_ATTRIBUTES=NULL"
     ",dwCreationDisposition:nt:DWORD=OPEN_EXISTING"
     ",dwFlagsAndAttributes:nt:DWORD=FILE_ATTRIBUTE_NORMAL"
     ",hTemplateFile:nt:HANDLE=0"
     "->" MAYBE_NONE("?GHANDLE")
     ); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_CreateFile_f_impl(DeeObject *lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, DeeObject *lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_CreateFile_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_CREATEFILE_DEF { "CreateFile", (DeeObject *)&libwin32_CreateFile, MODSYM_FNORMAL, DOC("(lpFileName,dwDesiredAccess:?Dint=!GFILE_GENERIC_READ,dwShareMode:?Dint=FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,lpSecurityAttributes?:?GSECURITY_ATTRIBUTES,dwCreationDisposition:?Dint=!GOPEN_EXISTING,dwFlagsAndAttributes:?Dint=!GFILE_ATTRIBUTE_NORMAL,hTemplateFile:?X3?Dint?DFile?Ewin32:HANDLE=!0)->?GHANDLE") },
#define LIBWIN32_CREATEFILE_DEF_DOC(doc) { "CreateFile", (DeeObject *)&libwin32_CreateFile, MODSYM_FNORMAL, DOC("(lpFileName,dwDesiredAccess:?Dint=!GFILE_GENERIC_READ,dwShareMode:?Dint=FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,lpSecurityAttributes?:?GSECURITY_ATTRIBUTES,dwCreationDisposition:?Dint=!GOPEN_EXISTING,dwFlagsAndAttributes:?Dint=!GFILE_ATTRIBUTE_NORMAL,hTemplateFile:?X3?Dint?DFile?Ewin32:HANDLE=!0)->?GHANDLE\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_CreateFile, &libwin32_CreateFile_f);
#ifndef LIBWIN32_KWDS_LPFILENAME_DWDESIREDACCESS_DWSHAREMODE_LPSECURITYATTRIBUTES_DWCREATIONDISPOSITION_DWFLAGSANDATTRIBUTES_HTEMPLATEFILE_DEFINED
#define LIBWIN32_KWDS_LPFILENAME_DWDESIREDACCESS_DWSHAREMODE_LPSECURITYATTRIBUTES_DWCREATIONDISPOSITION_DWFLAGSANDATTRIBUTES_HTEMPLATEFILE_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_lpFileName_dwDesiredAccess_dwShareMode_lpSecurityAttributes_dwCreationDisposition_dwFlagsAndAttributes_hTemplateFile, { K(lpFileName), K(dwDesiredAccess), K(dwShareMode), K(lpSecurityAttributes), K(dwCreationDisposition), K(dwFlagsAndAttributes), K(hTemplateFile), KEND });
#endif /* !LIBWIN32_KWDS_LPFILENAME_DWDESIREDACCESS_DWSHAREMODE_LPSECURITYATTRIBUTES_DWCREATIONDISPOSITION_DWFLAGSANDATTRIBUTES_HTEMPLATEFILE_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_CreateFile_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *lpFileName;
	DWORD dwDesiredAccess = FILE_GENERIC_READ;
	DWORD dwShareMode = FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE;
	DeeObject *lpSecurityAttributes = NULL;
	DWORD dwCreationDisposition = OPEN_EXISTING;
	DWORD dwFlagsAndAttributes = FILE_ATTRIBUTE_NORMAL;
	HANDLE hhTemplateFile = 0;
	DeeObject *hTemplateFile = (DeeObject *)Dee_None;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_lpFileName_dwDesiredAccess_dwShareMode_lpSecurityAttributes_dwCreationDisposition_dwFlagsAndAttributes_hTemplateFile, "o|" UNPu32 UNPu32 "o" UNPu32 UNPu32 "o:CreateFile", &lpFileName, &dwDesiredAccess, &dwShareMode, &lpSecurityAttributes, &dwCreationDisposition, &dwFlagsAndAttributes, &hTemplateFile))
		goto err;
	if (!DeeNone_Check(hTemplateFile)) {
		if (DeeNTSystem_TryGetHandle(hTemplateFile, (void **)&hhTemplateFile))
			goto err;
	}
	return libwin32_CreateFile_f_impl(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hhTemplateFile);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_CreateFile_f_impl(DeeObject *lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, DeeObject *lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
/*[[[end]]]*/
{
	HANDLE hResult;
	DREF DeeObject *result;
again:
	DBG_ALIGNMENT_DISABLE();
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
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(dwError)) {
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		RETURN_ERROR(dwError,
		             "Failed to open file %lq (dwDesiredAccess: %#" PRFx32 ", dwShareMode: %#" PRFx32 ", "
		             "dwCreationDisposition: %#" PRFx32 ", dwFlagsAndAttributes: %#" PRFx32 ", hTemplateFile: %p)",
		             lpFileName, dwDesiredAccess, dwShareMode,
		             dwCreationDisposition, dwFlagsAndAttributes,
		             hTemplateFile);
	}
	DBG_ALIGNMENT_ENABLE();
	result = libwin32_CreateHandle(hResult);
	if likely(result)
		return result;
	CloseHandle(hResult);
err:
	return NULL;
}

/*[[[deemon import("rt.gen.dexutils").gw("WriteFile",
      "hFile:nt:HANDLE"
     ",lpBuffer:obj:buffer"
     ",lpOverlapped:?GOVERLAPPED=NULL"
     "->" MAYBE_NONE("?Dint")
     ); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_WriteFile_f_impl(HANDLE hFile, DeeObject *lpBuffer, DeeObject *lpOverlapped);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_WriteFile_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_WRITEFILE_DEF { "WriteFile", (DeeObject *)&libwin32_WriteFile, MODSYM_FNORMAL, DOC("(hFile:?X3?Dint?DFile?Ewin32:HANDLE,lpBuffer:?DBytes,lpOverlapped?:?GOVERLAPPED)->?Dint") },
#define LIBWIN32_WRITEFILE_DEF_DOC(doc) { "WriteFile", (DeeObject *)&libwin32_WriteFile, MODSYM_FNORMAL, DOC("(hFile:?X3?Dint?DFile?Ewin32:HANDLE,lpBuffer:?DBytes,lpOverlapped?:?GOVERLAPPED)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_WriteFile, &libwin32_WriteFile_f);
#ifndef LIBWIN32_KWDS_HFILE_LPBUFFER_LPOVERLAPPED_DEFINED
#define LIBWIN32_KWDS_HFILE_LPBUFFER_LPOVERLAPPED_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_hFile_lpBuffer_lpOverlapped, { K(hFile), K(lpBuffer), K(lpOverlapped), KEND });
#endif /* !LIBWIN32_KWDS_HFILE_LPBUFFER_LPOVERLAPPED_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_WriteFile_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	HANDLE hhFile;
	DeeObject *hFile;
	DeeObject *lpBuffer;
	DeeObject *lpOverlapped = NULL;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_hFile_lpBuffer_lpOverlapped, "oo|o:WriteFile", &hFile, &lpBuffer, &lpOverlapped))
		goto err;
	if (DeeNTSystem_TryGetHandle(hFile, (void **)&hhFile))
		goto err;
	return libwin32_WriteFile_f_impl(hhFile, lpBuffer, lpOverlapped);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_WriteFile_f_impl(HANDLE hFile, DeeObject *lpBuffer, DeeObject *lpOverlapped)
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
	result = DeeInt_NewU32((uint32_t)dwNumberOfBytesWritten);
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
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_ReadFile_f_impl(HANDLE hFile, DeeObject *lpBuffer, DeeObject *lpOverlapped);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_ReadFile_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_READFILE_DEF { "ReadFile", (DeeObject *)&libwin32_ReadFile, MODSYM_FNORMAL, DOC("(hFile:?X3?Dint?DFile?Ewin32:HANDLE,lpBuffer:?DBytes,lpOverlapped?:?GOVERLAPPED)->?Dint") },
#define LIBWIN32_READFILE_DEF_DOC(doc) { "ReadFile", (DeeObject *)&libwin32_ReadFile, MODSYM_FNORMAL, DOC("(hFile:?X3?Dint?DFile?Ewin32:HANDLE,lpBuffer:?DBytes,lpOverlapped?:?GOVERLAPPED)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_ReadFile, &libwin32_ReadFile_f);
#ifndef LIBWIN32_KWDS_HFILE_LPBUFFER_LPOVERLAPPED_DEFINED
#define LIBWIN32_KWDS_HFILE_LPBUFFER_LPOVERLAPPED_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_hFile_lpBuffer_lpOverlapped, { K(hFile), K(lpBuffer), K(lpOverlapped), KEND });
#endif /* !LIBWIN32_KWDS_HFILE_LPBUFFER_LPOVERLAPPED_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_ReadFile_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	HANDLE hhFile;
	DeeObject *hFile;
	DeeObject *lpBuffer;
	DeeObject *lpOverlapped = NULL;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_hFile_lpBuffer_lpOverlapped, "oo|o:ReadFile", &hFile, &lpBuffer, &lpOverlapped))
		goto err;
	if (DeeNTSystem_TryGetHandle(hFile, (void **)&hhFile))
		goto err;
	return libwin32_ReadFile_f_impl(hhFile, lpBuffer, lpOverlapped);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_ReadFile_f_impl(HANDLE hFile, DeeObject *lpBuffer, DeeObject *lpOverlapped)
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
	result = DeeInt_NewU32((uint32_t)dwNumberOfBytesRead);
	DeeObject_PutBuf(lpBuffer, &buffer, Dee_BUFFER_FWRITABLE);
	return result;
err:
	return NULL;
}


/*[[[deemon import("rt.gen.dexutils").gw("CreateDirectory", "lpPathName:nt:LPCWSTR,lpSecurityAttributes:?GSECURITY_ATTRIBUTES=NULL" ERROR_OR_BOOL); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_CreateDirectory_f_impl(LPCWSTR lpPathName, DeeObject *lpSecurityAttributes);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_CreateDirectory_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_CREATEDIRECTORY_DEF { "CreateDirectory", (DeeObject *)&libwin32_CreateDirectory, MODSYM_FNORMAL, DOC("(lpPathName:?Dstring,lpSecurityAttributes?:?GSECURITY_ATTRIBUTES)") },
#define LIBWIN32_CREATEDIRECTORY_DEF_DOC(doc) { "CreateDirectory", (DeeObject *)&libwin32_CreateDirectory, MODSYM_FNORMAL, DOC("(lpPathName:?Dstring,lpSecurityAttributes?:?GSECURITY_ATTRIBUTES)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_CreateDirectory, &libwin32_CreateDirectory_f);
#ifndef LIBWIN32_KWDS_LPPATHNAME_LPSECURITYATTRIBUTES_DEFINED
#define LIBWIN32_KWDS_LPPATHNAME_LPSECURITYATTRIBUTES_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_lpPathName_lpSecurityAttributes, { K(lpPathName), K(lpSecurityAttributes), KEND });
#endif /* !LIBWIN32_KWDS_LPPATHNAME_LPSECURITYATTRIBUTES_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_CreateDirectory_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	LPCWSTR lpPathName_str;
	DeeStringObject *lpPathName;
	DeeObject *lpSecurityAttributes = NULL;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_lpPathName_lpSecurityAttributes, "o|o:CreateDirectory", &lpPathName, &lpSecurityAttributes))
		goto err;
	if (DeeObject_AssertTypeExact(lpPathName, &DeeString_Type))
		goto err;
	lpPathName_str = (LPCWSTR)DeeString_AsWide((DeeObject *)lpPathName);
	if unlikely(!lpPathName_str)
		goto err;
	return libwin32_CreateDirectory_f_impl(lpPathName_str, lpSecurityAttributes);
err:
	return NULL;
}
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
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_RemoveDirectory_f_impl(LPCWSTR lpPathName);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_RemoveDirectory_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_REMOVEDIRECTORY_DEF { "RemoveDirectory", (DeeObject *)&libwin32_RemoveDirectory, MODSYM_FNORMAL, DOC("(lpPathName:?Dstring)") },
#define LIBWIN32_REMOVEDIRECTORY_DEF_DOC(doc) { "RemoveDirectory", (DeeObject *)&libwin32_RemoveDirectory, MODSYM_FNORMAL, DOC("(lpPathName:?Dstring)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_RemoveDirectory, &libwin32_RemoveDirectory_f);
#ifndef LIBWIN32_KWDS_LPPATHNAME_DEFINED
#define LIBWIN32_KWDS_LPPATHNAME_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_lpPathName, { K(lpPathName), KEND });
#endif /* !LIBWIN32_KWDS_LPPATHNAME_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_RemoveDirectory_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	LPCWSTR lpPathName_str;
	DeeStringObject *lpPathName;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_lpPathName, "o:RemoveDirectory", &lpPathName))
		goto err;
	if (DeeObject_AssertTypeExact(lpPathName, &DeeString_Type))
		goto err;
	lpPathName_str = (LPCWSTR)DeeString_AsWide((DeeObject *)lpPathName);
	if unlikely(!lpPathName_str)
		goto err;
	return libwin32_RemoveDirectory_f_impl(lpPathName_str);
err:
	return NULL;
}
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
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_DeleteFile_f_impl(LPCWSTR lpFileName);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_DeleteFile_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_DELETEFILE_DEF { "DeleteFile", (DeeObject *)&libwin32_DeleteFile, MODSYM_FNORMAL, DOC("(lpFileName:?Dstring)") },
#define LIBWIN32_DELETEFILE_DEF_DOC(doc) { "DeleteFile", (DeeObject *)&libwin32_DeleteFile, MODSYM_FNORMAL, DOC("(lpFileName:?Dstring)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_DeleteFile, &libwin32_DeleteFile_f);
#ifndef LIBWIN32_KWDS_LPFILENAME_DEFINED
#define LIBWIN32_KWDS_LPFILENAME_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_lpFileName, { K(lpFileName), KEND });
#endif /* !LIBWIN32_KWDS_LPFILENAME_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_DeleteFile_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	LPCWSTR lpFileName_str;
	DeeStringObject *lpFileName;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_lpFileName, "o:DeleteFile", &lpFileName))
		goto err;
	if (DeeObject_AssertTypeExact(lpFileName, &DeeString_Type))
		goto err;
	lpFileName_str = (LPCWSTR)DeeString_AsWide((DeeObject *)lpFileName);
	if unlikely(!lpFileName_str)
		goto err;
	return libwin32_DeleteFile_f_impl(lpFileName_str);
err:
	return NULL;
}
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
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_SetEndOfFile_f_impl(HANDLE hFile);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_SetEndOfFile_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_SETENDOFFILE_DEF { "SetEndOfFile", (DeeObject *)&libwin32_SetEndOfFile, MODSYM_FNORMAL, DOC("(hFile:?X3?Dint?DFile?Ewin32:HANDLE)") },
#define LIBWIN32_SETENDOFFILE_DEF_DOC(doc) { "SetEndOfFile", (DeeObject *)&libwin32_SetEndOfFile, MODSYM_FNORMAL, DOC("(hFile:?X3?Dint?DFile?Ewin32:HANDLE)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_SetEndOfFile, &libwin32_SetEndOfFile_f);
#ifndef LIBWIN32_KWDS_HFILE_DEFINED
#define LIBWIN32_KWDS_HFILE_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_hFile, { K(hFile), KEND });
#endif /* !LIBWIN32_KWDS_HFILE_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_SetEndOfFile_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	HANDLE hhFile;
	DeeObject *hFile;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_hFile, "o:SetEndOfFile", &hFile))
		goto err;
	if (DeeNTSystem_TryGetHandle(hFile, (void **)&hhFile))
		goto err;
	return libwin32_SetEndOfFile_f_impl(hhFile);
err:
	return NULL;
}
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

/*[[[deemon import("rt.gen.dexutils").gw("SetFileAttributesW", "lpFileName:nt:LPCWSTR,dwFileAttributes:nt:DWORD" ERROR_OR_BOOL); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_SetFileAttributesW_f_impl(LPCWSTR lpFileName, DWORD dwFileAttributes);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_SetFileAttributesW_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_SETFILEATTRIBUTESW_DEF { "SetFileAttributesW", (DeeObject *)&libwin32_SetFileAttributesW, MODSYM_FNORMAL, DOC("(lpFileName:?Dstring,dwFileAttributes:?Dint)") },
#define LIBWIN32_SETFILEATTRIBUTESW_DEF_DOC(doc) { "SetFileAttributesW", (DeeObject *)&libwin32_SetFileAttributesW, MODSYM_FNORMAL, DOC("(lpFileName:?Dstring,dwFileAttributes:?Dint)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_SetFileAttributesW, &libwin32_SetFileAttributesW_f);
#ifndef LIBWIN32_KWDS_LPFILENAME_DWFILEATTRIBUTES_DEFINED
#define LIBWIN32_KWDS_LPFILENAME_DWFILEATTRIBUTES_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_lpFileName_dwFileAttributes, { K(lpFileName), K(dwFileAttributes), KEND });
#endif /* !LIBWIN32_KWDS_LPFILENAME_DWFILEATTRIBUTES_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_SetFileAttributesW_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	LPCWSTR lpFileName_str;
	DeeStringObject *lpFileName;
	DWORD dwFileAttributes;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_lpFileName_dwFileAttributes, "o" UNPu32 ":SetFileAttributesW", &lpFileName, &dwFileAttributes))
		goto err;
	if (DeeObject_AssertTypeExact(lpFileName, &DeeString_Type))
		goto err;
	lpFileName_str = (LPCWSTR)DeeString_AsWide((DeeObject *)lpFileName);
	if unlikely(!lpFileName_str)
		goto err;
	return libwin32_SetFileAttributesW_f_impl(lpFileName_str, dwFileAttributes);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_SetFileAttributesW_f_impl(LPCWSTR lpFileName, DWORD dwFileAttributes)
/*[[[end]]]*/
{
	BOOL bResult;
again:
	DBG_ALIGNMENT_DISABLE();
	bResult = SetFileAttributesW(lpFileName, dwFileAttributes);
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
		                      "Failed to set file attribute of %lq to %#" PRFx32 "",
		                      lpFileName, dwFileAttributes);
	}
	DBG_ALIGNMENT_ENABLE();
	RETURN_SUCCESS_OR_TRUE;
err:
	return NULL;
}


/*[[[deemon import("rt.gen.dexutils").gw("SetFilePointer", "hFile:nt:HANDLE,lDistanceToMove:I64d,dwMoveMethod:nt:DWORD=FILE_BEGIN->?Dint"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_SetFilePointer_f_impl(HANDLE hFile, int64_t lDistanceToMove, DWORD dwMoveMethod);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_SetFilePointer_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_SETFILEPOINTER_DEF { "SetFilePointer", (DeeObject *)&libwin32_SetFilePointer, MODSYM_FNORMAL, DOC("(hFile:?X3?Dint?DFile?Ewin32:HANDLE,lDistanceToMove:?Dint,dwMoveMethod:?Dint=!GFILE_BEGIN)->?Dint") },
#define LIBWIN32_SETFILEPOINTER_DEF_DOC(doc) { "SetFilePointer", (DeeObject *)&libwin32_SetFilePointer, MODSYM_FNORMAL, DOC("(hFile:?X3?Dint?DFile?Ewin32:HANDLE,lDistanceToMove:?Dint,dwMoveMethod:?Dint=!GFILE_BEGIN)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_SetFilePointer, &libwin32_SetFilePointer_f);
#ifndef LIBWIN32_KWDS_HFILE_LDISTANCETOMOVE_DWMOVEMETHOD_DEFINED
#define LIBWIN32_KWDS_HFILE_LDISTANCETOMOVE_DWMOVEMETHOD_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_hFile_lDistanceToMove_dwMoveMethod, { K(hFile), K(lDistanceToMove), K(dwMoveMethod), KEND });
#endif /* !LIBWIN32_KWDS_HFILE_LDISTANCETOMOVE_DWMOVEMETHOD_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_SetFilePointer_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	HANDLE hhFile;
	DeeObject *hFile;
	int64_t lDistanceToMove;
	DWORD dwMoveMethod = FILE_BEGIN;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_hFile_lDistanceToMove_dwMoveMethod, "o" UNPd64 "|" UNPu32 ":SetFilePointer", &hFile, &lDistanceToMove, &dwMoveMethod))
		goto err;
	if (DeeNTSystem_TryGetHandle(hFile, (void **)&hhFile))
		goto err;
	return libwin32_SetFilePointer_f_impl(hhFile, lDistanceToMove, dwMoveMethod);
err:
	return NULL;
}
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
	return DeeInt_NewU64((uint64_t)(uint32_t)lDistanceToMoveHigh << 32 |
	                     (uint64_t)(uint32_t)dwResult);
err:
	return NULL;
}


typedef union {
	uint64_t u64;
	FILETIME ft;
} ALIGNED_FILETIME;


/*[[[deemon import("rt.gen.dexutils").gw("GetFileTime", "hFile:nt:HANDLE->" MAYBE_NONE("?T3?Dint?Dint?Dint")); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetFileTime_f_impl(HANDLE hFile);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetFileTime_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_GETFILETIME_DEF { "GetFileTime", (DeeObject *)&libwin32_GetFileTime, MODSYM_FNORMAL, DOC("(hFile:?X3?Dint?DFile?Ewin32:HANDLE)->?T3?Dint?Dint?Dint") },
#define LIBWIN32_GETFILETIME_DEF_DOC(doc) { "GetFileTime", (DeeObject *)&libwin32_GetFileTime, MODSYM_FNORMAL, DOC("(hFile:?X3?Dint?DFile?Ewin32:HANDLE)->?T3?Dint?Dint?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_GetFileTime, &libwin32_GetFileTime_f);
#ifndef LIBWIN32_KWDS_HFILE_DEFINED
#define LIBWIN32_KWDS_HFILE_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_hFile, { K(hFile), KEND });
#endif /* !LIBWIN32_KWDS_HFILE_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetFileTime_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	HANDLE hhFile;
	DeeObject *hFile;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_hFile, "o:GetFileTime", &hFile))
		goto err;
	if (DeeNTSystem_TryGetHandle(hFile, (void **)&hhFile))
		goto err;
	return libwin32_GetFileTime_f_impl(hhFile);
err:
	return NULL;
}
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
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_SetFileTime_f_impl(HANDLE hFile, DeeObject *lpCreationTime, DeeObject *lpLastAccessTime, DeeObject *lpLastWriteTime);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_SetFileTime_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_SETFILETIME_DEF { "SetFileTime", (DeeObject *)&libwin32_SetFileTime, MODSYM_FNORMAL, DOC("(hFile:?X3?Dint?DFile?Ewin32:HANDLE,lpCreationTime?:?Dint,lpLastAccessTime?:?Dint,lpLastWriteTime?:?Dint)") },
#define LIBWIN32_SETFILETIME_DEF_DOC(doc) { "SetFileTime", (DeeObject *)&libwin32_SetFileTime, MODSYM_FNORMAL, DOC("(hFile:?X3?Dint?DFile?Ewin32:HANDLE,lpCreationTime?:?Dint,lpLastAccessTime?:?Dint,lpLastWriteTime?:?Dint)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_SetFileTime, &libwin32_SetFileTime_f);
#ifndef LIBWIN32_KWDS_HFILE_LPCREATIONTIME_LPLASTACCESSTIME_LPLASTWRITETIME_DEFINED
#define LIBWIN32_KWDS_HFILE_LPCREATIONTIME_LPLASTACCESSTIME_LPLASTWRITETIME_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_hFile_lpCreationTime_lpLastAccessTime_lpLastWriteTime, { K(hFile), K(lpCreationTime), K(lpLastAccessTime), K(lpLastWriteTime), KEND });
#endif /* !LIBWIN32_KWDS_HFILE_LPCREATIONTIME_LPLASTACCESSTIME_LPLASTWRITETIME_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_SetFileTime_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	HANDLE hhFile;
	DeeObject *hFile;
	DeeObject *lpCreationTime = NULL;
	DeeObject *lpLastAccessTime = NULL;
	DeeObject *lpLastWriteTime = NULL;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_hFile_lpCreationTime_lpLastAccessTime_lpLastWriteTime, "o|ooo:SetFileTime", &hFile, &lpCreationTime, &lpLastAccessTime, &lpLastWriteTime))
		goto err;
	if (DeeNTSystem_TryGetHandle(hFile, (void **)&hhFile))
		goto err;
	return libwin32_SetFileTime_f_impl(hhFile, lpCreationTime, lpLastAccessTime, lpLastWriteTime);
err:
	return NULL;
}
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
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_SetFileValidData_f_impl(HANDLE hFile, uint64_t ValidDataLength);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_SetFileValidData_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_SETFILEVALIDDATA_DEF { "SetFileValidData", (DeeObject *)&libwin32_SetFileValidData, MODSYM_FNORMAL, DOC("(hFile:?X3?Dint?DFile?Ewin32:HANDLE,ValidDataLength:?Dint)") },
#define LIBWIN32_SETFILEVALIDDATA_DEF_DOC(doc) { "SetFileValidData", (DeeObject *)&libwin32_SetFileValidData, MODSYM_FNORMAL, DOC("(hFile:?X3?Dint?DFile?Ewin32:HANDLE,ValidDataLength:?Dint)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_SetFileValidData, &libwin32_SetFileValidData_f);
#ifndef LIBWIN32_KWDS_HFILE_VALIDDATALENGTH_DEFINED
#define LIBWIN32_KWDS_HFILE_VALIDDATALENGTH_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_hFile_ValidDataLength, { K(hFile), K(ValidDataLength), KEND });
#endif /* !LIBWIN32_KWDS_HFILE_VALIDDATALENGTH_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_SetFileValidData_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	HANDLE hhFile;
	DeeObject *hFile;
	uint64_t ValidDataLength;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_hFile_ValidDataLength, "o" UNPu64 ":SetFileValidData", &hFile, &ValidDataLength))
		goto err;
	if (DeeNTSystem_TryGetHandle(hFile, (void **)&hhFile))
		goto err;
	return libwin32_SetFileValidData_f_impl(hhFile, ValidDataLength);
err:
	return NULL;
}
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
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetTempPath_f_impl(void);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetTempPath_f(size_t argc, DeeObject *const *argv);
#define LIBWIN32_GETTEMPPATH_DEF { "GetTempPath", (DeeObject *)&libwin32_GetTempPath, MODSYM_FNORMAL, DOC("->?Dstring") },
#define LIBWIN32_GETTEMPPATH_DEF_DOC(doc) { "GetTempPath", (DeeObject *)&libwin32_GetTempPath, MODSYM_FNORMAL, DOC("->?Dstring\n" doc) },
PRIVATE DEFINE_CMETHOD(libwin32_GetTempPath, libwin32_GetTempPath_f);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetTempPath_f(size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":GetTempPath"))
		goto err;
	return libwin32_GetTempPath_f_impl();
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetTempPath_f_impl(void)
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
			if (dwError != NO_ERROR)
				RETURN_ERROR(dwError, "Failed to determine TEMPPATH");
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
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetDllDirectory_f_impl(void);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetDllDirectory_f(size_t argc, DeeObject *const *argv);
#define LIBWIN32_GETDLLDIRECTORY_DEF { "GetDllDirectory", (DeeObject *)&libwin32_GetDllDirectory, MODSYM_FNORMAL, DOC("->?Dstring") },
#define LIBWIN32_GETDLLDIRECTORY_DEF_DOC(doc) { "GetDllDirectory", (DeeObject *)&libwin32_GetDllDirectory, MODSYM_FNORMAL, DOC("->?Dstring\n" doc) },
PRIVATE DEFINE_CMETHOD(libwin32_GetDllDirectory, libwin32_GetDllDirectory_f);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetDllDirectory_f(size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":GetDllDirectory"))
		goto err;
	return libwin32_GetDllDirectory_f_impl();
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetDllDirectory_f_impl(void)
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
			if (dwError != NO_ERROR)
				RETURN_ERROR(dwError, "Failed to determine the DLL Directory");
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
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_SetDllDirectory_f_impl(LPCWSTR lpPathName);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_SetDllDirectory_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_SETDLLDIRECTORY_DEF { "SetDllDirectory", (DeeObject *)&libwin32_SetDllDirectory, MODSYM_FNORMAL, DOC("(lpPathName:?Dstring)") },
#define LIBWIN32_SETDLLDIRECTORY_DEF_DOC(doc) { "SetDllDirectory", (DeeObject *)&libwin32_SetDllDirectory, MODSYM_FNORMAL, DOC("(lpPathName:?Dstring)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_SetDllDirectory, &libwin32_SetDllDirectory_f);
#ifndef LIBWIN32_KWDS_LPPATHNAME_DEFINED
#define LIBWIN32_KWDS_LPPATHNAME_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_lpPathName, { K(lpPathName), KEND });
#endif /* !LIBWIN32_KWDS_LPPATHNAME_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_SetDllDirectory_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	LPCWSTR lpPathName_str;
	DeeStringObject *lpPathName;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_lpPathName, "o:SetDllDirectory", &lpPathName))
		goto err;
	if (DeeObject_AssertTypeExact(lpPathName, &DeeString_Type))
		goto err;
	lpPathName_str = (LPCWSTR)DeeString_AsWide((DeeObject *)lpPathName);
	if unlikely(!lpPathName_str)
		goto err;
	return libwin32_SetDllDirectory_f_impl(lpPathName_str);
err:
	return NULL;
}
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
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetDiskFreeSpace_f_impl(LPCWSTR lpRootPathName);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetDiskFreeSpace_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_GETDISKFREESPACE_DEF { "GetDiskFreeSpace", (DeeObject *)&libwin32_GetDiskFreeSpace, MODSYM_FNORMAL, DOC("(lpRootPathName:?Dstring)->?T4?Dint?Dint?Dint?Dint") },
#define LIBWIN32_GETDISKFREESPACE_DEF_DOC(doc) { "GetDiskFreeSpace", (DeeObject *)&libwin32_GetDiskFreeSpace, MODSYM_FNORMAL, DOC("(lpRootPathName:?Dstring)->?T4?Dint?Dint?Dint?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_GetDiskFreeSpace, &libwin32_GetDiskFreeSpace_f);
#ifndef LIBWIN32_KWDS_LPROOTPATHNAME_DEFINED
#define LIBWIN32_KWDS_LPROOTPATHNAME_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_lpRootPathName, { K(lpRootPathName), KEND });
#endif /* !LIBWIN32_KWDS_LPROOTPATHNAME_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetDiskFreeSpace_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	LPCWSTR lpRootPathName_str;
	DeeStringObject *lpRootPathName;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_lpRootPathName, "o:GetDiskFreeSpace", &lpRootPathName))
		goto err;
	if (DeeObject_AssertTypeExact(lpRootPathName, &DeeString_Type))
		goto err;
	lpRootPathName_str = (LPCWSTR)DeeString_AsWide((DeeObject *)lpRootPathName);
	if unlikely(!lpRootPathName_str)
		goto err;
	return libwin32_GetDiskFreeSpace_f_impl(lpRootPathName_str);
err:
	return NULL;
}
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
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetDiskFreeSpaceEx_f_impl(LPCWSTR lpDirectoryName);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetDiskFreeSpaceEx_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_GETDISKFREESPACEEX_DEF { "GetDiskFreeSpaceEx", (DeeObject *)&libwin32_GetDiskFreeSpaceEx, MODSYM_FNORMAL, DOC("(lpDirectoryName:?Dstring)->?T3?Dint?Dint?Dint") },
#define LIBWIN32_GETDISKFREESPACEEX_DEF_DOC(doc) { "GetDiskFreeSpaceEx", (DeeObject *)&libwin32_GetDiskFreeSpaceEx, MODSYM_FNORMAL, DOC("(lpDirectoryName:?Dstring)->?T3?Dint?Dint?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_GetDiskFreeSpaceEx, &libwin32_GetDiskFreeSpaceEx_f);
#ifndef LIBWIN32_KWDS_LPDIRECTORYNAME_DEFINED
#define LIBWIN32_KWDS_LPDIRECTORYNAME_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_lpDirectoryName, { K(lpDirectoryName), KEND });
#endif /* !LIBWIN32_KWDS_LPDIRECTORYNAME_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetDiskFreeSpaceEx_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	LPCWSTR lpDirectoryName_str;
	DeeStringObject *lpDirectoryName;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_lpDirectoryName, "o:GetDiskFreeSpaceEx", &lpDirectoryName))
		goto err;
	if (DeeObject_AssertTypeExact(lpDirectoryName, &DeeString_Type))
		goto err;
	lpDirectoryName_str = (LPCWSTR)DeeString_AsWide((DeeObject *)lpDirectoryName);
	if unlikely(!lpDirectoryName_str)
		goto err;
	return libwin32_GetDiskFreeSpaceEx_f_impl(lpDirectoryName_str);
err:
	return NULL;
}
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
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetDriveType_f_impl(LPCWSTR lpRootPathName);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetDriveType_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_GETDRIVETYPE_DEF { "GetDriveType", (DeeObject *)&libwin32_GetDriveType, MODSYM_FNORMAL, DOC("(lpRootPathName:?Dstring)->?Dint") },
#define LIBWIN32_GETDRIVETYPE_DEF_DOC(doc) { "GetDriveType", (DeeObject *)&libwin32_GetDriveType, MODSYM_FNORMAL, DOC("(lpRootPathName:?Dstring)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_GetDriveType, &libwin32_GetDriveType_f);
#ifndef LIBWIN32_KWDS_LPROOTPATHNAME_DEFINED
#define LIBWIN32_KWDS_LPROOTPATHNAME_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_lpRootPathName, { K(lpRootPathName), KEND });
#endif /* !LIBWIN32_KWDS_LPROOTPATHNAME_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetDriveType_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	LPCWSTR lpRootPathName_str;
	DeeStringObject *lpRootPathName;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_lpRootPathName, "o:GetDriveType", &lpRootPathName))
		goto err;
	if (DeeObject_AssertTypeExact(lpRootPathName, &DeeString_Type))
		goto err;
	lpRootPathName_str = (LPCWSTR)DeeString_AsWide((DeeObject *)lpRootPathName);
	if unlikely(!lpRootPathName_str)
		goto err;
	return libwin32_GetDriveType_f_impl(lpRootPathName_str);
err:
	return NULL;
}
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
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetModuleFileName_f_impl(HANDLE hModule);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetModuleFileName_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_GETMODULEFILENAME_DEF { "GetModuleFileName", (DeeObject *)&libwin32_GetModuleFileName, MODSYM_FNORMAL, DOC("(hModule:?X3?Dint?DFile?Ewin32:HANDLE)->?Dstring") },
#define LIBWIN32_GETMODULEFILENAME_DEF_DOC(doc) { "GetModuleFileName", (DeeObject *)&libwin32_GetModuleFileName, MODSYM_FNORMAL, DOC("(hModule:?X3?Dint?DFile?Ewin32:HANDLE)->?Dstring\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_GetModuleFileName, &libwin32_GetModuleFileName_f);
#ifndef LIBWIN32_KWDS_HMODULE_DEFINED
#define LIBWIN32_KWDS_HMODULE_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_hModule, { K(hModule), KEND });
#endif /* !LIBWIN32_KWDS_HMODULE_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetModuleFileName_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	HANDLE hhModule;
	DeeObject *hModule;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_hModule, "o:GetModuleFileName", &hModule))
		goto err;
	if (DeeNTSystem_TryGetHandle(hModule, (void **)&hhModule))
		goto err;
	return libwin32_GetModuleFileName_f_impl(hhModule);
err:
	return NULL;
}
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
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetSystemDirectory_f_impl(void);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetSystemDirectory_f(size_t argc, DeeObject *const *argv);
#define LIBWIN32_GETSYSTEMDIRECTORY_DEF { "GetSystemDirectory", (DeeObject *)&libwin32_GetSystemDirectory, MODSYM_FNORMAL, DOC("->?Dstring") },
#define LIBWIN32_GETSYSTEMDIRECTORY_DEF_DOC(doc) { "GetSystemDirectory", (DeeObject *)&libwin32_GetSystemDirectory, MODSYM_FNORMAL, DOC("->?Dstring\n" doc) },
PRIVATE DEFINE_CMETHOD(libwin32_GetSystemDirectory, libwin32_GetSystemDirectory_f);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetSystemDirectory_f(size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":GetSystemDirectory"))
		goto err;
	return libwin32_GetSystemDirectory_f_impl();
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetSystemDirectory_f_impl(void)
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
			if (dwError != NO_ERROR)
				RETURN_ERROR(dwError, "Failed to determine the system directory");
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
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetWindowsDirectory_f_impl(void);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetWindowsDirectory_f(size_t argc, DeeObject *const *argv);
#define LIBWIN32_GETWINDOWSDIRECTORY_DEF { "GetWindowsDirectory", (DeeObject *)&libwin32_GetWindowsDirectory, MODSYM_FNORMAL, DOC("->?Dstring") },
#define LIBWIN32_GETWINDOWSDIRECTORY_DEF_DOC(doc) { "GetWindowsDirectory", (DeeObject *)&libwin32_GetWindowsDirectory, MODSYM_FNORMAL, DOC("->?Dstring\n" doc) },
PRIVATE DEFINE_CMETHOD(libwin32_GetWindowsDirectory, libwin32_GetWindowsDirectory_f);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetWindowsDirectory_f(size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":GetWindowsDirectory"))
		goto err;
	return libwin32_GetWindowsDirectory_f_impl();
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetWindowsDirectory_f_impl(void)
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
			if (dwError != NO_ERROR)
				RETURN_ERROR(dwError, "Failed to determine the Windows directory");
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
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetSystemWindowsDirectory_f_impl(void);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetSystemWindowsDirectory_f(size_t argc, DeeObject *const *argv);
#define LIBWIN32_GETSYSTEMWINDOWSDIRECTORY_DEF { "GetSystemWindowsDirectory", (DeeObject *)&libwin32_GetSystemWindowsDirectory, MODSYM_FNORMAL, DOC("->?Dstring") },
#define LIBWIN32_GETSYSTEMWINDOWSDIRECTORY_DEF_DOC(doc) { "GetSystemWindowsDirectory", (DeeObject *)&libwin32_GetSystemWindowsDirectory, MODSYM_FNORMAL, DOC("->?Dstring\n" doc) },
PRIVATE DEFINE_CMETHOD(libwin32_GetSystemWindowsDirectory, libwin32_GetSystemWindowsDirectory_f);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetSystemWindowsDirectory_f(size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":GetSystemWindowsDirectory"))
		goto err;
	return libwin32_GetSystemWindowsDirectory_f_impl();
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetSystemWindowsDirectory_f_impl(void)
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
			if (dwError != NO_ERROR)
				RETURN_ERROR(dwError, "Failed to determine the System-Windows directory");
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
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetSystemWow64Directory_f_impl(void);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetSystemWow64Directory_f(size_t argc, DeeObject *const *argv);
#define LIBWIN32_GETSYSTEMWOW64DIRECTORY_DEF { "GetSystemWow64Directory", (DeeObject *)&libwin32_GetSystemWow64Directory, MODSYM_FNORMAL, DOC("->?Dstring") },
#define LIBWIN32_GETSYSTEMWOW64DIRECTORY_DEF_DOC(doc) { "GetSystemWow64Directory", (DeeObject *)&libwin32_GetSystemWow64Directory, MODSYM_FNORMAL, DOC("->?Dstring\n" doc) },
PRIVATE DEFINE_CMETHOD(libwin32_GetSystemWow64Directory, libwin32_GetSystemWow64Directory_f);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetSystemWow64Directory_f(size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":GetSystemWow64Directory"))
		goto err;
	return libwin32_GetSystemWow64Directory_f_impl();
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetSystemWow64Directory_f_impl(void)
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
			if (dwError != NO_ERROR)
				RETURN_ERROR(dwError, "Failed to determine the SystemWow64 directory");
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
PRIVATE DREF DeeObject *DCALL
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
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetLogicalDriveStrings_f_impl(void);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetLogicalDriveStrings_f(size_t argc, DeeObject *const *argv);
#define LIBWIN32_GETLOGICALDRIVESTRINGS_DEF { "GetLogicalDriveStrings", (DeeObject *)&libwin32_GetLogicalDriveStrings, MODSYM_FNORMAL, DOC("->?S?Dstring") },
#define LIBWIN32_GETLOGICALDRIVESTRINGS_DEF_DOC(doc) { "GetLogicalDriveStrings", (DeeObject *)&libwin32_GetLogicalDriveStrings, MODSYM_FNORMAL, DOC("->?S?Dstring\n" doc) },
PRIVATE DEFINE_CMETHOD(libwin32_GetLogicalDriveStrings, libwin32_GetLogicalDriveStrings_f);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetLogicalDriveStrings_f(size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":GetLogicalDriveStrings"))
		goto err;
	return libwin32_GetLogicalDriveStrings_f_impl();
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetLogicalDriveStrings_f_impl(void)
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
			if (dwError != NO_ERROR)
				RETURN_ERROR(dwError, "Failed to query logical drive strings");
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
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_QueryDosDevice_f_impl(LPCWSTR lpDeviceName);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_QueryDosDevice_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_QUERYDOSDEVICE_DEF { "QueryDosDevice", (DeeObject *)&libwin32_QueryDosDevice, MODSYM_FNORMAL, DOC("(lpDeviceName:?Dstring)->?S?Dstring") },
#define LIBWIN32_QUERYDOSDEVICE_DEF_DOC(doc) { "QueryDosDevice", (DeeObject *)&libwin32_QueryDosDevice, MODSYM_FNORMAL, DOC("(lpDeviceName:?Dstring)->?S?Dstring\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_QueryDosDevice, &libwin32_QueryDosDevice_f);
#ifndef LIBWIN32_KWDS_LPDEVICENAME_DEFINED
#define LIBWIN32_KWDS_LPDEVICENAME_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_lpDeviceName, { K(lpDeviceName), KEND });
#endif /* !LIBWIN32_KWDS_LPDEVICENAME_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_QueryDosDevice_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	LPCWSTR lpDeviceName_str;
	DeeStringObject *lpDeviceName;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_lpDeviceName, "o:QueryDosDevice", &lpDeviceName))
		goto err;
	if (DeeObject_AssertTypeExact(lpDeviceName, &DeeString_Type))
		goto err;
	lpDeviceName_str = (LPCWSTR)DeeString_AsWide((DeeObject *)lpDeviceName);
	if unlikely(!lpDeviceName_str)
		goto err;
	return libwin32_QueryDosDevice_f_impl(lpDeviceName_str);
err:
	return NULL;
}
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
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetFileType_f_impl(HANDLE hFile);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetFileType_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_GETFILETYPE_DEF { "GetFileType", (DeeObject *)&libwin32_GetFileType, MODSYM_FNORMAL, DOC("(hFile:?X3?Dint?DFile?Ewin32:HANDLE)->?Dint") },
#define LIBWIN32_GETFILETYPE_DEF_DOC(doc) { "GetFileType", (DeeObject *)&libwin32_GetFileType, MODSYM_FNORMAL, DOC("(hFile:?X3?Dint?DFile?Ewin32:HANDLE)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_GetFileType, &libwin32_GetFileType_f);
#ifndef LIBWIN32_KWDS_HFILE_DEFINED
#define LIBWIN32_KWDS_HFILE_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_hFile, { K(hFile), KEND });
#endif /* !LIBWIN32_KWDS_HFILE_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetFileType_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	HANDLE hhFile;
	DeeObject *hFile;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_hFile, "o:GetFileType", &hFile))
		goto err;
	if (DeeNTSystem_TryGetHandle(hFile, (void **)&hhFile))
		goto err;
	return libwin32_GetFileType_f_impl(hhFile);
err:
	return NULL;
}
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
	return DeeInt_NewU32((uint32_t)dwType);
err:
	return NULL;
}




/*[[[deemon import("rt.gen.dexutils").gw("GetFileSize", "hFile:nt:HANDLE->" MAYBE_NONE("?Dint")); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetFileSize_f_impl(HANDLE hFile);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetFileSize_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_GETFILESIZE_DEF { "GetFileSize", (DeeObject *)&libwin32_GetFileSize, MODSYM_FNORMAL, DOC("(hFile:?X3?Dint?DFile?Ewin32:HANDLE)->?Dint") },
#define LIBWIN32_GETFILESIZE_DEF_DOC(doc) { "GetFileSize", (DeeObject *)&libwin32_GetFileSize, MODSYM_FNORMAL, DOC("(hFile:?X3?Dint?DFile?Ewin32:HANDLE)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_GetFileSize, &libwin32_GetFileSize_f);
#ifndef LIBWIN32_KWDS_HFILE_DEFINED
#define LIBWIN32_KWDS_HFILE_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_hFile, { K(hFile), KEND });
#endif /* !LIBWIN32_KWDS_HFILE_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetFileSize_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	HANDLE hhFile;
	DeeObject *hFile;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_hFile, "o:GetFileSize", &hFile))
		goto err;
	if (DeeNTSystem_TryGetHandle(hFile, (void **)&hhFile))
		goto err;
	return libwin32_GetFileSize_f_impl(hhFile);
err:
	return NULL;
}
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
	return DeeInt_NewU64((uint64_t)dwSizeLow |
	                     (uint64_t)dwSizeHigh << 32);
err:
	return NULL;
}



/*[[[deemon import("rt.gen.dexutils").gw("GetFileAttributes", "lpFileName:nt:LPCWSTR->" MAYBE_NONE("?Dint")); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetFileAttributes_f_impl(LPCWSTR lpFileName);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetFileAttributes_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_GETFILEATTRIBUTES_DEF { "GetFileAttributes", (DeeObject *)&libwin32_GetFileAttributes, MODSYM_FNORMAL, DOC("(lpFileName:?Dstring)->?Dint") },
#define LIBWIN32_GETFILEATTRIBUTES_DEF_DOC(doc) { "GetFileAttributes", (DeeObject *)&libwin32_GetFileAttributes, MODSYM_FNORMAL, DOC("(lpFileName:?Dstring)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_GetFileAttributes, &libwin32_GetFileAttributes_f);
#ifndef LIBWIN32_KWDS_LPFILENAME_DEFINED
#define LIBWIN32_KWDS_LPFILENAME_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_lpFileName, { K(lpFileName), KEND });
#endif /* !LIBWIN32_KWDS_LPFILENAME_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetFileAttributes_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	LPCWSTR lpFileName_str;
	DeeStringObject *lpFileName;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_lpFileName, "o:GetFileAttributes", &lpFileName))
		goto err;
	if (DeeObject_AssertTypeExact(lpFileName, &DeeString_Type))
		goto err;
	lpFileName_str = (LPCWSTR)DeeString_AsWide((DeeObject *)lpFileName);
	if unlikely(!lpFileName_str)
		goto err;
	return libwin32_GetFileAttributes_f_impl(lpFileName_str);
err:
	return NULL;
}
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
	return DeeInt_NewU32((uint32_t)dwResult);
err:
	return NULL;
}



/*[[[deemon import("rt.gen.dexutils").gw("SetFileAttributes", "lpFileName:nt:LPCWSTR,dwFileAttributes:nt:DWORD" ERROR_OR_BOOL); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_SetFileAttributes_f_impl(LPCWSTR lpFileName, DWORD dwFileAttributes);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_SetFileAttributes_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_SETFILEATTRIBUTES_DEF { "SetFileAttributes", (DeeObject *)&libwin32_SetFileAttributes, MODSYM_FNORMAL, DOC("(lpFileName:?Dstring,dwFileAttributes:?Dint)") },
#define LIBWIN32_SETFILEATTRIBUTES_DEF_DOC(doc) { "SetFileAttributes", (DeeObject *)&libwin32_SetFileAttributes, MODSYM_FNORMAL, DOC("(lpFileName:?Dstring,dwFileAttributes:?Dint)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_SetFileAttributes, &libwin32_SetFileAttributes_f);
#ifndef LIBWIN32_KWDS_LPFILENAME_DWFILEATTRIBUTES_DEFINED
#define LIBWIN32_KWDS_LPFILENAME_DWFILEATTRIBUTES_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_lpFileName_dwFileAttributes, { K(lpFileName), K(dwFileAttributes), KEND });
#endif /* !LIBWIN32_KWDS_LPFILENAME_DWFILEATTRIBUTES_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_SetFileAttributes_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	LPCWSTR lpFileName_str;
	DeeStringObject *lpFileName;
	DWORD dwFileAttributes;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_lpFileName_dwFileAttributes, "o" UNPu32 ":SetFileAttributes", &lpFileName, &dwFileAttributes))
		goto err;
	if (DeeObject_AssertTypeExact(lpFileName, &DeeString_Type))
		goto err;
	lpFileName_str = (LPCWSTR)DeeString_AsWide((DeeObject *)lpFileName);
	if unlikely(!lpFileName_str)
		goto err;
	return libwin32_SetFileAttributes_f_impl(lpFileName_str, dwFileAttributes);
err:
	return NULL;
}
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
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetCompressedFileSize_f_impl(LPCWSTR lpFileName);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetCompressedFileSize_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_GETCOMPRESSEDFILESIZE_DEF { "GetCompressedFileSize", (DeeObject *)&libwin32_GetCompressedFileSize, MODSYM_FNORMAL, DOC("(lpFileName:?Dstring)->?Dint") },
#define LIBWIN32_GETCOMPRESSEDFILESIZE_DEF_DOC(doc) { "GetCompressedFileSize", (DeeObject *)&libwin32_GetCompressedFileSize, MODSYM_FNORMAL, DOC("(lpFileName:?Dstring)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_GetCompressedFileSize, &libwin32_GetCompressedFileSize_f);
#ifndef LIBWIN32_KWDS_LPFILENAME_DEFINED
#define LIBWIN32_KWDS_LPFILENAME_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_lpFileName, { K(lpFileName), KEND });
#endif /* !LIBWIN32_KWDS_LPFILENAME_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetCompressedFileSize_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	LPCWSTR lpFileName_str;
	DeeStringObject *lpFileName;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_lpFileName, "o:GetCompressedFileSize", &lpFileName))
		goto err;
	if (DeeObject_AssertTypeExact(lpFileName, &DeeString_Type))
		goto err;
	lpFileName_str = (LPCWSTR)DeeString_AsWide((DeeObject *)lpFileName);
	if unlikely(!lpFileName_str)
		goto err;
	return libwin32_GetCompressedFileSize_f_impl(lpFileName_str);
err:
	return NULL;
}
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
	return DeeInt_NewU64((uint64_t)dwSizeLow |
	                     (uint64_t)dwSizeHigh << 32);
err:
	return NULL;
}



/*[[[deemon import("rt.gen.dexutils").gw("FlushFileBuffers", "hFile:nt:HANDLE" ERROR_OR_BOOL); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_FlushFileBuffers_f_impl(HANDLE hFile);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_FlushFileBuffers_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_FLUSHFILEBUFFERS_DEF { "FlushFileBuffers", (DeeObject *)&libwin32_FlushFileBuffers, MODSYM_FNORMAL, DOC("(hFile:?X3?Dint?DFile?Ewin32:HANDLE)") },
#define LIBWIN32_FLUSHFILEBUFFERS_DEF_DOC(doc) { "FlushFileBuffers", (DeeObject *)&libwin32_FlushFileBuffers, MODSYM_FNORMAL, DOC("(hFile:?X3?Dint?DFile?Ewin32:HANDLE)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_FlushFileBuffers, &libwin32_FlushFileBuffers_f);
#ifndef LIBWIN32_KWDS_HFILE_DEFINED
#define LIBWIN32_KWDS_HFILE_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_hFile, { K(hFile), KEND });
#endif /* !LIBWIN32_KWDS_HFILE_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_FlushFileBuffers_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	HANDLE hhFile;
	DeeObject *hFile;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_hFile, "o:FlushFileBuffers", &hFile))
		goto err;
	if (DeeNTSystem_TryGetHandle(hFile, (void **)&hhFile))
		goto err;
	return libwin32_FlushFileBuffers_f_impl(hhFile);
err:
	return NULL;
}
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
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetFinalPathNameByHandle_f_impl(HANDLE hFile, DWORD dwFlags);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetFinalPathNameByHandle_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_GETFINALPATHNAMEBYHANDLE_DEF { "GetFinalPathNameByHandle", (DeeObject *)&libwin32_GetFinalPathNameByHandle, MODSYM_FNORMAL, DOC("(hFile:?X3?Dint?DFile?Ewin32:HANDLE,dwFlags:?Dint=!0)->?Dstring") },
#define LIBWIN32_GETFINALPATHNAMEBYHANDLE_DEF_DOC(doc) { "GetFinalPathNameByHandle", (DeeObject *)&libwin32_GetFinalPathNameByHandle, MODSYM_FNORMAL, DOC("(hFile:?X3?Dint?DFile?Ewin32:HANDLE,dwFlags:?Dint=!0)->?Dstring\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_GetFinalPathNameByHandle, &libwin32_GetFinalPathNameByHandle_f);
#ifndef LIBWIN32_KWDS_HFILE_DWFLAGS_DEFINED
#define LIBWIN32_KWDS_HFILE_DWFLAGS_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_hFile_dwFlags, { K(hFile), K(dwFlags), KEND });
#endif /* !LIBWIN32_KWDS_HFILE_DWFLAGS_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetFinalPathNameByHandle_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	HANDLE hhFile;
	DeeObject *hFile;
	DWORD dwFlags = 0;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_hFile_dwFlags, "o|" UNPu32 ":GetFinalPathNameByHandle", &hFile, &dwFlags))
		goto err;
	if (DeeNTSystem_TryGetHandle(hFile, (void **)&hhFile))
		goto err;
	return libwin32_GetFinalPathNameByHandle_f_impl(hhFile, dwFlags);
err:
	return NULL;
}
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
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetFilenameOfHandle_f_impl(HANDLE hFile);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetFilenameOfHandle_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_GETFILENAMEOFHANDLE_DEF { "GetFilenameOfHandle", (DeeObject *)&libwin32_GetFilenameOfHandle, MODSYM_FNORMAL, DOC("(hFile:?X3?Dint?DFile?Ewin32:HANDLE)->?Dstring") },
#define LIBWIN32_GETFILENAMEOFHANDLE_DEF_DOC(doc) { "GetFilenameOfHandle", (DeeObject *)&libwin32_GetFilenameOfHandle, MODSYM_FNORMAL, DOC("(hFile:?X3?Dint?DFile?Ewin32:HANDLE)->?Dstring\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_GetFilenameOfHandle, &libwin32_GetFilenameOfHandle_f);
#ifndef LIBWIN32_KWDS_HFILE_DEFINED
#define LIBWIN32_KWDS_HFILE_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_hFile, { K(hFile), KEND });
#endif /* !LIBWIN32_KWDS_HFILE_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetFilenameOfHandle_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	HANDLE hhFile;
	DeeObject *hFile;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_hFile, "o:GetFilenameOfHandle", &hFile))
		goto err;
	if (DeeNTSystem_TryGetHandle(hFile, (void **)&hhFile))
		goto err;
	return libwin32_GetFilenameOfHandle_f_impl(hhFile);
err:
	return NULL;
}
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
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_FormatErrorMessage_f_impl(DWORD dwError);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_FormatErrorMessage_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_FORMATERRORMESSAGE_DEF { "FormatErrorMessage", (DeeObject *)&libwin32_FormatErrorMessage, MODSYM_FNORMAL, DOC("(dwError:?Dint)->?Dstring") },
#define LIBWIN32_FORMATERRORMESSAGE_DEF_DOC(doc) { "FormatErrorMessage", (DeeObject *)&libwin32_FormatErrorMessage, MODSYM_FNORMAL, DOC("(dwError:?Dint)->?Dstring\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_FormatErrorMessage, &libwin32_FormatErrorMessage_f);
#ifndef LIBWIN32_KWDS_DWERROR_DEFINED
#define LIBWIN32_KWDS_DWERROR_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_dwError, { K(dwError), KEND });
#endif /* !LIBWIN32_KWDS_DWERROR_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_FormatErrorMessage_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DWORD dwError;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_dwError, UNPu32 ":FormatErrorMessage", &dwError))
		goto err;
	return libwin32_FormatErrorMessage_f_impl(dwError);
err:
	return NULL;
}
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
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetMappedFileName_f_impl(HANDLE hProcess, void *lpv);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetMappedFileName_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_GETMAPPEDFILENAME_DEF { "GetMappedFileName", (DeeObject *)&libwin32_GetMappedFileName, MODSYM_FNORMAL, DOC("(hProcess:?X3?Dint?DFile?Ewin32:HANDLE,lpv:?Aptr?Ectypes:void)->?Dstring") },
#define LIBWIN32_GETMAPPEDFILENAME_DEF_DOC(doc) { "GetMappedFileName", (DeeObject *)&libwin32_GetMappedFileName, MODSYM_FNORMAL, DOC("(hProcess:?X3?Dint?DFile?Ewin32:HANDLE,lpv:?Aptr?Ectypes:void)->?Dstring\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_GetMappedFileName, &libwin32_GetMappedFileName_f);
#ifndef LIBWIN32_KWDS_HPROCESS_LPV_DEFINED
#define LIBWIN32_KWDS_HPROCESS_LPV_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_hProcess_lpv, { K(hProcess), K(lpv), KEND });
#endif /* !LIBWIN32_KWDS_HPROCESS_LPV_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetMappedFileName_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	HANDLE hhProcess;
	DeeObject *hProcess;
	void *lpv_ptr;
	DeeObject *lpv;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_hProcess_lpv, "oo:GetMappedFileName", &hProcess, &lpv))
		goto err;
	if (DeeNTSystem_TryGetHandle(hProcess, (void **)&hhProcess))
		goto err;
	if (DeeCTypes_GetPointer(lpv, &lpv_ptr))
		goto err;
	return libwin32_GetMappedFileName_f_impl(hhProcess, lpv_ptr);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetMappedFileName_f_impl(HANDLE hProcess, void *lpv)
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
    ",dwDesiredAccess:nt:DWORD=FILE_MAP_READ"
    ",dwFileOffset:I64u=0"
    ",dwNumberOfBytesToMap:Iu=0"
    "->" MAYBE_NONE("?Aptr?Ectypes:void")); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_MapViewOfFile_f_impl(HANDLE hFileMappingObject, DWORD dwDesiredAccess, uint64_t dwFileOffset, size_t dwNumberOfBytesToMap);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_MapViewOfFile_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_MAPVIEWOFFILE_DEF { "MapViewOfFile", (DeeObject *)&libwin32_MapViewOfFile, MODSYM_FNORMAL, DOC("(hFileMappingObject:?X3?Dint?DFile?Ewin32:HANDLE,dwDesiredAccess:?Dint=!GFILE_MAP_READ,dwFileOffset:?Dint=!0,dwNumberOfBytesToMap:?Dint=!0)->?Aptr?Ectypes:void") },
#define LIBWIN32_MAPVIEWOFFILE_DEF_DOC(doc) { "MapViewOfFile", (DeeObject *)&libwin32_MapViewOfFile, MODSYM_FNORMAL, DOC("(hFileMappingObject:?X3?Dint?DFile?Ewin32:HANDLE,dwDesiredAccess:?Dint=!GFILE_MAP_READ,dwFileOffset:?Dint=!0,dwNumberOfBytesToMap:?Dint=!0)->?Aptr?Ectypes:void\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_MapViewOfFile, &libwin32_MapViewOfFile_f);
#ifndef LIBWIN32_KWDS_HFILEMAPPINGOBJECT_DWDESIREDACCESS_DWFILEOFFSET_DWNUMBEROFBYTESTOMAP_DEFINED
#define LIBWIN32_KWDS_HFILEMAPPINGOBJECT_DWDESIREDACCESS_DWFILEOFFSET_DWNUMBEROFBYTESTOMAP_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_hFileMappingObject_dwDesiredAccess_dwFileOffset_dwNumberOfBytesToMap, { K(hFileMappingObject), K(dwDesiredAccess), K(dwFileOffset), K(dwNumberOfBytesToMap), KEND });
#endif /* !LIBWIN32_KWDS_HFILEMAPPINGOBJECT_DWDESIREDACCESS_DWFILEOFFSET_DWNUMBEROFBYTESTOMAP_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_MapViewOfFile_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	HANDLE hhFileMappingObject;
	DeeObject *hFileMappingObject;
	DWORD dwDesiredAccess = FILE_MAP_READ;
	uint64_t dwFileOffset = 0;
	size_t dwNumberOfBytesToMap = 0;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_hFileMappingObject_dwDesiredAccess_dwFileOffset_dwNumberOfBytesToMap, "o|" UNPu32 UNPu64 UNPuSIZ ":MapViewOfFile", &hFileMappingObject, &dwDesiredAccess, &dwFileOffset, &dwNumberOfBytesToMap))
		goto err;
	if (DeeNTSystem_TryGetHandle(hFileMappingObject, (void **)&hhFileMappingObject))
		goto err;
	return libwin32_MapViewOfFile_f_impl(hhFileMappingObject, dwDesiredAccess, dwFileOffset, dwNumberOfBytesToMap);
err:
	return NULL;
}
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
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_UnmapViewOfFile_f_impl(void *lpBaseAddress);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_UnmapViewOfFile_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_UNMAPVIEWOFFILE_DEF { "UnmapViewOfFile", (DeeObject *)&libwin32_UnmapViewOfFile, MODSYM_FNORMAL, DOC("(lpBaseAddress:?Aptr?Ectypes:void)") },
#define LIBWIN32_UNMAPVIEWOFFILE_DEF_DOC(doc) { "UnmapViewOfFile", (DeeObject *)&libwin32_UnmapViewOfFile, MODSYM_FNORMAL, DOC("(lpBaseAddress:?Aptr?Ectypes:void)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_UnmapViewOfFile, &libwin32_UnmapViewOfFile_f);
#ifndef LIBWIN32_KWDS_LPBASEADDRESS_DEFINED
#define LIBWIN32_KWDS_LPBASEADDRESS_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_lpBaseAddress, { K(lpBaseAddress), KEND });
#endif /* !LIBWIN32_KWDS_LPBASEADDRESS_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_UnmapViewOfFile_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	void *lpBaseAddress_ptr;
	DeeObject *lpBaseAddress;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_lpBaseAddress, "o:UnmapViewOfFile", &lpBaseAddress))
		goto err;
	if (DeeCTypes_GetPointer(lpBaseAddress, &lpBaseAddress_ptr))
		goto err;
	return libwin32_UnmapViewOfFile_f_impl(lpBaseAddress_ptr);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_UnmapViewOfFile_f_impl(void *lpBaseAddress)
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
    ",flProtect:nt:DWORD=PAGE_READONLY"
    ",dwMaximumSize:I64u=0"
    ",lpName:nt:LPCWSTR=NULL"
    "->" MAYBE_NONE("?GHANDLE")); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_CreateFileMapping_f_impl(HANDLE hFile, DeeObject *lpFileMappingAttributes, DWORD flProtect, uint64_t dwMaximumSize, LPCWSTR lpName);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_CreateFileMapping_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_CREATEFILEMAPPING_DEF { "CreateFileMapping", (DeeObject *)&libwin32_CreateFileMapping, MODSYM_FNORMAL, DOC("(hFile:?X3?Dint?DFile?Ewin32:HANDLE,lpFileMappingAttributes?:?GSECURITY_ATTRIBUTES,flProtect:?Dint=!GPAGE_READONLY,dwMaximumSize:?Dint=!0,lpName?:?Dstring)->?GHANDLE") },
#define LIBWIN32_CREATEFILEMAPPING_DEF_DOC(doc) { "CreateFileMapping", (DeeObject *)&libwin32_CreateFileMapping, MODSYM_FNORMAL, DOC("(hFile:?X3?Dint?DFile?Ewin32:HANDLE,lpFileMappingAttributes?:?GSECURITY_ATTRIBUTES,flProtect:?Dint=!GPAGE_READONLY,dwMaximumSize:?Dint=!0,lpName?:?Dstring)->?GHANDLE\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_CreateFileMapping, &libwin32_CreateFileMapping_f);
#ifndef LIBWIN32_KWDS_HFILE_LPFILEMAPPINGATTRIBUTES_FLPROTECT_DWMAXIMUMSIZE_LPNAME_DEFINED
#define LIBWIN32_KWDS_HFILE_LPFILEMAPPINGATTRIBUTES_FLPROTECT_DWMAXIMUMSIZE_LPNAME_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_hFile_lpFileMappingAttributes_flProtect_dwMaximumSize_lpName, { K(hFile), K(lpFileMappingAttributes), K(flProtect), K(dwMaximumSize), K(lpName), KEND });
#endif /* !LIBWIN32_KWDS_HFILE_LPFILEMAPPINGATTRIBUTES_FLPROTECT_DWMAXIMUMSIZE_LPNAME_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_CreateFileMapping_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	HANDLE hhFile;
	DeeObject *hFile;
	DeeObject *lpFileMappingAttributes = NULL;
	DWORD flProtect = PAGE_READONLY;
	uint64_t dwMaximumSize = 0;
	LPCWSTR lpName_str = NULL;
	DeeStringObject *lpName = (DeeStringObject *)Dee_None;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_hFile_lpFileMappingAttributes_flProtect_dwMaximumSize_lpName, "o|o" UNPu32 UNPu64 "o:CreateFileMapping", &hFile, &lpFileMappingAttributes, &flProtect, &dwMaximumSize, &lpName))
		goto err;
	if (DeeNTSystem_TryGetHandle(hFile, (void **)&hhFile))
		goto err;
	if (!DeeNone_Check(lpName)) {
		if (DeeObject_AssertTypeExact(lpName, &DeeString_Type))
			goto err;
		lpName_str = (LPCWSTR)DeeString_AsWide((DeeObject *)lpName);
		if unlikely(!lpName_str)
			goto err;
	}
	return libwin32_CreateFileMapping_f_impl(hhFile, lpFileMappingAttributes, flProtect, dwMaximumSize, lpName_str);
err:
	return NULL;
}
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
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetCurrentProcess_f_impl(void);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetCurrentProcess_f(size_t argc, DeeObject *const *argv);
#define LIBWIN32_GETCURRENTPROCESS_DEF { "GetCurrentProcess", (DeeObject *)&libwin32_GetCurrentProcess, MODSYM_FNORMAL, DOC("->?GHANDLE") },
#define LIBWIN32_GETCURRENTPROCESS_DEF_DOC(doc) { "GetCurrentProcess", (DeeObject *)&libwin32_GetCurrentProcess, MODSYM_FNORMAL, DOC("->?GHANDLE\n" doc) },
PRIVATE DEFINE_CMETHOD(libwin32_GetCurrentProcess, libwin32_GetCurrentProcess_f);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetCurrentProcess_f(size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":GetCurrentProcess"))
		goto err;
	return libwin32_GetCurrentProcess_f_impl();
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetCurrentProcess_f_impl(void)
/*[[[end]]]*/
{
	HANDLE hCurrentProcess;
	DBG_ALIGNMENT_DISABLE();
	hCurrentProcess = GetCurrentProcess();
	DBG_ALIGNMENT_ENABLE();
	return libwin32_CreateHandle(hCurrentProcess);
}



/*[[[deemon import("rt.gen.dexutils").gw("GetCurrentThread", "->?GHANDLE"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetCurrentThread_f_impl(void);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetCurrentThread_f(size_t argc, DeeObject *const *argv);
#define LIBWIN32_GETCURRENTTHREAD_DEF { "GetCurrentThread", (DeeObject *)&libwin32_GetCurrentThread, MODSYM_FNORMAL, DOC("->?GHANDLE") },
#define LIBWIN32_GETCURRENTTHREAD_DEF_DOC(doc) { "GetCurrentThread", (DeeObject *)&libwin32_GetCurrentThread, MODSYM_FNORMAL, DOC("->?GHANDLE\n" doc) },
PRIVATE DEFINE_CMETHOD(libwin32_GetCurrentThread, libwin32_GetCurrentThread_f);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetCurrentThread_f(size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":GetCurrentThread"))
		goto err;
	return libwin32_GetCurrentThread_f_impl();
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetCurrentThread_f_impl(void)
/*[[[end]]]*/
{
	HANDLE hCurrentThread;
	DBG_ALIGNMENT_DISABLE();
	hCurrentThread = GetCurrentProcess();
	DBG_ALIGNMENT_ENABLE();
	return libwin32_CreateHandle(hCurrentThread);
}



/*[[[deemon import("rt.gen.dexutils").gw("GetCurrentProcessId", "->?Dint"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetCurrentProcessId_f_impl(void);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetCurrentProcessId_f(size_t argc, DeeObject *const *argv);
#define LIBWIN32_GETCURRENTPROCESSID_DEF { "GetCurrentProcessId", (DeeObject *)&libwin32_GetCurrentProcessId, MODSYM_FNORMAL, DOC("->?Dint") },
#define LIBWIN32_GETCURRENTPROCESSID_DEF_DOC(doc) { "GetCurrentProcessId", (DeeObject *)&libwin32_GetCurrentProcessId, MODSYM_FNORMAL, DOC("->?Dint\n" doc) },
PRIVATE DEFINE_CMETHOD(libwin32_GetCurrentProcessId, libwin32_GetCurrentProcessId_f);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetCurrentProcessId_f(size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":GetCurrentProcessId"))
		goto err;
	return libwin32_GetCurrentProcessId_f_impl();
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetCurrentProcessId_f_impl(void)
/*[[[end]]]*/
{
	DWORD dwCurrentProcessId;
	DBG_ALIGNMENT_DISABLE();
	dwCurrentProcessId = GetCurrentProcessId();
	DBG_ALIGNMENT_ENABLE();
	return DeeInt_NewU32(dwCurrentProcessId);
}



/*[[[deemon import("rt.gen.dexutils").gw("GetCurrentThreadId", "->?Dint"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetCurrentThreadId_f_impl(void);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetCurrentThreadId_f(size_t argc, DeeObject *const *argv);
#define LIBWIN32_GETCURRENTTHREADID_DEF { "GetCurrentThreadId", (DeeObject *)&libwin32_GetCurrentThreadId, MODSYM_FNORMAL, DOC("->?Dint") },
#define LIBWIN32_GETCURRENTTHREADID_DEF_DOC(doc) { "GetCurrentThreadId", (DeeObject *)&libwin32_GetCurrentThreadId, MODSYM_FNORMAL, DOC("->?Dint\n" doc) },
PRIVATE DEFINE_CMETHOD(libwin32_GetCurrentThreadId, libwin32_GetCurrentThreadId_f);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetCurrentThreadId_f(size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":GetCurrentThreadId"))
		goto err;
	return libwin32_GetCurrentThreadId_f_impl();
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetCurrentThreadId_f_impl(void)
/*[[[end]]]*/
{
	DWORD dwCurrentThreadId;
	DBG_ALIGNMENT_DISABLE();
	dwCurrentThreadId = GetCurrentThreadId();
	DBG_ALIGNMENT_ENABLE();
	return DeeInt_NewU32(dwCurrentThreadId);
}



/*[[[deemon import("rt.gen.dexutils").gw("GetStdHandle", "nStdHandle:I32d->?GHANDLE"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetStdHandle_f_impl(int32_t nStdHandle);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetStdHandle_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_GETSTDHANDLE_DEF { "GetStdHandle", (DeeObject *)&libwin32_GetStdHandle, MODSYM_FNORMAL, DOC("(nStdHandle:?Dint)->?GHANDLE") },
#define LIBWIN32_GETSTDHANDLE_DEF_DOC(doc) { "GetStdHandle", (DeeObject *)&libwin32_GetStdHandle, MODSYM_FNORMAL, DOC("(nStdHandle:?Dint)->?GHANDLE\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_GetStdHandle, &libwin32_GetStdHandle_f);
#ifndef LIBWIN32_KWDS_NSTDHANDLE_DEFINED
#define LIBWIN32_KWDS_NSTDHANDLE_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_nStdHandle, { K(nStdHandle), KEND });
#endif /* !LIBWIN32_KWDS_NSTDHANDLE_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetStdHandle_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	int32_t nStdHandle;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_nStdHandle, UNPd32 ":GetStdHandle", &nStdHandle))
		goto err;
	return libwin32_GetStdHandle_f_impl(nStdHandle);
err:
	return NULL;
}
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
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_SetStdHandle_f_impl(DWORD nStdHandle, HANDLE hHandle);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_SetStdHandle_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_SETSTDHANDLE_DEF { "SetStdHandle", (DeeObject *)&libwin32_SetStdHandle, MODSYM_FNORMAL, DOC("(nStdHandle:?Dint,hHandle:?X3?Dint?DFile?Ewin32:HANDLE)") },
#define LIBWIN32_SETSTDHANDLE_DEF_DOC(doc) { "SetStdHandle", (DeeObject *)&libwin32_SetStdHandle, MODSYM_FNORMAL, DOC("(nStdHandle:?Dint,hHandle:?X3?Dint?DFile?Ewin32:HANDLE)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_SetStdHandle, &libwin32_SetStdHandle_f);
#ifndef LIBWIN32_KWDS_NSTDHANDLE_HHANDLE_DEFINED
#define LIBWIN32_KWDS_NSTDHANDLE_HHANDLE_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_nStdHandle_hHandle, { K(nStdHandle), K(hHandle), KEND });
#endif /* !LIBWIN32_KWDS_NSTDHANDLE_HHANDLE_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_SetStdHandle_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DWORD nStdHandle;
	HANDLE hhHandle;
	DeeObject *hHandle;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_nStdHandle_hHandle, UNPu32 "o:SetStdHandle", &nStdHandle, &hHandle))
		goto err;
	if (DeeNTSystem_TryGetHandle(hHandle, (void **)&hhHandle))
		goto err;
	return libwin32_SetStdHandle_f_impl(nStdHandle, hhHandle);
err:
	return NULL;
}
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
     ",dwOpenMode:nt:DWORD=PIPE_ACCESS_DUPLEX"
     ",dwPipeMode:nt:DWORD=PIPE_TYPE_BYTE|PIPE_READMODE_BYTE"
     ",nMaxInstances:nt:DWORD=PIPE_UNLIMITED_INSTANCES"
     ",nOutBufferSize:nt:DWORD=65536"
     ",nInBufferSize:nt:DWORD=65536"
     ",nDefaultTimeOut:nt:DWORD=0"
     ",lpSecurityAttributes:?GSECURITY_ATTRIBUTES=NULL"
     "->" MAYBE_NONE("?GHANDLE")
     ); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_CreateNamedPipe_f_impl(LPCWSTR lpName, DWORD dwOpenMode, DWORD dwPipeMode, DWORD nMaxInstances, DWORD nOutBufferSize, DWORD nInBufferSize, DWORD nDefaultTimeOut, DeeObject *lpSecurityAttributes);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_CreateNamedPipe_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_CREATENAMEDPIPE_DEF { "CreateNamedPipe", (DeeObject *)&libwin32_CreateNamedPipe, MODSYM_FNORMAL, DOC("(lpName:?Dstring,dwOpenMode:?Dint=!GPIPE_ACCESS_DUPLEX,dwPipeMode:?Dint=PIPE_TYPE_BYTE|PIPE_READMODE_BYTE,nMaxInstances:?Dint=!GPIPE_UNLIMITED_INSTANCES,nOutBufferSize:?Dint=!65536,nInBufferSize:?Dint=!65536,nDefaultTimeOut:?Dint=!0,lpSecurityAttributes?:?GSECURITY_ATTRIBUTES)->?GHANDLE") },
#define LIBWIN32_CREATENAMEDPIPE_DEF_DOC(doc) { "CreateNamedPipe", (DeeObject *)&libwin32_CreateNamedPipe, MODSYM_FNORMAL, DOC("(lpName:?Dstring,dwOpenMode:?Dint=!GPIPE_ACCESS_DUPLEX,dwPipeMode:?Dint=PIPE_TYPE_BYTE|PIPE_READMODE_BYTE,nMaxInstances:?Dint=!GPIPE_UNLIMITED_INSTANCES,nOutBufferSize:?Dint=!65536,nInBufferSize:?Dint=!65536,nDefaultTimeOut:?Dint=!0,lpSecurityAttributes?:?GSECURITY_ATTRIBUTES)->?GHANDLE\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_CreateNamedPipe, &libwin32_CreateNamedPipe_f);
#ifndef LIBWIN32_KWDS_LPNAME_DWOPENMODE_DWPIPEMODE_NMAXINSTANCES_NOUTBUFFERSIZE_NINBUFFERSIZE_NDEFAULTTIMEOUT_LPSECURITYATTRIBUTES_DEFINED
#define LIBWIN32_KWDS_LPNAME_DWOPENMODE_DWPIPEMODE_NMAXINSTANCES_NOUTBUFFERSIZE_NINBUFFERSIZE_NDEFAULTTIMEOUT_LPSECURITYATTRIBUTES_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_lpName_dwOpenMode_dwPipeMode_nMaxInstances_nOutBufferSize_nInBufferSize_nDefaultTimeOut_lpSecurityAttributes, { K(lpName), K(dwOpenMode), K(dwPipeMode), K(nMaxInstances), K(nOutBufferSize), K(nInBufferSize), K(nDefaultTimeOut), K(lpSecurityAttributes), KEND });
#endif /* !LIBWIN32_KWDS_LPNAME_DWOPENMODE_DWPIPEMODE_NMAXINSTANCES_NOUTBUFFERSIZE_NINBUFFERSIZE_NDEFAULTTIMEOUT_LPSECURITYATTRIBUTES_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_CreateNamedPipe_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	LPCWSTR lpName_str;
	DeeStringObject *lpName;
	DWORD dwOpenMode = PIPE_ACCESS_DUPLEX;
	DWORD dwPipeMode = PIPE_TYPE_BYTE|PIPE_READMODE_BYTE;
	DWORD nMaxInstances = PIPE_UNLIMITED_INSTANCES;
	DWORD nOutBufferSize = 65536;
	DWORD nInBufferSize = 65536;
	DWORD nDefaultTimeOut = 0;
	DeeObject *lpSecurityAttributes = NULL;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_lpName_dwOpenMode_dwPipeMode_nMaxInstances_nOutBufferSize_nInBufferSize_nDefaultTimeOut_lpSecurityAttributes, "o|" UNPu32 UNPu32 UNPu32 UNPu32 UNPu32 UNPu32 "o:CreateNamedPipe", &lpName, &dwOpenMode, &dwPipeMode, &nMaxInstances, &nOutBufferSize, &nInBufferSize, &nDefaultTimeOut, &lpSecurityAttributes))
		goto err;
	if (DeeObject_AssertTypeExact(lpName, &DeeString_Type))
		goto err;
	lpName_str = (LPCWSTR)DeeString_AsWide((DeeObject *)lpName);
	if unlikely(!lpName_str)
		goto err;
	return libwin32_CreateNamedPipe_f_impl(lpName_str, dwOpenMode, dwPipeMode, nMaxInstances, nOutBufferSize, nInBufferSize, nDefaultTimeOut, lpSecurityAttributes);
err:
	return NULL;
}
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
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_ConnectNamedPipe_f_impl(HANDLE hNamedPipe, DeeObject *lpOverlapped);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_ConnectNamedPipe_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_CONNECTNAMEDPIPE_DEF { "ConnectNamedPipe", (DeeObject *)&libwin32_ConnectNamedPipe, MODSYM_FNORMAL, DOC("(hNamedPipe:?X3?Dint?DFile?Ewin32:HANDLE,lpOverlapped?:?GOVERLAPPED)") },
#define LIBWIN32_CONNECTNAMEDPIPE_DEF_DOC(doc) { "ConnectNamedPipe", (DeeObject *)&libwin32_ConnectNamedPipe, MODSYM_FNORMAL, DOC("(hNamedPipe:?X3?Dint?DFile?Ewin32:HANDLE,lpOverlapped?:?GOVERLAPPED)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_ConnectNamedPipe, &libwin32_ConnectNamedPipe_f);
#ifndef LIBWIN32_KWDS_HNAMEDPIPE_LPOVERLAPPED_DEFINED
#define LIBWIN32_KWDS_HNAMEDPIPE_LPOVERLAPPED_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_hNamedPipe_lpOverlapped, { K(hNamedPipe), K(lpOverlapped), KEND });
#endif /* !LIBWIN32_KWDS_HNAMEDPIPE_LPOVERLAPPED_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_ConnectNamedPipe_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	HANDLE hhNamedPipe;
	DeeObject *hNamedPipe;
	DeeObject *lpOverlapped = NULL;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_hNamedPipe_lpOverlapped, "o|o:ConnectNamedPipe", &hNamedPipe, &lpOverlapped))
		goto err;
	if (DeeNTSystem_TryGetHandle(hNamedPipe, (void **)&hhNamedPipe))
		goto err;
	return libwin32_ConnectNamedPipe_f_impl(hhNamedPipe, lpOverlapped);
err:
	return NULL;
}
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
     ",nTimeOut:nt:DWORD=NMPWAIT_WAIT_FOREVER"
     "->" ERROR_OR_BOOL
     ); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_WaitNamedPipe_f_impl(LPCWSTR lpNamedPipeName, DWORD nTimeOut);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_WaitNamedPipe_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_WAITNAMEDPIPE_DEF { "WaitNamedPipe", (DeeObject *)&libwin32_WaitNamedPipe, MODSYM_FNORMAL, DOC("(lpNamedPipeName:?Dstring,nTimeOut:?Dint=!GNMPWAIT_WAIT_FOREVER)") },
#define LIBWIN32_WAITNAMEDPIPE_DEF_DOC(doc) { "WaitNamedPipe", (DeeObject *)&libwin32_WaitNamedPipe, MODSYM_FNORMAL, DOC("(lpNamedPipeName:?Dstring,nTimeOut:?Dint=!GNMPWAIT_WAIT_FOREVER)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_WaitNamedPipe, &libwin32_WaitNamedPipe_f);
#ifndef LIBWIN32_KWDS_LPNAMEDPIPENAME_NTIMEOUT_DEFINED
#define LIBWIN32_KWDS_LPNAMEDPIPENAME_NTIMEOUT_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_lpNamedPipeName_nTimeOut, { K(lpNamedPipeName), K(nTimeOut), KEND });
#endif /* !LIBWIN32_KWDS_LPNAMEDPIPENAME_NTIMEOUT_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_WaitNamedPipe_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	LPCWSTR lpNamedPipeName_str;
	DeeStringObject *lpNamedPipeName;
	DWORD nTimeOut = NMPWAIT_WAIT_FOREVER;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_lpNamedPipeName_nTimeOut, "o|" UNPu32 ":WaitNamedPipe", &lpNamedPipeName, &nTimeOut))
		goto err;
	if (DeeObject_AssertTypeExact(lpNamedPipeName, &DeeString_Type))
		goto err;
	lpNamedPipeName_str = (LPCWSTR)DeeString_AsWide((DeeObject *)lpNamedPipeName);
	if unlikely(!lpNamedPipeName_str)
		goto err;
	return libwin32_WaitNamedPipe_f_impl(lpNamedPipeName_str, nTimeOut);
err:
	return NULL;
}
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
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_OpenProcess_f_impl(DWORD dwDesiredAccess, bool bInheritHandle, DWORD dwProcessId);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_OpenProcess_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_OPENPROCESS_DEF { "OpenProcess", (DeeObject *)&libwin32_OpenProcess, MODSYM_FNORMAL, DOC("(dwDesiredAccess:?Dint,bInheritHandle:?Dbool,dwProcessId:?Dint)->?GHANDLE") },
#define LIBWIN32_OPENPROCESS_DEF_DOC(doc) { "OpenProcess", (DeeObject *)&libwin32_OpenProcess, MODSYM_FNORMAL, DOC("(dwDesiredAccess:?Dint,bInheritHandle:?Dbool,dwProcessId:?Dint)->?GHANDLE\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_OpenProcess, &libwin32_OpenProcess_f);
#ifndef LIBWIN32_KWDS_DWDESIREDACCESS_BINHERITHANDLE_DWPROCESSID_DEFINED
#define LIBWIN32_KWDS_DWDESIREDACCESS_BINHERITHANDLE_DWPROCESSID_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_dwDesiredAccess_bInheritHandle_dwProcessId, { K(dwDesiredAccess), K(bInheritHandle), K(dwProcessId), KEND });
#endif /* !LIBWIN32_KWDS_DWDESIREDACCESS_BINHERITHANDLE_DWPROCESSID_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_OpenProcess_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DWORD dwDesiredAccess;
	bool bInheritHandle;
	DWORD dwProcessId;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_dwDesiredAccess_bInheritHandle_dwProcessId, UNPu32 "b" UNPu32 ":OpenProcess", &dwDesiredAccess, &bInheritHandle, &dwProcessId))
		goto err;
	return libwin32_OpenProcess_f_impl(dwDesiredAccess, bInheritHandle, dwProcessId);
err:
	return NULL;
}
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
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetExitCodeProcess_f_impl(HANDLE hProcess);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetExitCodeProcess_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_GETEXITCODEPROCESS_DEF { "GetExitCodeProcess", (DeeObject *)&libwin32_GetExitCodeProcess, MODSYM_FNORMAL, DOC("(hProcess:?X3?Dint?DFile?Ewin32:HANDLE)->?Dint") },
#define LIBWIN32_GETEXITCODEPROCESS_DEF_DOC(doc) { "GetExitCodeProcess", (DeeObject *)&libwin32_GetExitCodeProcess, MODSYM_FNORMAL, DOC("(hProcess:?X3?Dint?DFile?Ewin32:HANDLE)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_GetExitCodeProcess, &libwin32_GetExitCodeProcess_f);
#ifndef LIBWIN32_KWDS_HPROCESS_DEFINED
#define LIBWIN32_KWDS_HPROCESS_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_hProcess, { K(hProcess), KEND });
#endif /* !LIBWIN32_KWDS_HPROCESS_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetExitCodeProcess_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	HANDLE hhProcess;
	DeeObject *hProcess;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_hProcess, "o:GetExitCodeProcess", &hProcess))
		goto err;
	if (DeeNTSystem_TryGetHandle(hProcess, (void **)&hhProcess))
		goto err;
	return libwin32_GetExitCodeProcess_f_impl(hhProcess);
err:
	return NULL;
}
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
	return DeeInt_NewU32(dwExitCode);
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
     ",dwFilterFlag:nt:DWORD=LIST_MODULES_DEFAULT"
     "->" MAYBE_NONE("?S?GHANDLE")); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_EnumProcessModules_f_impl(HANDLE hProcess, DWORD dwFilterFlag);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_EnumProcessModules_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_ENUMPROCESSMODULES_DEF { "EnumProcessModules", (DeeObject *)&libwin32_EnumProcessModules, MODSYM_FNORMAL, DOC("(hProcess:?X3?Dint?DFile?Ewin32:HANDLE,dwFilterFlag:?Dint=!GLIST_MODULES_DEFAULT)->?S?GHANDLE") },
#define LIBWIN32_ENUMPROCESSMODULES_DEF_DOC(doc) { "EnumProcessModules", (DeeObject *)&libwin32_EnumProcessModules, MODSYM_FNORMAL, DOC("(hProcess:?X3?Dint?DFile?Ewin32:HANDLE,dwFilterFlag:?Dint=!GLIST_MODULES_DEFAULT)->?S?GHANDLE\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_EnumProcessModules, &libwin32_EnumProcessModules_f);
#ifndef LIBWIN32_KWDS_HPROCESS_DWFILTERFLAG_DEFINED
#define LIBWIN32_KWDS_HPROCESS_DWFILTERFLAG_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_hProcess_dwFilterFlag, { K(hProcess), K(dwFilterFlag), KEND });
#endif /* !LIBWIN32_KWDS_HPROCESS_DWFILTERFLAG_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_EnumProcessModules_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	HANDLE hhProcess;
	DeeObject *hProcess;
	DWORD dwFilterFlag = LIST_MODULES_DEFAULT;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_hProcess_dwFilterFlag, "o|" UNPu32 ":EnumProcessModules", &hProcess, &dwFilterFlag))
		goto err;
	if (DeeNTSystem_TryGetHandle(hProcess, (void **)&hhProcess))
		goto err;
	return libwin32_EnumProcessModules_f_impl(hhProcess, dwFilterFlag);
err:
	return NULL;
}
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
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_EnumProcesses_f_impl(void);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_EnumProcesses_f(size_t argc, DeeObject *const *argv);
#define LIBWIN32_ENUMPROCESSES_DEF { "EnumProcesses", (DeeObject *)&libwin32_EnumProcesses, MODSYM_FNORMAL, DOC("->?S?Dint") },
#define LIBWIN32_ENUMPROCESSES_DEF_DOC(doc) { "EnumProcesses", (DeeObject *)&libwin32_EnumProcesses, MODSYM_FNORMAL, DOC("->?S?Dint\n" doc) },
PRIVATE DEFINE_CMETHOD(libwin32_EnumProcesses, libwin32_EnumProcesses_f);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_EnumProcesses_f(size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":EnumProcesses"))
		goto err;
	return libwin32_EnumProcesses_f_impl();
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_EnumProcesses_f_impl(void)
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
			pid_obj = DeeInt_NewU32(pidProcesses[i]);
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
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetProcessImageFileName_f_impl(HANDLE hProcess);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetProcessImageFileName_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_GETPROCESSIMAGEFILENAME_DEF { "GetProcessImageFileName", (DeeObject *)&libwin32_GetProcessImageFileName, MODSYM_FNORMAL, DOC("(hProcess:?X3?Dint?DFile?Ewin32:HANDLE)->?Dstring") },
#define LIBWIN32_GETPROCESSIMAGEFILENAME_DEF_DOC(doc) { "GetProcessImageFileName", (DeeObject *)&libwin32_GetProcessImageFileName, MODSYM_FNORMAL, DOC("(hProcess:?X3?Dint?DFile?Ewin32:HANDLE)->?Dstring\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_GetProcessImageFileName, &libwin32_GetProcessImageFileName_f);
#ifndef LIBWIN32_KWDS_HPROCESS_DEFINED
#define LIBWIN32_KWDS_HPROCESS_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_hProcess, { K(hProcess), KEND });
#endif /* !LIBWIN32_KWDS_HPROCESS_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetProcessImageFileName_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	HANDLE hhProcess;
	DeeObject *hProcess;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_hProcess, "o:GetProcessImageFileName", &hProcess))
		goto err;
	if (DeeNTSystem_TryGetHandle(hProcess, (void **)&hhProcess))
		goto err;
	return libwin32_GetProcessImageFileName_f_impl(hhProcess);
err:
	return NULL;
}
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
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetModuleBaseName_f_impl(HANDLE hProcess, HANDLE hModule);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetModuleBaseName_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_GETMODULEBASENAME_DEF { "GetModuleBaseName", (DeeObject *)&libwin32_GetModuleBaseName, MODSYM_FNORMAL, DOC("(hProcess:?X3?Dint?DFile?Ewin32:HANDLE,hModule:?X3?Dint?DFile?Ewin32:HANDLE)->?Dstring") },
#define LIBWIN32_GETMODULEBASENAME_DEF_DOC(doc) { "GetModuleBaseName", (DeeObject *)&libwin32_GetModuleBaseName, MODSYM_FNORMAL, DOC("(hProcess:?X3?Dint?DFile?Ewin32:HANDLE,hModule:?X3?Dint?DFile?Ewin32:HANDLE)->?Dstring\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_GetModuleBaseName, &libwin32_GetModuleBaseName_f);
#ifndef LIBWIN32_KWDS_HPROCESS_HMODULE_DEFINED
#define LIBWIN32_KWDS_HPROCESS_HMODULE_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_hProcess_hModule, { K(hProcess), K(hModule), KEND });
#endif /* !LIBWIN32_KWDS_HPROCESS_HMODULE_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetModuleBaseName_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	HANDLE hhProcess;
	DeeObject *hProcess;
	HANDLE hhModule;
	DeeObject *hModule;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_hProcess_hModule, "oo:GetModuleBaseName", &hProcess, &hModule))
		goto err;
	if (DeeNTSystem_TryGetHandle(hProcess, (void **)&hhProcess))
		goto err;
	if (DeeNTSystem_TryGetHandle(hModule, (void **)&hhModule))
		goto err;
	return libwin32_GetModuleBaseName_f_impl(hhProcess, hhModule);
err:
	return NULL;
}
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
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetModuleFileNameEx_f_impl(HANDLE hProcess, HANDLE hModule);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetModuleFileNameEx_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_GETMODULEFILENAMEEX_DEF { "GetModuleFileNameEx", (DeeObject *)&libwin32_GetModuleFileNameEx, MODSYM_FNORMAL, DOC("(hProcess:?X3?Dint?DFile?Ewin32:HANDLE,hModule:?X3?Dint?DFile?Ewin32:HANDLE)->?Dstring") },
#define LIBWIN32_GETMODULEFILENAMEEX_DEF_DOC(doc) { "GetModuleFileNameEx", (DeeObject *)&libwin32_GetModuleFileNameEx, MODSYM_FNORMAL, DOC("(hProcess:?X3?Dint?DFile?Ewin32:HANDLE,hModule:?X3?Dint?DFile?Ewin32:HANDLE)->?Dstring\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_GetModuleFileNameEx, &libwin32_GetModuleFileNameEx_f);
#ifndef LIBWIN32_KWDS_HPROCESS_HMODULE_DEFINED
#define LIBWIN32_KWDS_HPROCESS_HMODULE_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_hProcess_hModule, { K(hProcess), K(hModule), KEND });
#endif /* !LIBWIN32_KWDS_HPROCESS_HMODULE_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetModuleFileNameEx_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	HANDLE hhProcess;
	DeeObject *hProcess;
	HANDLE hhModule;
	DeeObject *hModule;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_hProcess_hModule, "oo:GetModuleFileNameEx", &hProcess, &hModule))
		goto err;
	if (DeeNTSystem_TryGetHandle(hProcess, (void **)&hhProcess))
		goto err;
	if (DeeNTSystem_TryGetHandle(hModule, (void **)&hhModule))
		goto err;
	return libwin32_GetModuleFileNameEx_f_impl(hhProcess, hhModule);
err:
	return NULL;
}
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
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_TerminateProcess_f_impl(HANDLE hProcess, UINT uExitCode);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_TerminateProcess_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_TERMINATEPROCESS_DEF { "TerminateProcess", (DeeObject *)&libwin32_TerminateProcess, MODSYM_FNORMAL, DOC("(hProcess:?X3?Dint?DFile?Ewin32:HANDLE,uExitCode:?Dint)") },
#define LIBWIN32_TERMINATEPROCESS_DEF_DOC(doc) { "TerminateProcess", (DeeObject *)&libwin32_TerminateProcess, MODSYM_FNORMAL, DOC("(hProcess:?X3?Dint?DFile?Ewin32:HANDLE,uExitCode:?Dint)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_TerminateProcess, &libwin32_TerminateProcess_f);
#ifndef LIBWIN32_KWDS_HPROCESS_UEXITCODE_DEFINED
#define LIBWIN32_KWDS_HPROCESS_UEXITCODE_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_hProcess_uExitCode, { K(hProcess), K(uExitCode), KEND });
#endif /* !LIBWIN32_KWDS_HPROCESS_UEXITCODE_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_TerminateProcess_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	HANDLE hhProcess;
	DeeObject *hProcess;
	UINT uExitCode;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_hProcess_uExitCode, "o" UNPu32 ":TerminateProcess", &hProcess, &uExitCode))
		goto err;
	if (DeeNTSystem_TryGetHandle(hProcess, (void **)&hhProcess))
		goto err;
	return libwin32_TerminateProcess_f_impl(hhProcess, uExitCode);
err:
	return NULL;
}
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
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_TerminateThread_f_impl(HANDLE hThread, DWORD dwExitCode);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_TerminateThread_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_TERMINATETHREAD_DEF { "TerminateThread", (DeeObject *)&libwin32_TerminateThread, MODSYM_FNORMAL, DOC("(hThread:?X3?Dint?DFile?Ewin32:HANDLE,dwExitCode:?Dint)") },
#define LIBWIN32_TERMINATETHREAD_DEF_DOC(doc) { "TerminateThread", (DeeObject *)&libwin32_TerminateThread, MODSYM_FNORMAL, DOC("(hThread:?X3?Dint?DFile?Ewin32:HANDLE,dwExitCode:?Dint)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_TerminateThread, &libwin32_TerminateThread_f);
#ifndef LIBWIN32_KWDS_HTHREAD_DWEXITCODE_DEFINED
#define LIBWIN32_KWDS_HTHREAD_DWEXITCODE_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_hThread_dwExitCode, { K(hThread), K(dwExitCode), KEND });
#endif /* !LIBWIN32_KWDS_HTHREAD_DWEXITCODE_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_TerminateThread_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	HANDLE hhThread;
	DeeObject *hThread;
	DWORD dwExitCode;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_hThread_dwExitCode, "o" UNPu32 ":TerminateThread", &hThread, &dwExitCode))
		goto err;
	if (DeeNTSystem_TryGetHandle(hThread, (void **)&hhThread))
		goto err;
	return libwin32_TerminateThread_f_impl(hhThread, dwExitCode);
err:
	return NULL;
}
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
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_SuspendThread_f_impl(HANDLE hThread);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_SuspendThread_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_SUSPENDTHREAD_DEF { "SuspendThread", (DeeObject *)&libwin32_SuspendThread, MODSYM_FNORMAL, DOC("(hThread:?X3?Dint?DFile?Ewin32:HANDLE)->?Dint") },
#define LIBWIN32_SUSPENDTHREAD_DEF_DOC(doc) { "SuspendThread", (DeeObject *)&libwin32_SuspendThread, MODSYM_FNORMAL, DOC("(hThread:?X3?Dint?DFile?Ewin32:HANDLE)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_SuspendThread, &libwin32_SuspendThread_f);
#ifndef LIBWIN32_KWDS_HTHREAD_DEFINED
#define LIBWIN32_KWDS_HTHREAD_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_hThread, { K(hThread), KEND });
#endif /* !LIBWIN32_KWDS_HTHREAD_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_SuspendThread_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	HANDLE hhThread;
	DeeObject *hThread;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_hThread, "o:SuspendThread", &hThread))
		goto err;
	if (DeeNTSystem_TryGetHandle(hThread, (void **)&hhThread))
		goto err;
	return libwin32_SuspendThread_f_impl(hhThread);
err:
	return NULL;
}
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
	return DeeInt_NewU32(dwResult);
err:
	return NULL;
}


/*[[[deemon import("rt.gen.dexutils").gw("ResumeThread",
      "hThread:nt:HANDLE"
     "->" MAYBE_NONE("?Dint")); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_ResumeThread_f_impl(HANDLE hThread);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_ResumeThread_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_RESUMETHREAD_DEF { "ResumeThread", (DeeObject *)&libwin32_ResumeThread, MODSYM_FNORMAL, DOC("(hThread:?X3?Dint?DFile?Ewin32:HANDLE)->?Dint") },
#define LIBWIN32_RESUMETHREAD_DEF_DOC(doc) { "ResumeThread", (DeeObject *)&libwin32_ResumeThread, MODSYM_FNORMAL, DOC("(hThread:?X3?Dint?DFile?Ewin32:HANDLE)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_ResumeThread, &libwin32_ResumeThread_f);
#ifndef LIBWIN32_KWDS_HTHREAD_DEFINED
#define LIBWIN32_KWDS_HTHREAD_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_hThread, { K(hThread), KEND });
#endif /* !LIBWIN32_KWDS_HTHREAD_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_ResumeThread_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	HANDLE hhThread;
	DeeObject *hThread;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_hThread, "o:ResumeThread", &hThread))
		goto err;
	if (DeeNTSystem_TryGetHandle(hThread, (void **)&hhThread))
		goto err;
	return libwin32_ResumeThread_f_impl(hhThread);
err:
	return NULL;
}
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
	return DeeInt_NewU32(dwResult);
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
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_NtQueryInformationProcess_f_impl(HANDLE ProcessHandle, unsigned int ProcessInformationClass, size_t ProcessInformationLength);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_NtQueryInformationProcess_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_NTQUERYINFORMATIONPROCESS_DEF { "NtQueryInformationProcess", (DeeObject *)&libwin32_NtQueryInformationProcess, MODSYM_FNORMAL, DOC("(ProcessHandle:?X3?Dint?DFile?Ewin32:HANDLE,ProcessInformationClass:?Dint,ProcessInformationLength:?Dint=!64)->?DBytes") },
#define LIBWIN32_NTQUERYINFORMATIONPROCESS_DEF_DOC(doc) { "NtQueryInformationProcess", (DeeObject *)&libwin32_NtQueryInformationProcess, MODSYM_FNORMAL, DOC("(ProcessHandle:?X3?Dint?DFile?Ewin32:HANDLE,ProcessInformationClass:?Dint,ProcessInformationLength:?Dint=!64)->?DBytes\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_NtQueryInformationProcess, &libwin32_NtQueryInformationProcess_f);
#ifndef LIBWIN32_KWDS_PROCESSHANDLE_PROCESSINFORMATIONCLASS_PROCESSINFORMATIONLENGTH_DEFINED
#define LIBWIN32_KWDS_PROCESSHANDLE_PROCESSINFORMATIONCLASS_PROCESSINFORMATIONLENGTH_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_ProcessHandle_ProcessInformationClass_ProcessInformationLength, { K(ProcessHandle), K(ProcessInformationClass), K(ProcessInformationLength), KEND });
#endif /* !LIBWIN32_KWDS_PROCESSHANDLE_PROCESSINFORMATIONCLASS_PROCESSINFORMATIONLENGTH_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_NtQueryInformationProcess_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	HANDLE hProcessHandle;
	DeeObject *ProcessHandle;
	unsigned int ProcessInformationClass;
	size_t ProcessInformationLength = 64;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_ProcessHandle_ProcessInformationClass_ProcessInformationLength, "ou|" UNPuSIZ ":NtQueryInformationProcess", &ProcessHandle, &ProcessInformationClass, &ProcessInformationLength))
		goto err;
	if (DeeNTSystem_TryGetHandle(ProcessHandle, (void **)&hProcessHandle))
		goto err;
	return libwin32_NtQueryInformationProcess_f_impl(hProcessHandle, ProcessInformationClass, ProcessInformationLength);
err:
	return NULL;
}
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
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_NtWow64QueryInformationProcess64_f_impl(HANDLE ProcessHandle, unsigned int ProcessInformationClass, size_t ProcessInformationLength);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_NtWow64QueryInformationProcess64_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_NTWOW64QUERYINFORMATIONPROCESS64_DEF { "NtWow64QueryInformationProcess64", (DeeObject *)&libwin32_NtWow64QueryInformationProcess64, MODSYM_FNORMAL, DOC("(ProcessHandle:?X3?Dint?DFile?Ewin32:HANDLE,ProcessInformationClass:?Dint,ProcessInformationLength:?Dint=!64)->?DBytes") },
#define LIBWIN32_NTWOW64QUERYINFORMATIONPROCESS64_DEF_DOC(doc) { "NtWow64QueryInformationProcess64", (DeeObject *)&libwin32_NtWow64QueryInformationProcess64, MODSYM_FNORMAL, DOC("(ProcessHandle:?X3?Dint?DFile?Ewin32:HANDLE,ProcessInformationClass:?Dint,ProcessInformationLength:?Dint=!64)->?DBytes\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_NtWow64QueryInformationProcess64, &libwin32_NtWow64QueryInformationProcess64_f);
#ifndef LIBWIN32_KWDS_PROCESSHANDLE_PROCESSINFORMATIONCLASS_PROCESSINFORMATIONLENGTH_DEFINED
#define LIBWIN32_KWDS_PROCESSHANDLE_PROCESSINFORMATIONCLASS_PROCESSINFORMATIONLENGTH_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_ProcessHandle_ProcessInformationClass_ProcessInformationLength, { K(ProcessHandle), K(ProcessInformationClass), K(ProcessInformationLength), KEND });
#endif /* !LIBWIN32_KWDS_PROCESSHANDLE_PROCESSINFORMATIONCLASS_PROCESSINFORMATIONLENGTH_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_NtWow64QueryInformationProcess64_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	HANDLE hProcessHandle;
	DeeObject *ProcessHandle;
	unsigned int ProcessInformationClass;
	size_t ProcessInformationLength = 64;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_ProcessHandle_ProcessInformationClass_ProcessInformationLength, "ou|" UNPuSIZ ":NtWow64QueryInformationProcess64", &ProcessHandle, &ProcessInformationClass, &ProcessInformationLength))
		goto err;
	if (DeeNTSystem_TryGetHandle(ProcessHandle, (void **)&hProcessHandle))
		goto err;
	return libwin32_NtWow64QueryInformationProcess64_f_impl(hProcessHandle, ProcessInformationClass, ProcessInformationLength);
err:
	return NULL;
}
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
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_ReadProcessMemory_f_impl(HANDLE hProcess, uintptr_t lpBaseAddress, SIZE_T nSize);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_ReadProcessMemory_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_READPROCESSMEMORY_DEF { "ReadProcessMemory", (DeeObject *)&libwin32_ReadProcessMemory, MODSYM_FNORMAL, DOC("(hProcess:?X3?Dint?DFile?Ewin32:HANDLE,lpBaseAddress:?Dint,nSize:?Dint)->?DBytes") },
#define LIBWIN32_READPROCESSMEMORY_DEF_DOC(doc) { "ReadProcessMemory", (DeeObject *)&libwin32_ReadProcessMemory, MODSYM_FNORMAL, DOC("(hProcess:?X3?Dint?DFile?Ewin32:HANDLE,lpBaseAddress:?Dint,nSize:?Dint)->?DBytes\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_ReadProcessMemory, &libwin32_ReadProcessMemory_f);
#ifndef LIBWIN32_KWDS_HPROCESS_LPBASEADDRESS_NSIZE_DEFINED
#define LIBWIN32_KWDS_HPROCESS_LPBASEADDRESS_NSIZE_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_hProcess_lpBaseAddress_nSize, { K(hProcess), K(lpBaseAddress), K(nSize), KEND });
#endif /* !LIBWIN32_KWDS_HPROCESS_LPBASEADDRESS_NSIZE_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_ReadProcessMemory_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	HANDLE hhProcess;
	DeeObject *hProcess;
	uintptr_t lpBaseAddress;
	SIZE_T nSize;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_hProcess_lpBaseAddress_nSize, "o" UNPuSIZ UNPuSIZ ":ReadProcessMemory", &hProcess, &lpBaseAddress, &nSize))
		goto err;
	if (DeeNTSystem_TryGetHandle(hProcess, (void **)&hhProcess))
		goto err;
	return libwin32_ReadProcessMemory_f_impl(hhProcess, lpBaseAddress, nSize);
err:
	return NULL;
}
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
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_NtWow64ReadVirtualMemory64_f_impl(HANDLE hProcess, uint64_t lpBaseAddress, ULONG64 nSize);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_NtWow64ReadVirtualMemory64_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_NTWOW64READVIRTUALMEMORY64_DEF { "NtWow64ReadVirtualMemory64", (DeeObject *)&libwin32_NtWow64ReadVirtualMemory64, MODSYM_FNORMAL, DOC("(hProcess:?X3?Dint?DFile?Ewin32:HANDLE,lpBaseAddress:?Dint,nSize:?Dint)->?DBytes") },
#define LIBWIN32_NTWOW64READVIRTUALMEMORY64_DEF_DOC(doc) { "NtWow64ReadVirtualMemory64", (DeeObject *)&libwin32_NtWow64ReadVirtualMemory64, MODSYM_FNORMAL, DOC("(hProcess:?X3?Dint?DFile?Ewin32:HANDLE,lpBaseAddress:?Dint,nSize:?Dint)->?DBytes\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_NtWow64ReadVirtualMemory64, &libwin32_NtWow64ReadVirtualMemory64_f);
#ifndef LIBWIN32_KWDS_HPROCESS_LPBASEADDRESS_NSIZE_DEFINED
#define LIBWIN32_KWDS_HPROCESS_LPBASEADDRESS_NSIZE_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_hProcess_lpBaseAddress_nSize, { K(hProcess), K(lpBaseAddress), K(nSize), KEND });
#endif /* !LIBWIN32_KWDS_HPROCESS_LPBASEADDRESS_NSIZE_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_NtWow64ReadVirtualMemory64_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	HANDLE hhProcess;
	DeeObject *hProcess;
	uint64_t lpBaseAddress;
	ULONG64 nSize;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_hProcess_lpBaseAddress_nSize, "o" UNPu64 UNPu64 ":NtWow64ReadVirtualMemory64", &hProcess, &lpBaseAddress, &nSize))
		goto err;
	if (DeeNTSystem_TryGetHandle(hProcess, (void **)&hhProcess))
		goto err;
	return libwin32_NtWow64ReadVirtualMemory64_f_impl(hhProcess, lpBaseAddress, nSize);
err:
	return NULL;
}
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
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_WriteProcessMemory_f_impl(HANDLE hProcess, uintptr_t lpBaseAddress, DeeObject *lpBuffer);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_WriteProcessMemory_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_WRITEPROCESSMEMORY_DEF { "WriteProcessMemory", (DeeObject *)&libwin32_WriteProcessMemory, MODSYM_FNORMAL, DOC("(hProcess:?X3?Dint?DFile?Ewin32:HANDLE,lpBaseAddress:?Dint,lpBuffer:?DBytes)->?Dint") },
#define LIBWIN32_WRITEPROCESSMEMORY_DEF_DOC(doc) { "WriteProcessMemory", (DeeObject *)&libwin32_WriteProcessMemory, MODSYM_FNORMAL, DOC("(hProcess:?X3?Dint?DFile?Ewin32:HANDLE,lpBaseAddress:?Dint,lpBuffer:?DBytes)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_WriteProcessMemory, &libwin32_WriteProcessMemory_f);
#ifndef LIBWIN32_KWDS_HPROCESS_LPBASEADDRESS_LPBUFFER_DEFINED
#define LIBWIN32_KWDS_HPROCESS_LPBASEADDRESS_LPBUFFER_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_hProcess_lpBaseAddress_lpBuffer, { K(hProcess), K(lpBaseAddress), K(lpBuffer), KEND });
#endif /* !LIBWIN32_KWDS_HPROCESS_LPBASEADDRESS_LPBUFFER_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_WriteProcessMemory_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	HANDLE hhProcess;
	DeeObject *hProcess;
	uintptr_t lpBaseAddress;
	DeeObject *lpBuffer;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_hProcess_lpBaseAddress_lpBuffer, "o" UNPuSIZ "o:WriteProcessMemory", &hProcess, &lpBaseAddress, &lpBuffer))
		goto err;
	if (DeeNTSystem_TryGetHandle(hProcess, (void **)&hhProcess))
		goto err;
	return libwin32_WriteProcessMemory_f_impl(hhProcess, lpBaseAddress, lpBuffer);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_WriteProcessMemory_f_impl(HANDLE hProcess, uintptr_t lpBaseAddress, DeeObject *lpBuffer)
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
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_NtWow64WriteVirtualMemory64_f_impl(HANDLE hProcess, uint64_t lpBaseAddress, DeeObject *lpBuffer);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_NtWow64WriteVirtualMemory64_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_NTWOW64WRITEVIRTUALMEMORY64_DEF { "NtWow64WriteVirtualMemory64", (DeeObject *)&libwin32_NtWow64WriteVirtualMemory64, MODSYM_FNORMAL, DOC("(hProcess:?X3?Dint?DFile?Ewin32:HANDLE,lpBaseAddress:?Dint,lpBuffer:?DBytes)->?Dint") },
#define LIBWIN32_NTWOW64WRITEVIRTUALMEMORY64_DEF_DOC(doc) { "NtWow64WriteVirtualMemory64", (DeeObject *)&libwin32_NtWow64WriteVirtualMemory64, MODSYM_FNORMAL, DOC("(hProcess:?X3?Dint?DFile?Ewin32:HANDLE,lpBaseAddress:?Dint,lpBuffer:?DBytes)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_NtWow64WriteVirtualMemory64, &libwin32_NtWow64WriteVirtualMemory64_f);
#ifndef LIBWIN32_KWDS_HPROCESS_LPBASEADDRESS_LPBUFFER_DEFINED
#define LIBWIN32_KWDS_HPROCESS_LPBASEADDRESS_LPBUFFER_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_hProcess_lpBaseAddress_lpBuffer, { K(hProcess), K(lpBaseAddress), K(lpBuffer), KEND });
#endif /* !LIBWIN32_KWDS_HPROCESS_LPBASEADDRESS_LPBUFFER_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_NtWow64WriteVirtualMemory64_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	HANDLE hhProcess;
	DeeObject *hProcess;
	uint64_t lpBaseAddress;
	DeeObject *lpBuffer;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_hProcess_lpBaseAddress_lpBuffer, "o" UNPu64 "o:NtWow64WriteVirtualMemory64", &hProcess, &lpBaseAddress, &lpBuffer))
		goto err;
	if (DeeNTSystem_TryGetHandle(hProcess, (void **)&hhProcess))
		goto err;
	return libwin32_NtWow64WriteVirtualMemory64_f_impl(hhProcess, lpBaseAddress, lpBuffer);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_NtWow64WriteVirtualMemory64_f_impl(HANDLE hProcess, uint64_t lpBaseAddress, DeeObject *lpBuffer)
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
	return DeeInt_NewU64(szNumberOfBytesWritten);
err_buf:
	DeeObject_PutBuf(lpBuffer, &buf, Dee_BUFFER_FREADONLY);
err:
	return NULL;
#endif /* __SIZEOF_POINTER__ < 8 */
}


/*[[[deemon import("rt.gen.dexutils").gw("WaitForSingleObject",
      "hHandle:nt:HANDLE"
     ",dwMilliseconds:nt:DWORD=INFINITE"
     "->" MAYBE_NONE("?Dint")); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_WaitForSingleObject_f_impl(HANDLE hHandle, DWORD dwMilliseconds);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_WaitForSingleObject_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_WAITFORSINGLEOBJECT_DEF { "WaitForSingleObject", (DeeObject *)&libwin32_WaitForSingleObject, MODSYM_FNORMAL, DOC("(hHandle:?X3?Dint?DFile?Ewin32:HANDLE,dwMilliseconds:?Dint=!GINFINITE)->?Dint") },
#define LIBWIN32_WAITFORSINGLEOBJECT_DEF_DOC(doc) { "WaitForSingleObject", (DeeObject *)&libwin32_WaitForSingleObject, MODSYM_FNORMAL, DOC("(hHandle:?X3?Dint?DFile?Ewin32:HANDLE,dwMilliseconds:?Dint=!GINFINITE)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_WaitForSingleObject, &libwin32_WaitForSingleObject_f);
#ifndef LIBWIN32_KWDS_HHANDLE_DWMILLISECONDS_DEFINED
#define LIBWIN32_KWDS_HHANDLE_DWMILLISECONDS_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_hHandle_dwMilliseconds, { K(hHandle), K(dwMilliseconds), KEND });
#endif /* !LIBWIN32_KWDS_HHANDLE_DWMILLISECONDS_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_WaitForSingleObject_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	HANDLE hhHandle;
	DeeObject *hHandle;
	DWORD dwMilliseconds = INFINITE;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_hHandle_dwMilliseconds, "o|" UNPu32 ":WaitForSingleObject", &hHandle, &dwMilliseconds))
		goto err;
	if (DeeNTSystem_TryGetHandle(hHandle, (void **)&hhHandle))
		goto err;
	return libwin32_WaitForSingleObject_f_impl(hhHandle, dwMilliseconds);
err:
	return NULL;
}
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
	return DeeInt_NewU32(dwResult);
err:
	return NULL;
}


/*[[[deemon import("rt.gen.dexutils").gw("WaitForMultipleObjects",
      "lpHandles:obj:sequence"
     ",bWaitAll:c:bool"
     ",dwMilliseconds:nt:DWORD=INFINITE"
     "->" MAYBE_NONE("?Dint")); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_WaitForMultipleObjects_f_impl(DeeObject *lpHandles, bool bWaitAll, DWORD dwMilliseconds);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_WaitForMultipleObjects_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_WAITFORMULTIPLEOBJECTS_DEF { "WaitForMultipleObjects", (DeeObject *)&libwin32_WaitForMultipleObjects, MODSYM_FNORMAL, DOC("(lpHandles:?S?O,bWaitAll:?Dbool,dwMilliseconds:?Dint=!GINFINITE)->?Dint") },
#define LIBWIN32_WAITFORMULTIPLEOBJECTS_DEF_DOC(doc) { "WaitForMultipleObjects", (DeeObject *)&libwin32_WaitForMultipleObjects, MODSYM_FNORMAL, DOC("(lpHandles:?S?O,bWaitAll:?Dbool,dwMilliseconds:?Dint=!GINFINITE)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_WaitForMultipleObjects, &libwin32_WaitForMultipleObjects_f);
#ifndef LIBWIN32_KWDS_LPHANDLES_BWAITALL_DWMILLISECONDS_DEFINED
#define LIBWIN32_KWDS_LPHANDLES_BWAITALL_DWMILLISECONDS_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_lpHandles_bWaitAll_dwMilliseconds, { K(lpHandles), K(bWaitAll), K(dwMilliseconds), KEND });
#endif /* !LIBWIN32_KWDS_LPHANDLES_BWAITALL_DWMILLISECONDS_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_WaitForMultipleObjects_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *lpHandles;
	bool bWaitAll;
	DWORD dwMilliseconds = INFINITE;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_lpHandles_bWaitAll_dwMilliseconds, "ob|" UNPu32 ":WaitForMultipleObjects", &lpHandles, &bWaitAll, &dwMilliseconds))
		goto err;
	return libwin32_WaitForMultipleObjects_f_impl(lpHandles, bWaitAll, dwMilliseconds);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_WaitForMultipleObjects_f_impl(DeeObject *lpHandles, bool bWaitAll, DWORD dwMilliseconds)
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
		if unlikely(DeeNTSystem_TryGetHandle(pHandles.o[i], &hHandle)) {
			do {
				Dee_Decref(pHandles.o[i]);
			} while (++i < nCount);
			goto err_handles;
		}
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
				goto err;
			goto again;
		}
		RETURN_ERROR(dwError,
		             "Failed to wait for %" PRFuSIZ " handles (bWaitAll: %u, dwMilliseconds: %#" PRFx32 ")",
		             nCount, (unsigned int)bWaitAll, dwMilliseconds);
	}
	DBG_ALIGNMENT_ENABLE();
	return DeeInt_NewU32(dwResult);
err_handles:
	Dee_Free(pHandles.v);
err:
	return NULL;
}


/*[[[deemon import("rt.gen.dexutils").gw("SignalObjectAndWait",
      "hObjectToSignal:nt:HANDLE"
     ",hObjectToWaitOn:nt:HANDLE"
     ",dwMilliseconds:nt:DWORD=INFINITE"
     "->" MAYBE_NONE("?Dint")); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_SignalObjectAndWait_f_impl(HANDLE hObjectToSignal, HANDLE hObjectToWaitOn, DWORD dwMilliseconds);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_SignalObjectAndWait_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_SIGNALOBJECTANDWAIT_DEF { "SignalObjectAndWait", (DeeObject *)&libwin32_SignalObjectAndWait, MODSYM_FNORMAL, DOC("(hObjectToSignal:?X3?Dint?DFile?Ewin32:HANDLE,hObjectToWaitOn:?X3?Dint?DFile?Ewin32:HANDLE,dwMilliseconds:?Dint=!GINFINITE)->?Dint") },
#define LIBWIN32_SIGNALOBJECTANDWAIT_DEF_DOC(doc) { "SignalObjectAndWait", (DeeObject *)&libwin32_SignalObjectAndWait, MODSYM_FNORMAL, DOC("(hObjectToSignal:?X3?Dint?DFile?Ewin32:HANDLE,hObjectToWaitOn:?X3?Dint?DFile?Ewin32:HANDLE,dwMilliseconds:?Dint=!GINFINITE)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_SignalObjectAndWait, &libwin32_SignalObjectAndWait_f);
#ifndef LIBWIN32_KWDS_HOBJECTTOSIGNAL_HOBJECTTOWAITON_DWMILLISECONDS_DEFINED
#define LIBWIN32_KWDS_HOBJECTTOSIGNAL_HOBJECTTOWAITON_DWMILLISECONDS_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_hObjectToSignal_hObjectToWaitOn_dwMilliseconds, { K(hObjectToSignal), K(hObjectToWaitOn), K(dwMilliseconds), KEND });
#endif /* !LIBWIN32_KWDS_HOBJECTTOSIGNAL_HOBJECTTOWAITON_DWMILLISECONDS_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_SignalObjectAndWait_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	HANDLE hhObjectToSignal;
	DeeObject *hObjectToSignal;
	HANDLE hhObjectToWaitOn;
	DeeObject *hObjectToWaitOn;
	DWORD dwMilliseconds = INFINITE;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_hObjectToSignal_hObjectToWaitOn_dwMilliseconds, "oo|" UNPu32 ":SignalObjectAndWait", &hObjectToSignal, &hObjectToWaitOn, &dwMilliseconds))
		goto err;
	if (DeeNTSystem_TryGetHandle(hObjectToSignal, (void **)&hhObjectToSignal))
		goto err;
	if (DeeNTSystem_TryGetHandle(hObjectToWaitOn, (void **)&hhObjectToWaitOn))
		goto err;
	return libwin32_SignalObjectAndWait_f_impl(hhObjectToSignal, hhObjectToWaitOn, dwMilliseconds);
err:
	return NULL;
}
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
	return DeeInt_NewU32(dwResult);
err:
	return NULL;
}


/*[[[deemon import("rt.gen.dexutils").gw("Sleep",
     "dwMilliseconds:nt:DWORD"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_Sleep_f_impl(DWORD dwMilliseconds);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_Sleep_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_SLEEP_DEF { "Sleep", (DeeObject *)&libwin32_Sleep, MODSYM_FNORMAL, DOC("(dwMilliseconds:?Dint)") },
#define LIBWIN32_SLEEP_DEF_DOC(doc) { "Sleep", (DeeObject *)&libwin32_Sleep, MODSYM_FNORMAL, DOC("(dwMilliseconds:?Dint)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_Sleep, &libwin32_Sleep_f);
#ifndef LIBWIN32_KWDS_DWMILLISECONDS_DEFINED
#define LIBWIN32_KWDS_DWMILLISECONDS_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_dwMilliseconds, { K(dwMilliseconds), KEND });
#endif /* !LIBWIN32_KWDS_DWMILLISECONDS_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_Sleep_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DWORD dwMilliseconds;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_dwMilliseconds, UNPu32 ":Sleep", &dwMilliseconds))
		goto err;
	return libwin32_Sleep_f_impl(dwMilliseconds);
err:
	return NULL;
}
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
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_CreateEvent_f_impl(DeeObject *lpEventAttributes, bool bManualReset, bool bInitialState, LPCWSTR lpName);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_CreateEvent_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_CREATEEVENT_DEF { "CreateEvent", (DeeObject *)&libwin32_CreateEvent, MODSYM_FNORMAL, DOC("(lpEventAttributes?:?GSECURITY_ATTRIBUTES,bManualReset:?Dbool=!f,bInitialState:?Dbool=!f,lpName?:?Dstring)->?GHANDLE") },
#define LIBWIN32_CREATEEVENT_DEF_DOC(doc) { "CreateEvent", (DeeObject *)&libwin32_CreateEvent, MODSYM_FNORMAL, DOC("(lpEventAttributes?:?GSECURITY_ATTRIBUTES,bManualReset:?Dbool=!f,bInitialState:?Dbool=!f,lpName?:?Dstring)->?GHANDLE\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_CreateEvent, &libwin32_CreateEvent_f);
#ifndef LIBWIN32_KWDS_LPEVENTATTRIBUTES_BMANUALRESET_BINITIALSTATE_LPNAME_DEFINED
#define LIBWIN32_KWDS_LPEVENTATTRIBUTES_BMANUALRESET_BINITIALSTATE_LPNAME_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_lpEventAttributes_bManualReset_bInitialState_lpName, { K(lpEventAttributes), K(bManualReset), K(bInitialState), K(lpName), KEND });
#endif /* !LIBWIN32_KWDS_LPEVENTATTRIBUTES_BMANUALRESET_BINITIALSTATE_LPNAME_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_CreateEvent_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *lpEventAttributes = NULL;
	bool bManualReset = false;
	bool bInitialState = false;
	LPCWSTR lpName_str = NULL;
	DeeStringObject *lpName = (DeeStringObject *)Dee_None;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_lpEventAttributes_bManualReset_bInitialState_lpName, "|obbo:CreateEvent", &lpEventAttributes, &bManualReset, &bInitialState, &lpName))
		goto err;
	if (!DeeNone_Check(lpName)) {
		if (DeeObject_AssertTypeExact(lpName, &DeeString_Type))
			goto err;
		lpName_str = (LPCWSTR)DeeString_AsWide((DeeObject *)lpName);
		if unlikely(!lpName_str)
			goto err;
	}
	return libwin32_CreateEvent_f_impl(lpEventAttributes, bManualReset, bInitialState, lpName_str);
err:
	return NULL;
}
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
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_OpenEvent_f_impl(DWORD dwDesiredAccess, bool bInheritHandle, LPCWSTR lpName);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_OpenEvent_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_OPENEVENT_DEF { "OpenEvent", (DeeObject *)&libwin32_OpenEvent, MODSYM_FNORMAL, DOC("(dwDesiredAccess:?Dint,bInheritHandle:?Dbool,lpName:?Dstring)->?GHANDLE") },
#define LIBWIN32_OPENEVENT_DEF_DOC(doc) { "OpenEvent", (DeeObject *)&libwin32_OpenEvent, MODSYM_FNORMAL, DOC("(dwDesiredAccess:?Dint,bInheritHandle:?Dbool,lpName:?Dstring)->?GHANDLE\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_OpenEvent, &libwin32_OpenEvent_f);
#ifndef LIBWIN32_KWDS_DWDESIREDACCESS_BINHERITHANDLE_LPNAME_DEFINED
#define LIBWIN32_KWDS_DWDESIREDACCESS_BINHERITHANDLE_LPNAME_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_dwDesiredAccess_bInheritHandle_lpName, { K(dwDesiredAccess), K(bInheritHandle), K(lpName), KEND });
#endif /* !LIBWIN32_KWDS_DWDESIREDACCESS_BINHERITHANDLE_LPNAME_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_OpenEvent_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DWORD dwDesiredAccess;
	bool bInheritHandle;
	LPCWSTR lpName_str;
	DeeStringObject *lpName;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_dwDesiredAccess_bInheritHandle_lpName, UNPu32 "bo:OpenEvent", &dwDesiredAccess, &bInheritHandle, &lpName))
		goto err;
	if (DeeObject_AssertTypeExact(lpName, &DeeString_Type))
		goto err;
	lpName_str = (LPCWSTR)DeeString_AsWide((DeeObject *)lpName);
	if unlikely(!lpName_str)
		goto err;
	return libwin32_OpenEvent_f_impl(dwDesiredAccess, bInheritHandle, lpName_str);
err:
	return NULL;
}
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
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_ResetEvent_f_impl(HANDLE hEvent);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_ResetEvent_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_RESETEVENT_DEF { "ResetEvent", (DeeObject *)&libwin32_ResetEvent, MODSYM_FNORMAL, DOC("(hEvent:?X3?Dint?DFile?Ewin32:HANDLE)") },
#define LIBWIN32_RESETEVENT_DEF_DOC(doc) { "ResetEvent", (DeeObject *)&libwin32_ResetEvent, MODSYM_FNORMAL, DOC("(hEvent:?X3?Dint?DFile?Ewin32:HANDLE)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_ResetEvent, &libwin32_ResetEvent_f);
#ifndef LIBWIN32_KWDS_HEVENT_DEFINED
#define LIBWIN32_KWDS_HEVENT_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_hEvent, { K(hEvent), KEND });
#endif /* !LIBWIN32_KWDS_HEVENT_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_ResetEvent_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	HANDLE hhEvent;
	DeeObject *hEvent;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_hEvent, "o:ResetEvent", &hEvent))
		goto err;
	if (DeeNTSystem_TryGetHandle(hEvent, (void **)&hhEvent))
		goto err;
	return libwin32_ResetEvent_f_impl(hhEvent);
err:
	return NULL;
}
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
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_SetEvent_f_impl(HANDLE hEvent);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_SetEvent_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_SETEVENT_DEF { "SetEvent", (DeeObject *)&libwin32_SetEvent, MODSYM_FNORMAL, DOC("(hEvent:?X3?Dint?DFile?Ewin32:HANDLE)") },
#define LIBWIN32_SETEVENT_DEF_DOC(doc) { "SetEvent", (DeeObject *)&libwin32_SetEvent, MODSYM_FNORMAL, DOC("(hEvent:?X3?Dint?DFile?Ewin32:HANDLE)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_SetEvent, &libwin32_SetEvent_f);
#ifndef LIBWIN32_KWDS_HEVENT_DEFINED
#define LIBWIN32_KWDS_HEVENT_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_hEvent, { K(hEvent), KEND });
#endif /* !LIBWIN32_KWDS_HEVENT_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_SetEvent_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	HANDLE hhEvent;
	DeeObject *hEvent;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_hEvent, "o:SetEvent", &hEvent))
		goto err;
	if (DeeNTSystem_TryGetHandle(hEvent, (void **)&hhEvent))
		goto err;
	return libwin32_SetEvent_f_impl(hhEvent);
err:
	return NULL;
}
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
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_CreateMutex_f_impl(DeeObject *lpMutexAttributes, bool bInitialOwner, LPCWSTR lpName);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_CreateMutex_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_CREATEMUTEX_DEF { "CreateMutex", (DeeObject *)&libwin32_CreateMutex, MODSYM_FNORMAL, DOC("(lpMutexAttributes?:?GSECURITY_ATTRIBUTES,bInitialOwner:?Dbool=!f,lpName?:?Dstring)->?GHANDLE") },
#define LIBWIN32_CREATEMUTEX_DEF_DOC(doc) { "CreateMutex", (DeeObject *)&libwin32_CreateMutex, MODSYM_FNORMAL, DOC("(lpMutexAttributes?:?GSECURITY_ATTRIBUTES,bInitialOwner:?Dbool=!f,lpName?:?Dstring)->?GHANDLE\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_CreateMutex, &libwin32_CreateMutex_f);
#ifndef LIBWIN32_KWDS_LPMUTEXATTRIBUTES_BINITIALOWNER_LPNAME_DEFINED
#define LIBWIN32_KWDS_LPMUTEXATTRIBUTES_BINITIALOWNER_LPNAME_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_lpMutexAttributes_bInitialOwner_lpName, { K(lpMutexAttributes), K(bInitialOwner), K(lpName), KEND });
#endif /* !LIBWIN32_KWDS_LPMUTEXATTRIBUTES_BINITIALOWNER_LPNAME_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_CreateMutex_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *lpMutexAttributes = NULL;
	bool bInitialOwner = false;
	LPCWSTR lpName_str = NULL;
	DeeStringObject *lpName = (DeeStringObject *)Dee_None;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_lpMutexAttributes_bInitialOwner_lpName, "|obo:CreateMutex", &lpMutexAttributes, &bInitialOwner, &lpName))
		goto err;
	if (!DeeNone_Check(lpName)) {
		if (DeeObject_AssertTypeExact(lpName, &DeeString_Type))
			goto err;
		lpName_str = (LPCWSTR)DeeString_AsWide((DeeObject *)lpName);
		if unlikely(!lpName_str)
			goto err;
	}
	return libwin32_CreateMutex_f_impl(lpMutexAttributes, bInitialOwner, lpName_str);
err:
	return NULL;
}
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
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_OpenMutex_f_impl(DWORD dwDesiredAccess, bool bInheritHandle, LPCWSTR lpName);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_OpenMutex_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_OPENMUTEX_DEF { "OpenMutex", (DeeObject *)&libwin32_OpenMutex, MODSYM_FNORMAL, DOC("(dwDesiredAccess:?Dint,bInheritHandle:?Dbool,lpName:?Dstring)->?GHANDLE") },
#define LIBWIN32_OPENMUTEX_DEF_DOC(doc) { "OpenMutex", (DeeObject *)&libwin32_OpenMutex, MODSYM_FNORMAL, DOC("(dwDesiredAccess:?Dint,bInheritHandle:?Dbool,lpName:?Dstring)->?GHANDLE\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_OpenMutex, &libwin32_OpenMutex_f);
#ifndef LIBWIN32_KWDS_DWDESIREDACCESS_BINHERITHANDLE_LPNAME_DEFINED
#define LIBWIN32_KWDS_DWDESIREDACCESS_BINHERITHANDLE_LPNAME_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_dwDesiredAccess_bInheritHandle_lpName, { K(dwDesiredAccess), K(bInheritHandle), K(lpName), KEND });
#endif /* !LIBWIN32_KWDS_DWDESIREDACCESS_BINHERITHANDLE_LPNAME_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_OpenMutex_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DWORD dwDesiredAccess;
	bool bInheritHandle;
	LPCWSTR lpName_str;
	DeeStringObject *lpName;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_dwDesiredAccess_bInheritHandle_lpName, UNPu32 "bo:OpenMutex", &dwDesiredAccess, &bInheritHandle, &lpName))
		goto err;
	if (DeeObject_AssertTypeExact(lpName, &DeeString_Type))
		goto err;
	lpName_str = (LPCWSTR)DeeString_AsWide((DeeObject *)lpName);
	if unlikely(!lpName_str)
		goto err;
	return libwin32_OpenMutex_f_impl(dwDesiredAccess, bInheritHandle, lpName_str);
err:
	return NULL;
}
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
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_ReleaseMutex_f_impl(HANDLE hMutex);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_ReleaseMutex_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_RELEASEMUTEX_DEF { "ReleaseMutex", (DeeObject *)&libwin32_ReleaseMutex, MODSYM_FNORMAL, DOC("(hMutex:?X3?Dint?DFile?Ewin32:HANDLE)") },
#define LIBWIN32_RELEASEMUTEX_DEF_DOC(doc) { "ReleaseMutex", (DeeObject *)&libwin32_ReleaseMutex, MODSYM_FNORMAL, DOC("(hMutex:?X3?Dint?DFile?Ewin32:HANDLE)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_ReleaseMutex, &libwin32_ReleaseMutex_f);
#ifndef LIBWIN32_KWDS_HMUTEX_DEFINED
#define LIBWIN32_KWDS_HMUTEX_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_hMutex, { K(hMutex), KEND });
#endif /* !LIBWIN32_KWDS_HMUTEX_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_ReleaseMutex_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	HANDLE hhMutex;
	DeeObject *hMutex;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_hMutex, "o:ReleaseMutex", &hMutex))
		goto err;
	if (DeeNTSystem_TryGetHandle(hMutex, (void **)&hhMutex))
		goto err;
	return libwin32_ReleaseMutex_f_impl(hhMutex);
err:
	return NULL;
}
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
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_CreateSemaphore_f_impl(DeeObject *lpSemaphoreAttributes, LONG lInitialCount, LONG lMaximumCount, LPCWSTR lpName);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_CreateSemaphore_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_CREATESEMAPHORE_DEF { "CreateSemaphore", (DeeObject *)&libwin32_CreateSemaphore, MODSYM_FNORMAL, DOC("(lpSemaphoreAttributes?:?GSECURITY_ATTRIBUTES,lInitialCount:?Dint=!0,lMaximumCount:?Dint=!0x10000,lpName?:?Dstring)->?GHANDLE") },
#define LIBWIN32_CREATESEMAPHORE_DEF_DOC(doc) { "CreateSemaphore", (DeeObject *)&libwin32_CreateSemaphore, MODSYM_FNORMAL, DOC("(lpSemaphoreAttributes?:?GSECURITY_ATTRIBUTES,lInitialCount:?Dint=!0,lMaximumCount:?Dint=!0x10000,lpName?:?Dstring)->?GHANDLE\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_CreateSemaphore, &libwin32_CreateSemaphore_f);
#ifndef LIBWIN32_KWDS_LPSEMAPHOREATTRIBUTES_LINITIALCOUNT_LMAXIMUMCOUNT_LPNAME_DEFINED
#define LIBWIN32_KWDS_LPSEMAPHOREATTRIBUTES_LINITIALCOUNT_LMAXIMUMCOUNT_LPNAME_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_lpSemaphoreAttributes_lInitialCount_lMaximumCount_lpName, { K(lpSemaphoreAttributes), K(lInitialCount), K(lMaximumCount), K(lpName), KEND });
#endif /* !LIBWIN32_KWDS_LPSEMAPHOREATTRIBUTES_LINITIALCOUNT_LMAXIMUMCOUNT_LPNAME_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_CreateSemaphore_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *lpSemaphoreAttributes = NULL;
	LONG lInitialCount = 0;
	LONG lMaximumCount = 0x10000;
	LPCWSTR lpName_str = NULL;
	DeeStringObject *lpName = (DeeStringObject *)Dee_None;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_lpSemaphoreAttributes_lInitialCount_lMaximumCount_lpName, "|oI32sI32so:CreateSemaphore", &lpSemaphoreAttributes, &lInitialCount, &lMaximumCount, &lpName))
		goto err;
	if (!DeeNone_Check(lpName)) {
		if (DeeObject_AssertTypeExact(lpName, &DeeString_Type))
			goto err;
		lpName_str = (LPCWSTR)DeeString_AsWide((DeeObject *)lpName);
		if unlikely(!lpName_str)
			goto err;
	}
	return libwin32_CreateSemaphore_f_impl(lpSemaphoreAttributes, lInitialCount, lMaximumCount, lpName_str);
err:
	return NULL;
}
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
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_OpenSemaphore_f_impl(DWORD dwDesiredAccess, bool bInheritHandle, LPCWSTR lpName);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_OpenSemaphore_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_OPENSEMAPHORE_DEF { "OpenSemaphore", (DeeObject *)&libwin32_OpenSemaphore, MODSYM_FNORMAL, DOC("(dwDesiredAccess:?Dint,bInheritHandle:?Dbool,lpName:?Dstring)->?GHANDLE") },
#define LIBWIN32_OPENSEMAPHORE_DEF_DOC(doc) { "OpenSemaphore", (DeeObject *)&libwin32_OpenSemaphore, MODSYM_FNORMAL, DOC("(dwDesiredAccess:?Dint,bInheritHandle:?Dbool,lpName:?Dstring)->?GHANDLE\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_OpenSemaphore, &libwin32_OpenSemaphore_f);
#ifndef LIBWIN32_KWDS_DWDESIREDACCESS_BINHERITHANDLE_LPNAME_DEFINED
#define LIBWIN32_KWDS_DWDESIREDACCESS_BINHERITHANDLE_LPNAME_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_dwDesiredAccess_bInheritHandle_lpName, { K(dwDesiredAccess), K(bInheritHandle), K(lpName), KEND });
#endif /* !LIBWIN32_KWDS_DWDESIREDACCESS_BINHERITHANDLE_LPNAME_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_OpenSemaphore_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DWORD dwDesiredAccess;
	bool bInheritHandle;
	LPCWSTR lpName_str;
	DeeStringObject *lpName;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_dwDesiredAccess_bInheritHandle_lpName, UNPu32 "bo:OpenSemaphore", &dwDesiredAccess, &bInheritHandle, &lpName))
		goto err;
	if (DeeObject_AssertTypeExact(lpName, &DeeString_Type))
		goto err;
	lpName_str = (LPCWSTR)DeeString_AsWide((DeeObject *)lpName);
	if unlikely(!lpName_str)
		goto err;
	return libwin32_OpenSemaphore_f_impl(dwDesiredAccess, bInheritHandle, lpName_str);
err:
	return NULL;
}
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
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_ReleaseSemaphore_f_impl(HANDLE hSemaphore, LONG lReleaseCount);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_ReleaseSemaphore_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_RELEASESEMAPHORE_DEF { "ReleaseSemaphore", (DeeObject *)&libwin32_ReleaseSemaphore, MODSYM_FNORMAL, DOC("(hSemaphore:?X3?Dint?DFile?Ewin32:HANDLE,lReleaseCount:?Dint=!1)->?Dint") },
#define LIBWIN32_RELEASESEMAPHORE_DEF_DOC(doc) { "ReleaseSemaphore", (DeeObject *)&libwin32_ReleaseSemaphore, MODSYM_FNORMAL, DOC("(hSemaphore:?X3?Dint?DFile?Ewin32:HANDLE,lReleaseCount:?Dint=!1)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_ReleaseSemaphore, &libwin32_ReleaseSemaphore_f);
#ifndef LIBWIN32_KWDS_HSEMAPHORE_LRELEASECOUNT_DEFINED
#define LIBWIN32_KWDS_HSEMAPHORE_LRELEASECOUNT_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_hSemaphore_lReleaseCount, { K(hSemaphore), K(lReleaseCount), KEND });
#endif /* !LIBWIN32_KWDS_HSEMAPHORE_LRELEASECOUNT_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_ReleaseSemaphore_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	HANDLE hhSemaphore;
	DeeObject *hSemaphore;
	LONG lReleaseCount = 1;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_hSemaphore_lReleaseCount, "o|I32s:ReleaseSemaphore", &hSemaphore, &lReleaseCount))
		goto err;
	if (DeeNTSystem_TryGetHandle(hSemaphore, (void **)&hhSemaphore))
		goto err;
	return libwin32_ReleaseSemaphore_f_impl(hhSemaphore, lReleaseCount);
err:
	return NULL;
}
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
	return DeeInt_NewU32(lPreviousCount);
err:
	return NULL;
}


/*[[[deemon import("rt.gen.dexutils").gw("CreateWaitableTimer",
     "lpTimerAttributes:?GSECURITY_ATTRIBUTES=NULL"
     ",bManualReset:c:bool=false"
     ",lpTimerName:nt:LPCWSTR=NULL"
     "->" MAYBE_NONE("?GHANDLE")); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_CreateWaitableTimer_f_impl(DeeObject *lpTimerAttributes, bool bManualReset, LPCWSTR lpTimerName);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_CreateWaitableTimer_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_CREATEWAITABLETIMER_DEF { "CreateWaitableTimer", (DeeObject *)&libwin32_CreateWaitableTimer, MODSYM_FNORMAL, DOC("(lpTimerAttributes?:?GSECURITY_ATTRIBUTES,bManualReset:?Dbool=!f,lpTimerName?:?Dstring)->?GHANDLE") },
#define LIBWIN32_CREATEWAITABLETIMER_DEF_DOC(doc) { "CreateWaitableTimer", (DeeObject *)&libwin32_CreateWaitableTimer, MODSYM_FNORMAL, DOC("(lpTimerAttributes?:?GSECURITY_ATTRIBUTES,bManualReset:?Dbool=!f,lpTimerName?:?Dstring)->?GHANDLE\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_CreateWaitableTimer, &libwin32_CreateWaitableTimer_f);
#ifndef LIBWIN32_KWDS_LPTIMERATTRIBUTES_BMANUALRESET_LPTIMERNAME_DEFINED
#define LIBWIN32_KWDS_LPTIMERATTRIBUTES_BMANUALRESET_LPTIMERNAME_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_lpTimerAttributes_bManualReset_lpTimerName, { K(lpTimerAttributes), K(bManualReset), K(lpTimerName), KEND });
#endif /* !LIBWIN32_KWDS_LPTIMERATTRIBUTES_BMANUALRESET_LPTIMERNAME_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_CreateWaitableTimer_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *lpTimerAttributes = NULL;
	bool bManualReset = false;
	LPCWSTR lpTimerName_str = NULL;
	DeeStringObject *lpTimerName = (DeeStringObject *)Dee_None;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_lpTimerAttributes_bManualReset_lpTimerName, "|obo:CreateWaitableTimer", &lpTimerAttributes, &bManualReset, &lpTimerName))
		goto err;
	if (!DeeNone_Check(lpTimerName)) {
		if (DeeObject_AssertTypeExact(lpTimerName, &DeeString_Type))
			goto err;
		lpTimerName_str = (LPCWSTR)DeeString_AsWide((DeeObject *)lpTimerName);
		if unlikely(!lpTimerName_str)
			goto err;
	}
	return libwin32_CreateWaitableTimer_f_impl(lpTimerAttributes, bManualReset, lpTimerName_str);
err:
	return NULL;
}
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
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_OpenWaitableTimer_f_impl(DWORD dwDesiredAccess, bool bInheritHandle, LPCWSTR lpName);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_OpenWaitableTimer_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_OPENWAITABLETIMER_DEF { "OpenWaitableTimer", (DeeObject *)&libwin32_OpenWaitableTimer, MODSYM_FNORMAL, DOC("(dwDesiredAccess:?Dint,bInheritHandle:?Dbool,lpName:?Dstring)->?GHANDLE") },
#define LIBWIN32_OPENWAITABLETIMER_DEF_DOC(doc) { "OpenWaitableTimer", (DeeObject *)&libwin32_OpenWaitableTimer, MODSYM_FNORMAL, DOC("(dwDesiredAccess:?Dint,bInheritHandle:?Dbool,lpName:?Dstring)->?GHANDLE\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_OpenWaitableTimer, &libwin32_OpenWaitableTimer_f);
#ifndef LIBWIN32_KWDS_DWDESIREDACCESS_BINHERITHANDLE_LPNAME_DEFINED
#define LIBWIN32_KWDS_DWDESIREDACCESS_BINHERITHANDLE_LPNAME_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_dwDesiredAccess_bInheritHandle_lpName, { K(dwDesiredAccess), K(bInheritHandle), K(lpName), KEND });
#endif /* !LIBWIN32_KWDS_DWDESIREDACCESS_BINHERITHANDLE_LPNAME_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_OpenWaitableTimer_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DWORD dwDesiredAccess;
	bool bInheritHandle;
	LPCWSTR lpName_str;
	DeeStringObject *lpName;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_dwDesiredAccess_bInheritHandle_lpName, UNPu32 "bo:OpenWaitableTimer", &dwDesiredAccess, &bInheritHandle, &lpName))
		goto err;
	if (DeeObject_AssertTypeExact(lpName, &DeeString_Type))
		goto err;
	lpName_str = (LPCWSTR)DeeString_AsWide((DeeObject *)lpName);
	if unlikely(!lpName_str)
		goto err;
	return libwin32_OpenWaitableTimer_f_impl(dwDesiredAccess, bInheritHandle, lpName_str);
err:
	return NULL;
}
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
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_CancelWaitableTimer_f_impl(HANDLE hTimer);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_CancelWaitableTimer_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define LIBWIN32_CANCELWAITABLETIMER_DEF { "CancelWaitableTimer", (DeeObject *)&libwin32_CancelWaitableTimer, MODSYM_FNORMAL, DOC("(hTimer:?X3?Dint?DFile?Ewin32:HANDLE)") },
#define LIBWIN32_CANCELWAITABLETIMER_DEF_DOC(doc) { "CancelWaitableTimer", (DeeObject *)&libwin32_CancelWaitableTimer, MODSYM_FNORMAL, DOC("(hTimer:?X3?Dint?DFile?Ewin32:HANDLE)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_CancelWaitableTimer, &libwin32_CancelWaitableTimer_f);
#ifndef LIBWIN32_KWDS_HTIMER_DEFINED
#define LIBWIN32_KWDS_HTIMER_DEFINED
PRIVATE DEFINE_KWLIST(libwin32_kwds_hTimer, { K(hTimer), KEND });
#endif /* !LIBWIN32_KWDS_HTIMER_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_CancelWaitableTimer_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	HANDLE hhTimer;
	DeeObject *hTimer;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_hTimer, "o:CancelWaitableTimer", &hTimer))
		goto err;
	if (DeeNTSystem_TryGetHandle(hTimer, (void **)&hhTimer))
		goto err;
	return libwin32_CancelWaitableTimer_f_impl(hhTimer);
err:
	return NULL;
}
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











PRIVATE struct dex_symbol symbols[] = {
	{ "HANDLE", (DeeObject *)&DeeHandle_Type, MODSYM_FNORMAL },
	{ "INVALID_HANDLE_VALUE", (DeeObject *)&Dee_INVALID_HANDLE_VALUE, MODSYM_FNORMAL },
	/* TODO: Wrapper types for `SECURITY_ATTRIBUTES' and `OVERLAPPED' */
	/* TODO: Wrapper types for `WIN32_FIND_DATA' */

	/* Error-related functions. */
	LIBWIN32_GETHANDLE_DEF_DOC("Return the underlying system handle that is bound to the given object\n"
	                           " - When ?N is given, always return ${HANDLE(0)}\n"
	                           " - When an :int is given, return the result of ${get_osfhandle(ob)}\n"
	                           " - When the given object has an attribute $" Dee_fd_osfhandle_GETSET ", return ${ob." Dee_fd_osfhandle_GETSET "}\n"
	                           " - When the given object has an attribute $" Dee_fd_fileno_GETSET ", return ${get_osfhandle(ob." Dee_fd_osfhandle_GETSET ")}\n"
	                           " - When another ?GHANDLE object is given, re-return that object\n"
	                           "Note that these sames steps are also performed by all other functions taking ?GHANDLE input arguments")

	/* Error-related functions. */
	LIBWIN32_GETLASTERROR_DEF
	LIBWIN32_SETLASTERROR_DEF
	LIBWIN32_FORMATERRORMESSAGE_DEF_DOC("Return a human-readable error message associated with "
	                                    "the given @dwError (as returned by ?GGetLastError)")

	/* Handle-related functions. */
	LIBWIN32_CLOSEHANDLE_DEF
	LIBWIN32_CREATEFILE_DEF
	LIBWIN32_DUPLICATEHANDLE_DEF
	LIBWIN32_READFILE_DEF_DOC("Returns dwNumberOfBytesRead")
	LIBWIN32_WRITEFILE_DEF_DOC("Returns dwNumberOfBytesWritten")
	LIBWIN32_SETENDOFFILE_DEF
	LIBWIN32_SETFILEPOINTER_DEF_DOC("Returns the new stream position")
	LIBWIN32_GETFILETIME_DEF_DOC("Returns a tuple (CreationTime, LastAccessTime, LastWriteTime) for the given @hFile")
	LIBWIN32_SETFILETIME_DEF
	LIBWIN32_SETFILEVALIDDATA_DEF
	LIBWIN32_GETDISKFREESPACE_DEF_DOC("Returns a tuple (uSectorsPerCluster, uBytesPerSector, uNumberOfFreeClusters, uTotalNumberOfClusters)")
	LIBWIN32_GETDISKFREESPACEEX_DEF_DOC("Returns a tuple (uFreeBytesAvailableToCaller, uTotalNumberOfBytes, uTotalNumberOfFreeBytes)")
	LIBWIN32_GETTEMPPATH_DEF_DOC("Returns a string containing a temporary path")
	LIBWIN32_GETDLLDIRECTORY_DEF_DOC("Returns a string describing the windows DLL directory")
	LIBWIN32_SETDLLDIRECTORY_DEF_DOC("Set the windows DLL directory, as used when loading dynamic libraries, and as returned by ?GGetDllDirectory")
	LIBWIN32_GETFILETYPE_DEF_DOC("Return one of `FILE_TYPE_*'")
	LIBWIN32_GETFILESIZE_DEF_DOC("Return the size of the given @hFile")
	LIBWIN32_GETDRIVETYPE_DEF_DOC("Returns the type of drive of @lpRootPathName (one of `DRIVE_*')")
	LIBWIN32_GETFILEATTRIBUTES_DEF_DOC("Returns attributes for @lpFileName (set of `FILE_ATTRIBUTE_*')")
	LIBWIN32_SETFILEATTRIBUTES_DEF_DOC("Set attributes for @lpFileName to @dwFileAttributes (set of `FILE_ATTRIBUTE_*')")
	LIBWIN32_GETCOMPRESSEDFILESIZE_DEF
	LIBWIN32_FLUSHFILEBUFFERS_DEF
	LIBWIN32_GETFINALPATHNAMEBYHANDLE_DEF
	LIBWIN32_GETFILENAMEOFHANDLE_DEF_DOC("Convenience wrapper for ?GGetFinalPathNameByHandle that also supports the "
	                                     /**/ "${GetMappedFileName(MapViewOfFile(CreateFileMapping(hFile)))} workaround "
	                                     /**/ "that is required on Windows XP\n"
	                                     "Note that a similar function is provided by ?Eposix:frealpath")
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
	LIBWIN32_GETWINDOWSDIRECTORY_DEF_DOC("Returns the windows system directory ($r\"C:\\Windows\")")
	LIBWIN32_GETSYSTEMWINDOWSDIRECTORY_DEF_DOC("Returns the system windows directory ($r\"C:\\Windows\")")
	LIBWIN32_GETSYSTEMWOW64DIRECTORY_DEF_DOC("Returns the windows SysWOW64 directory ($r\"C:\\Windows\\SysWOW64\")")
	LIBWIN32_GETLOGICALDRIVESTRINGS_DEF_DOC("Returns a list of known system drives ($({ r\"C:\\\", r\"D:\\\", r\"E:\\\" }))")
	LIBWIN32_QUERYDOSDEVICE_DEF_DOC("Returns a list of DOS devices mounted under the given drive (which should be one of ?GGetLogicalDriveStrings)")

	/* DLL functions */
	LIBWIN32_GETMODULEFILENAME_DEF_DOC("Returns ?N upon error, or the name of the module")
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

	{ NULL }
};

PUBLIC struct dex DEX = {
	/* .d_symbols = */ symbols
};

DECL_END

#else /* CONFIG_HOST_WINDOWS */
#include <deemon/dex.h>

DECL_BEGIN

PRIVATE struct dex_symbol symbols[] = {
	{ NULL }
};

PUBLIC struct dex DEX = {
	/* .d_symbols = */ symbols
};

DECL_END
#endif /* CONFIG_HOST_WINDOWS */

#endif /* !GUARD_DEX_WIN32_LIBWIN32_C */
