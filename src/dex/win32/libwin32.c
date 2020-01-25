/* Copyright (c) 2018-2020 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2020 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEX_WIN32_LIBWIN32_C
#define GUARD_DEX_WIN32_LIBWIN32_C 1
#define CONFIG_BUILDING_LIBWIN32 1
#define DEE_SOURCE 1

#include "libwin32.h"
#if defined(CONFIG_HOST_WINDOWS) || defined(__DEEMON__)

#include <deemon/alloc.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/ctypes-abi.h>
#include <deemon/dex.h>
#include <deemon/file.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/objmethod.h>
#include <deemon/system.h>
#include <deemon/thread.h>
#include <deemon/tuple.h>

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
import * from _dexutils;
import * from deemon;
local orig_stdout = File.stdout;
local allDecls = [];
include("constants.def");

function wgii(name) {
	allDecls.append(name);
	gii(name);
}

wgii("FILE_READ_DATA");
wgii("FILE_LIST_DIRECTORY");
wgii("FILE_WRITE_DATA");
wgii("FILE_ADD_FILE");
wgii("FILE_APPEND_DATA");
wgii("FILE_ADD_SUBDIRECTORY");
wgii("FILE_CREATE_PIPE_INSTANCE");
wgii("FILE_READ_EA");
wgii("FILE_WRITE_EA");
wgii("FILE_EXECUTE");
wgii("FILE_TRAVERSE");
wgii("FILE_DELETE_CHILD");
wgii("FILE_READ_ATTRIBUTES");
wgii("FILE_WRITE_ATTRIBUTES");
wgii("FILE_ALL_ACCESS");
wgii("FILE_GENERIC_READ");
wgii("FILE_GENERIC_WRITE");
wgii("FILE_GENERIC_EXECUTE");
wgii("FILE_SHARE_READ");
wgii("FILE_SHARE_WRITE");
wgii("FILE_SHARE_DELETE");
wgii("FILE_ATTRIBUTE_READONLY");
wgii("FILE_ATTRIBUTE_HIDDEN");
wgii("FILE_ATTRIBUTE_SYSTEM");
wgii("FILE_ATTRIBUTE_DIRECTORY");
wgii("FILE_ATTRIBUTE_ARCHIVE");
wgii("FILE_ATTRIBUTE_DEVICE");
wgii("FILE_ATTRIBUTE_NORMAL");
wgii("FILE_ATTRIBUTE_TEMPORARY");
wgii("FILE_ATTRIBUTE_SPARSE_FILE");
wgii("FILE_ATTRIBUTE_REPARSE_POINT");
wgii("FILE_ATTRIBUTE_COMPRESSED");
wgii("FILE_ATTRIBUTE_OFFLINE");
wgii("FILE_ATTRIBUTE_NOT_CONTENT_INDEXED");
wgii("FILE_ATTRIBUTE_ENCRYPTED");
wgii("FILE_ATTRIBUTE_INTEGRITY_STREAM");
wgii("FILE_ATTRIBUTE_VIRTUAL");
wgii("FILE_ATTRIBUTE_NO_SCRUB_DATA");
wgii("FILE_ATTRIBUTE_EA");
wgii("FILE_NOTIFY_CHANGE_FILE_NAME");
wgii("FILE_NOTIFY_CHANGE_DIR_NAME");
wgii("FILE_NOTIFY_CHANGE_ATTRIBUTES");
wgii("FILE_NOTIFY_CHANGE_SIZE");
wgii("FILE_NOTIFY_CHANGE_LAST_WRITE");
wgii("FILE_NOTIFY_CHANGE_LAST_ACCESS");
wgii("FILE_NOTIFY_CHANGE_CREATION");
wgii("FILE_NOTIFY_CHANGE_SECURITY");
wgii("FILE_ACTION_ADDED");
wgii("FILE_ACTION_REMOVED");
wgii("FILE_ACTION_MODIFIED");
wgii("FILE_ACTION_RENAMED_OLD_NAME");
wgii("FILE_ACTION_RENAMED_NEW_NAME");
wgii("FILE_CASE_SENSITIVE_SEARCH");
wgii("FILE_CASE_PRESERVED_NAMES");
wgii("FILE_UNICODE_ON_DISK");
wgii("FILE_PERSISTENT_ACLS");
wgii("FILE_FILE_COMPRESSION");
wgii("FILE_VOLUME_QUOTAS");
wgii("FILE_SUPPORTS_SPARSE_FILES");
wgii("FILE_SUPPORTS_REPARSE_POINTS");
wgii("FILE_SUPPORTS_REMOTE_STORAGE");
wgii("FILE_VOLUME_IS_COMPRESSED");
wgii("FILE_SUPPORTS_OBJECT_IDS");
wgii("FILE_SUPPORTS_ENCRYPTION");
wgii("FILE_NAMED_STREAMS");
wgii("FILE_READ_ONLY_VOLUME");
wgii("FILE_SEQUENTIAL_WRITE_ONCE");
wgii("FILE_SUPPORTS_TRANSACTIONS");
wgii("FILE_SUPPORTS_HARD_LINKS");
wgii("FILE_SUPPORTS_EXTENDED_ATTRIBUTES");
wgii("FILE_SUPPORTS_OPEN_BY_FILE_ID");
wgii("FILE_SUPPORTS_USN_JOURNAL");
wgii("FILE_SUPPORTS_INTEGRITY_STREAMS");
wgii("CREATE_NEW");
wgii("CREATE_ALWAYS");
wgii("OPEN_EXISTING");
wgii("OPEN_ALWAYS");
wgii("TRUNCATE_EXISTING");
wgii("FILE_BEGIN");
wgii("FILE_CURRENT");
wgii("FILE_END");

wgii("FILE_TYPE_UNKNOWN");
wgii("FILE_TYPE_DISK");
wgii("FILE_TYPE_CHAR");
wgii("FILE_TYPE_PIPE");
wgii("FILE_TYPE_REMOTE");

wgii("DRIVE_UNKNOWN");
wgii("DRIVE_NO_ROOT_DIR");
wgii("DRIVE_REMOVABLE");
wgii("DRIVE_FIXED");
wgii("DRIVE_REMOTE");
wgii("DRIVE_CDROM");
wgii("DRIVE_RAMDISK");

wgii("PAGE_EXECUTE_READ");
wgii("PAGE_EXECUTE_READWRITE");
wgii("PAGE_EXECUTE_WRITECOPY");
wgii("PAGE_READONLY");
wgii("PAGE_READWRITE");
wgii("PAGE_WRITECOPY");
wgii("SEC_COMMIT");
wgii("SEC_IMAGE");
wgii("SEC_IMAGE_NO_EXECUTE");
wgii("SEC_LARGE_PAGES");
wgii("SEC_NOCACHE");
wgii("SEC_RESERVE");
wgii("SEC_WRITECOMBINE");

wgii("FILE_MAP_WRITE");
wgii("FILE_MAP_READ");
wgii("FILE_MAP_ALL_ACCESS");
wgii("FILE_MAP_COPY");
wgii("FILE_MAP_EXECUTE");
wgii("FILE_MAP_LARGE_PAGES");
wgii("FILE_MAP_TARGETS_INVALID");

wgii("STD_INPUT_HANDLE");
wgii("STD_OUTPUT_HANDLE");
wgii("STD_ERROR_HANDLE");

File.stdout = orig_stdout;
print "#define LIBWIN32_CONSTANTS_DEFS \\";
for (local x: allDecls) {
	print "\tLIBWIN32_",;
	print x,;
	print "_DEF \\";
}
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
	LIBWIN32_CREATE_NEW_DEF \
	LIBWIN32_CREATE_ALWAYS_DEF \
	LIBWIN32_OPEN_EXISTING_DEF \
	LIBWIN32_OPEN_ALWAYS_DEF \
	LIBWIN32_TRUNCATE_EXISTING_DEF \
	LIBWIN32_FILE_BEGIN_DEF \
	LIBWIN32_FILE_CURRENT_DEF \
	LIBWIN32_FILE_END_DEF \
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
	LIBWIN32_PAGE_EXECUTE_READ_DEF \
	LIBWIN32_PAGE_EXECUTE_READWRITE_DEF \
	LIBWIN32_PAGE_EXECUTE_WRITECOPY_DEF \
	LIBWIN32_PAGE_READONLY_DEF \
	LIBWIN32_PAGE_READWRITE_DEF \
	LIBWIN32_PAGE_WRITECOPY_DEF \
	LIBWIN32_SEC_COMMIT_DEF \
	LIBWIN32_SEC_IMAGE_DEF \
	LIBWIN32_SEC_IMAGE_NO_EXECUTE_DEF \
	LIBWIN32_SEC_LARGE_PAGES_DEF \
	LIBWIN32_SEC_NOCACHE_DEF \
	LIBWIN32_SEC_RESERVE_DEF \
	LIBWIN32_SEC_WRITECOMBINE_DEF \
	LIBWIN32_FILE_MAP_WRITE_DEF \
	LIBWIN32_FILE_MAP_READ_DEF \
	LIBWIN32_FILE_MAP_ALL_ACCESS_DEF \
	LIBWIN32_FILE_MAP_COPY_DEF \
	LIBWIN32_FILE_MAP_EXECUTE_DEF \
	LIBWIN32_FILE_MAP_LARGE_PAGES_DEF \
	LIBWIN32_FILE_MAP_TARGETS_INVALID_DEF \
	LIBWIN32_STD_INPUT_HANDLE_DEF \
	LIBWIN32_STD_OUTPUT_HANDLE_DEF \
	LIBWIN32_STD_ERROR_HANDLE_DEF \
/**/
//[[[end]]]


typedef struct {
	OBJECT_HEAD
	HANDLE  ho_handle; /* The bound handle. */
} DeeHandleObject;

#ifndef LIBWIN32_KWDS_HHANDLE_DEFINED
#define LIBWIN32_KWDS_HHANDLE_DEFINED 1
PRIVATE DEFINE_KWLIST(libwin32_kwds_hHandle, { K(hHandle), KEND });
#endif /* !LIBWIN32_KWDS_HHANDLE_DEFINED */

PRIVATE WUNUSED NONNULL((1)) int DCALL
handle_init_kw(DeeHandleObject *__restrict self,
               size_t argc, DeeObject **argv, DeeObject *kw) {
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
	/* .tp_int32 = */ (int (DCALL *)(DeeObject *__restrict,int32_t *__restrict))NULL,
	/* .tp_int64 = */ (int (DCALL *)(DeeObject *__restrict,int64_t *__restrict))&handle_int64
#else /* __SIZEOF_POINTER__ > 4 */
	/* .tp_int32 = */ (int (DCALL *)(DeeObject *__restrict,int32_t *__restrict))&handle_int32,
	/* .tp_int64 = */ (int (DCALL *)(DeeObject *__restrict,int64_t *__restrict))NULL
#endif /* __SIZEOF_POINTER__ <= 4 */
};

PRIVATE struct type_member handle_members[] = {
	TYPE_MEMBER_FIELD(DeeSysFD_HANDLE_GETSET,
	                  STRUCT_UINTPTR_T,
	                  offsetof(DeeHandleObject, ho_handle)),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
handle_str(DeeHandleObject *__restrict self) {
	return DeeString_Newf("%p", self->ho_handle);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
handle_repr(DeeHandleObject *__restrict self) {
	return DeeString_Newf("HANDLE(0x%p)", self->ho_handle);
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
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *,DeeObject *))&handle_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *,DeeObject *))&handle_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *,DeeObject *))&handle_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *,DeeObject *))&handle_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *,DeeObject *))&handle_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *,DeeObject *))&handle_ge,
};

PRIVATE DeeTypeObject DeeHandle_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "HANDLE",
	/* .tp_doc      = */ DOC("(hHandle:?Dint=!GINVALID_HANDLE_VALUE)\n"
	                         "(hHandle:?Aptr?Ectypes:void)\n"
	                         "NOTE: Passing :none for @hHandle will also "
	                         "initialize the HANDLE as :INVALID_HANDLE_VALUE"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ NULL,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ NULL,
				TYPE_FIXED_ALLOCATOR(DeeHandleObject),
				/* .tp_any_ctor_kw = */ (void *)&handle_init_kw,
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&handle_str,
		/* .tp_repr = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&handle_repr,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&handle_bool
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
	} __WHILE0
#define RETURN_ERROR_OR_FALSE  RETURN_ERROR
#define RETURN_SUCCESS_OR_TRUE return_none
#else
#define ERROR_OR_BOOL "->?Dbool"
#define MAYBE_NONE(x) "?X2" x "?N"
#define RETURN_ERROR(dwError, ...)          return_none
#define RETURN_ERROR_OR_FALSE(dwError, ...) return_false
#define RETURN_SUCCESS_OR_TRUE              return_true
#endif




/*[[[deemon import("_dexutils").gw("GetHandle", "hHandle:nt:HANDLE->?GHANDLE"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetHandle_f_impl(HANDLE hHandle);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetHandle_f(size_t argc, DeeObject **argv, DeeObject *kw);
#define LIBWIN32_GETHANDLE_DEF { "GetHandle", (DeeObject *)&libwin32_GetHandle, MODSYM_FNORMAL, DOC("(hHandle:?X3?Dint?DFile?Ewin32:HANDLE)->?GHANDLE") },
#define LIBWIN32_GETHANDLE_DEF_DOC(doc) { "GetHandle", (DeeObject *)&libwin32_GetHandle, MODSYM_FNORMAL, DOC("(hHandle:?X3?Dint?DFile?Ewin32:HANDLE)->?GHANDLE\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_GetHandle, libwin32_GetHandle_f);
#ifndef LIBWIN32_KWDS_HHANDLE_DEFINED
#define LIBWIN32_KWDS_HHANDLE_DEFINED 1
PRIVATE DEFINE_KWLIST(libwin32_kwds_hHandle, { K(hHandle), KEND });
#endif /* !LIBWIN32_KWDS_HHANDLE_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetHandle_f(size_t argc, DeeObject **argv, DeeObject *kw) {
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
//[[[end]]]
{
	return libwin32_CreateHandle(hHandle);
}

/*[[[deemon import("_dexutils").gw("GetLastError", "->?Dint"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetLastError_f_impl(void);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetLastError_f(size_t argc, DeeObject **argv);
#define LIBWIN32_GETLASTERROR_DEF { "GetLastError", (DeeObject *)&libwin32_GetLastError, MODSYM_FNORMAL, DOC("->?Dint") },
#define LIBWIN32_GETLASTERROR_DEF_DOC(doc) { "GetLastError", (DeeObject *)&libwin32_GetLastError, MODSYM_FNORMAL, DOC("->?Dint\n" doc) },
PRIVATE DEFINE_CMETHOD(libwin32_GetLastError, libwin32_GetLastError_f);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetLastError_f(size_t argc, DeeObject **argv) {
	if (DeeArg_Unpack(argc, argv, ":GetLastError"))
		goto err;
	return libwin32_GetLastError_f_impl();
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetLastError_f_impl(void)
//[[[end]]]
{
	return DeeInt_NewU32((uint32_t)GetLastError());
}

/*[[[deemon import("_dexutils").gw("SetLastError", "dwErrCode:nt:DWORD"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_SetLastError_f_impl(DWORD dwErrCode);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_SetLastError_f(size_t argc, DeeObject **argv, DeeObject *kw);
#define LIBWIN32_SETLASTERROR_DEF { "SetLastError", (DeeObject *)&libwin32_SetLastError, MODSYM_FNORMAL, DOC("(dwErrCode:?Dint)") },
#define LIBWIN32_SETLASTERROR_DEF_DOC(doc) { "SetLastError", (DeeObject *)&libwin32_SetLastError, MODSYM_FNORMAL, DOC("(dwErrCode:?Dint)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_SetLastError, libwin32_SetLastError_f);
#ifndef LIBWIN32_KWDS_DWERRCODE_DEFINED
#define LIBWIN32_KWDS_DWERRCODE_DEFINED 1
PRIVATE DEFINE_KWLIST(libwin32_kwds_dwErrCode, { K(dwErrCode), KEND });
#endif /* !LIBWIN32_KWDS_DWERRCODE_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_SetLastError_f(size_t argc, DeeObject **argv, DeeObject *kw) {
	DWORD dwErrCode;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_dwErrCode, "I32u:SetLastError", &dwErrCode))
		goto err;
	return libwin32_SetLastError_f_impl(dwErrCode);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_SetLastError_f_impl(DWORD dwErrCode)
//[[[end]]]
{
	SetLastError(dwErrCode);
	return_none;
}

/*[[[deemon import("_dexutils").gw("CloseHandle", "hObject:nt:HANDLE" ERROR_OR_BOOL); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_CloseHandle_f_impl(HANDLE hObject);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_CloseHandle_f(size_t argc, DeeObject **argv, DeeObject *kw);
#define LIBWIN32_CLOSEHANDLE_DEF { "CloseHandle", (DeeObject *)&libwin32_CloseHandle, MODSYM_FNORMAL, DOC("(hObject:?X3?Dint?DFile?Ewin32:HANDLE)") },
#define LIBWIN32_CLOSEHANDLE_DEF_DOC(doc) { "CloseHandle", (DeeObject *)&libwin32_CloseHandle, MODSYM_FNORMAL, DOC("(hObject:?X3?Dint?DFile?Ewin32:HANDLE)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_CloseHandle, libwin32_CloseHandle_f);
#ifndef LIBWIN32_KWDS_HOBJECT_DEFINED
#define LIBWIN32_KWDS_HOBJECT_DEFINED 1
PRIVATE DEFINE_KWLIST(libwin32_kwds_hObject, { K(hObject), KEND });
#endif /* !LIBWIN32_KWDS_HOBJECT_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_CloseHandle_f(size_t argc, DeeObject **argv, DeeObject *kw) {
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
//[[[end]]]
{
	BOOL bResult;
again:
	DBG_ALIGNMENT_DISABLE();
	bResult = CloseHandle(hObject);
	if (!bResult) {
		DWORD dwError = GetLastError();
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

/*[[[deemon import("_dexutils").gw("DuplicateHandle",
      "hSourceProcessHandle:nt:HANDLE"
     ",hSourceHandle:nt:HANDLE"
     ",hTargetProcessHandle:nt:HANDLE"
     ",dwDesiredAccess:nt:DWORD=0"
     ",bInheritHandle:c:bool=true"
     ",dwOptions:nt:DWORD=DUPLICATE_SAME_ACCESS"
     "->" MAYBE_NONE("?GHANDLE")
     ); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_DuplicateHandle_f_impl(HANDLE hSourceProcessHandle, HANDLE hSourceHandle, HANDLE hTargetProcessHandle, DWORD dwDesiredAccess, bool bInheritHandle, DWORD dwOptions);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_DuplicateHandle_f(size_t argc, DeeObject **argv, DeeObject *kw);
#define LIBWIN32_DUPLICATEHANDLE_DEF { "DuplicateHandle", (DeeObject *)&libwin32_DuplicateHandle, MODSYM_FNORMAL, DOC("(hSourceProcessHandle:?X3?Dint?DFile?Ewin32:HANDLE,hSourceHandle:?X3?Dint?DFile?Ewin32:HANDLE,hTargetProcessHandle:?X3?Dint?DFile?Ewin32:HANDLE,dwDesiredAccess:?Dint=!0,bInheritHandle:?Dbool=!t,dwOptions:?Dint=!GDUPLICATE_SAME_ACCESS)->?GHANDLE") },
#define LIBWIN32_DUPLICATEHANDLE_DEF_DOC(doc) { "DuplicateHandle", (DeeObject *)&libwin32_DuplicateHandle, MODSYM_FNORMAL, DOC("(hSourceProcessHandle:?X3?Dint?DFile?Ewin32:HANDLE,hSourceHandle:?X3?Dint?DFile?Ewin32:HANDLE,hTargetProcessHandle:?X3?Dint?DFile?Ewin32:HANDLE,dwDesiredAccess:?Dint=!0,bInheritHandle:?Dbool=!t,dwOptions:?Dint=!GDUPLICATE_SAME_ACCESS)->?GHANDLE\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_DuplicateHandle, libwin32_DuplicateHandle_f);
#ifndef LIBWIN32_KWDS_HSOURCEPROCESSHANDLE_HSOURCEHANDLE_HTARGETPROCESSHANDLE_DWDESIREDACCESS_BINHERITHANDLE_DWOPTIONS_DEFINED
#define LIBWIN32_KWDS_HSOURCEPROCESSHANDLE_HSOURCEHANDLE_HTARGETPROCESSHANDLE_DWDESIREDACCESS_BINHERITHANDLE_DWOPTIONS_DEFINED 1
PRIVATE DEFINE_KWLIST(libwin32_kwds_hSourceProcessHandle_hSourceHandle_hTargetProcessHandle_dwDesiredAccess_bInheritHandle_dwOptions, { K(hSourceProcessHandle), K(hSourceHandle), K(hTargetProcessHandle), K(dwDesiredAccess), K(bInheritHandle), K(dwOptions), KEND });
#endif /* !LIBWIN32_KWDS_HSOURCEPROCESSHANDLE_HSOURCEHANDLE_HTARGETPROCESSHANDLE_DWDESIREDACCESS_BINHERITHANDLE_DWOPTIONS_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_DuplicateHandle_f(size_t argc, DeeObject **argv, DeeObject *kw) {
	HANDLE hhSourceProcessHandle;
	DeeObject *hSourceProcessHandle;
	HANDLE hhSourceHandle;
	DeeObject *hSourceHandle;
	HANDLE hhTargetProcessHandle;
	DeeObject *hTargetProcessHandle;
	DWORD dwDesiredAccess = 0;
	bool bInheritHandle = true;
	DWORD dwOptions = DUPLICATE_SAME_ACCESS;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_hSourceProcessHandle_hSourceHandle_hTargetProcessHandle_dwDesiredAccess_bInheritHandle_dwOptions, "ooo|I32ubI32u:DuplicateHandle", &hSourceProcessHandle, &hSourceHandle, &hTargetProcessHandle, &dwDesiredAccess, &bInheritHandle, &dwOptions))
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
//[[[end]]]
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
		if (DeeNTSystem_IsIntr(dwError)) {
			DBG_ALIGNMENT_ENABLE();
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		DBG_ALIGNMENT_ENABLE();
		RETURN_ERROR(dwError,
		             "Failed to duplicate handle "
		             "(hSourceProcessHandle: %p, "
		             "hSourceHandle: %p, "
		             "hTargetProcessHandle: %p, "
		             "dwDesiredAccess: %#I32x, "
		             "bInheritHandle: %d, "
		             "dwOptions: %#I32x)",
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

/*[[[deemon import("_dexutils").gw("CreateFile",
      "lpFileName:nt:LPCWSTR"
     ",dwDesiredAccess:nt:DWORD=FILE_GENERIC_READ"
     ",dwShareMode:nt:DWORD=FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE"
     ",lpSecurityAttributes:?GSECURITY_ATTRIBUTES=NULL"
     ",dwCreationDisposition:nt:DWORD=OPEN_EXISTING"
     ",dwFlagsAndAttributes:nt:DWORD=FILE_ATTRIBUTE_NORMAL"
     ",hTemplateFile:nt:HANDLE=0"
     "->" MAYBE_NONE("?GHANDLE")
     ); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_CreateFile_f_impl(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, DeeObject *lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_CreateFile_f(size_t argc, DeeObject **argv, DeeObject *kw);
#define LIBWIN32_CREATEFILE_DEF { "CreateFile", (DeeObject *)&libwin32_CreateFile, MODSYM_FNORMAL, DOC("(lpFileName:?Dstring,dwDesiredAccess:?Dint=!GFILE_GENERIC_READ,dwShareMode:?Dint=FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,lpSecurityAttributes?:?GSECURITY_ATTRIBUTES,dwCreationDisposition:?Dint=!GOPEN_EXISTING,dwFlagsAndAttributes:?Dint=!GFILE_ATTRIBUTE_NORMAL,hTemplateFile:?X3?Dint?DFile?Ewin32:HANDLE=!0)->?GHANDLE") },
#define LIBWIN32_CREATEFILE_DEF_DOC(doc) { "CreateFile", (DeeObject *)&libwin32_CreateFile, MODSYM_FNORMAL, DOC("(lpFileName:?Dstring,dwDesiredAccess:?Dint=!GFILE_GENERIC_READ,dwShareMode:?Dint=FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,lpSecurityAttributes?:?GSECURITY_ATTRIBUTES,dwCreationDisposition:?Dint=!GOPEN_EXISTING,dwFlagsAndAttributes:?Dint=!GFILE_ATTRIBUTE_NORMAL,hTemplateFile:?X3?Dint?DFile?Ewin32:HANDLE=!0)->?GHANDLE\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_CreateFile, libwin32_CreateFile_f);
#ifndef LIBWIN32_KWDS_LPFILENAME_DWDESIREDACCESS_DWSHAREMODE_LPSECURITYATTRIBUTES_DWCREATIONDISPOSITION_DWFLAGSANDATTRIBUTES_HTEMPLATEFILE_DEFINED
#define LIBWIN32_KWDS_LPFILENAME_DWDESIREDACCESS_DWSHAREMODE_LPSECURITYATTRIBUTES_DWCREATIONDISPOSITION_DWFLAGSANDATTRIBUTES_HTEMPLATEFILE_DEFINED 1
PRIVATE DEFINE_KWLIST(libwin32_kwds_lpFileName_dwDesiredAccess_dwShareMode_lpSecurityAttributes_dwCreationDisposition_dwFlagsAndAttributes_hTemplateFile, { K(lpFileName), K(dwDesiredAccess), K(dwShareMode), K(lpSecurityAttributes), K(dwCreationDisposition), K(dwFlagsAndAttributes), K(hTemplateFile), KEND });
#endif /* !LIBWIN32_KWDS_LPFILENAME_DWDESIREDACCESS_DWSHAREMODE_LPSECURITYATTRIBUTES_DWCREATIONDISPOSITION_DWFLAGSANDATTRIBUTES_HTEMPLATEFILE_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_CreateFile_f(size_t argc, DeeObject **argv, DeeObject *kw) {
	LPCWSTR lpFileName_str;
	DeeStringObject *lpFileName;
	DWORD dwDesiredAccess = FILE_GENERIC_READ;
	DWORD dwShareMode = FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE;
	DeeObject *lpSecurityAttributes = NULL;
	DWORD dwCreationDisposition = OPEN_EXISTING;
	DWORD dwFlagsAndAttributes = FILE_ATTRIBUTE_NORMAL;
	HANDLE hhTemplateFile = 0;
	DeeObject *hTemplateFile = (DeeObject *)Dee_None;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_lpFileName_dwDesiredAccess_dwShareMode_lpSecurityAttributes_dwCreationDisposition_dwFlagsAndAttributes_hTemplateFile, "o|I32uI32uoI32uI32uo:CreateFile", &lpFileName, &dwDesiredAccess, &dwShareMode, &lpSecurityAttributes, &dwCreationDisposition, &dwFlagsAndAttributes, &hTemplateFile))
		goto err;
	if (DeeObject_AssertTypeExact(lpFileName, &DeeString_Type))
		goto err;
	lpFileName_str = (LPCWSTR)DeeString_AsWide((DeeObject *)lpFileName);
	if unlikely(!lpFileName_str)
		goto err;
	if (!DeeNone_Check(hTemplateFile)) {
		if (DeeNTSystem_TryGetHandle(hTemplateFile, (void **)&hhTemplateFile))
			goto err;
	}
	return libwin32_CreateFile_f_impl(lpFileName_str, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hhTemplateFile);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_CreateFile_f_impl(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, DeeObject *lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
//[[[end]]]
{
	HANDLE hResult;
	DREF DeeObject *result;
again:
	DBG_ALIGNMENT_DISABLE();
	(void)lpSecurityAttributes; /* TODO */
	hResult = CreateFileW(lpFileName,
	                      dwDesiredAccess,
	                      dwShareMode,
	                      NULL,
	                      dwCreationDisposition,
	                      dwFlagsAndAttributes,
	                      hTemplateFile);
	if unlikely(hResult == INVALID_HANDLE_VALUE) {
		DWORD dwError = GetLastError();
		if (DeeNTSystem_IsIntr(dwError)) {
			DBG_ALIGNMENT_ENABLE();
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		DBG_ALIGNMENT_ENABLE();
		RETURN_ERROR(dwError,
		             "Failed to open file %lq (dwDesiredAccess: %#I32x, dwShareMode: %#I32x, "
		             "dwCreationDisposition: %#I32x, dwFlagsAndAttributes: %#I32x, hTemplateFile: %p)",
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

/*[[[deemon import("_dexutils").gw("WriteFile",
      "hFile:nt:HANDLE"
     ",lpBuffer:obj:buffer"
     ",lpOverlapped:?GOVERLAPPED=NULL"
     "->" MAYBE_NONE("?Dint")
     ); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_WriteFile_f_impl(HANDLE hFile, DeeObject *lpBuffer, DeeObject *lpOverlapped);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_WriteFile_f(size_t argc, DeeObject **argv, DeeObject *kw);
#define LIBWIN32_WRITEFILE_DEF { "WriteFile", (DeeObject *)&libwin32_WriteFile, MODSYM_FNORMAL, DOC("(hFile:?X3?Dint?DFile?Ewin32:HANDLE,lpBuffer:?DBytes,lpOverlapped?:?GOVERLAPPED)->?Dint") },
#define LIBWIN32_WRITEFILE_DEF_DOC(doc) { "WriteFile", (DeeObject *)&libwin32_WriteFile, MODSYM_FNORMAL, DOC("(hFile:?X3?Dint?DFile?Ewin32:HANDLE,lpBuffer:?DBytes,lpOverlapped?:?GOVERLAPPED)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_WriteFile, libwin32_WriteFile_f);
#ifndef LIBWIN32_KWDS_HFILE_LPBUFFER_LPOVERLAPPED_DEFINED
#define LIBWIN32_KWDS_HFILE_LPBUFFER_LPOVERLAPPED_DEFINED 1
PRIVATE DEFINE_KWLIST(libwin32_kwds_hFile_lpBuffer_lpOverlapped, { K(hFile), K(lpBuffer), K(lpOverlapped), KEND });
#endif /* !LIBWIN32_KWDS_HFILE_LPBUFFER_LPOVERLAPPED_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_WriteFile_f(size_t argc, DeeObject **argv, DeeObject *kw) {
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
//[[[end]]]
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
		DWORD dwError = GetLastError();
		if (DeeNTSystem_IsIntr(dwError)) {
			DBG_ALIGNMENT_ENABLE();
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		DBG_ALIGNMENT_ENABLE();
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



/*[[[deemon import("_dexutils").gw("ReadFile",
      "hFile:nt:HANDLE"
     ",lpBuffer:obj:buffer"
     ",lpOverlapped:?GOVERLAPPED=NULL"
     "->" MAYBE_NONE("?Dint")
     ); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_ReadFile_f_impl(HANDLE hFile, DeeObject *lpBuffer, DeeObject *lpOverlapped);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_ReadFile_f(size_t argc, DeeObject **argv, DeeObject *kw);
#define LIBWIN32_READFILE_DEF { "ReadFile", (DeeObject *)&libwin32_ReadFile, MODSYM_FNORMAL, DOC("(hFile:?X3?Dint?DFile?Ewin32:HANDLE,lpBuffer:?DBytes,lpOverlapped?:?GOVERLAPPED)->?Dint") },
#define LIBWIN32_READFILE_DEF_DOC(doc) { "ReadFile", (DeeObject *)&libwin32_ReadFile, MODSYM_FNORMAL, DOC("(hFile:?X3?Dint?DFile?Ewin32:HANDLE,lpBuffer:?DBytes,lpOverlapped?:?GOVERLAPPED)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_ReadFile, libwin32_ReadFile_f);
#ifndef LIBWIN32_KWDS_HFILE_LPBUFFER_LPOVERLAPPED_DEFINED
#define LIBWIN32_KWDS_HFILE_LPBUFFER_LPOVERLAPPED_DEFINED 1
PRIVATE DEFINE_KWLIST(libwin32_kwds_hFile_lpBuffer_lpOverlapped, { K(hFile), K(lpBuffer), K(lpOverlapped), KEND });
#endif /* !LIBWIN32_KWDS_HFILE_LPBUFFER_LPOVERLAPPED_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_ReadFile_f(size_t argc, DeeObject **argv, DeeObject *kw) {
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
//[[[end]]]
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
		DWORD dwError = GetLastError();
		if (DeeNTSystem_IsIntr(dwError)) {
			DBG_ALIGNMENT_ENABLE();
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		DBG_ALIGNMENT_ENABLE();
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


/*[[[deemon import("_dexutils").gw("CreateDirectory", "lpPathName:nt:LPCWSTR,lpSecurityAttributes:?GSECURITY_ATTRIBUTES=NULL" ERROR_OR_BOOL); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_CreateDirectory_f_impl(LPCWSTR lpPathName, DeeObject *lpSecurityAttributes);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_CreateDirectory_f(size_t argc, DeeObject **argv, DeeObject *kw);
#define LIBWIN32_CREATEDIRECTORY_DEF { "CreateDirectory", (DeeObject *)&libwin32_CreateDirectory, MODSYM_FNORMAL, DOC("(lpPathName:?Dstring,lpSecurityAttributes?:?GSECURITY_ATTRIBUTES)") },
#define LIBWIN32_CREATEDIRECTORY_DEF_DOC(doc) { "CreateDirectory", (DeeObject *)&libwin32_CreateDirectory, MODSYM_FNORMAL, DOC("(lpPathName:?Dstring,lpSecurityAttributes?:?GSECURITY_ATTRIBUTES)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_CreateDirectory, libwin32_CreateDirectory_f);
#ifndef LIBWIN32_KWDS_LPPATHNAME_LPSECURITYATTRIBUTES_DEFINED
#define LIBWIN32_KWDS_LPPATHNAME_LPSECURITYATTRIBUTES_DEFINED 1
PRIVATE DEFINE_KWLIST(libwin32_kwds_lpPathName_lpSecurityAttributes, { K(lpPathName), K(lpSecurityAttributes), KEND });
#endif /* !LIBWIN32_KWDS_LPPATHNAME_LPSECURITYATTRIBUTES_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_CreateDirectory_f(size_t argc, DeeObject **argv, DeeObject *kw) {
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
//[[[end]]]
{
	BOOL bResult;
again:
	DBG_ALIGNMENT_DISABLE();
	(void)lpSecurityAttributes; /* TODO */
	bResult = CreateDirectoryW(lpPathName, NULL);
	if (!bResult) {
		DWORD dwError = GetLastError();
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

/*[[[deemon import("_dexutils").gw("RemoveDirectory", "lpPathName:nt:LPCWSTR" ERROR_OR_BOOL); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_RemoveDirectory_f_impl(LPCWSTR lpPathName);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_RemoveDirectory_f(size_t argc, DeeObject **argv, DeeObject *kw);
#define LIBWIN32_REMOVEDIRECTORY_DEF { "RemoveDirectory", (DeeObject *)&libwin32_RemoveDirectory, MODSYM_FNORMAL, DOC("(lpPathName:?Dstring)") },
#define LIBWIN32_REMOVEDIRECTORY_DEF_DOC(doc) { "RemoveDirectory", (DeeObject *)&libwin32_RemoveDirectory, MODSYM_FNORMAL, DOC("(lpPathName:?Dstring)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_RemoveDirectory, libwin32_RemoveDirectory_f);
#ifndef LIBWIN32_KWDS_LPPATHNAME_DEFINED
#define LIBWIN32_KWDS_LPPATHNAME_DEFINED 1
PRIVATE DEFINE_KWLIST(libwin32_kwds_lpPathName, { K(lpPathName), KEND });
#endif /* !LIBWIN32_KWDS_LPPATHNAME_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_RemoveDirectory_f(size_t argc, DeeObject **argv, DeeObject *kw) {
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
//[[[end]]]
{
	BOOL bResult;
again:
	DBG_ALIGNMENT_DISABLE();
	bResult = RemoveDirectoryW(lpPathName);
	if (!bResult) {
		DWORD dwError = GetLastError();
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

/*[[[deemon import("_dexutils").gw("DeleteFile", "lpFileName:nt:LPCWSTR" ERROR_OR_BOOL); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_DeleteFile_f_impl(LPCWSTR lpFileName);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_DeleteFile_f(size_t argc, DeeObject **argv, DeeObject *kw);
#define LIBWIN32_DELETEFILE_DEF { "DeleteFile", (DeeObject *)&libwin32_DeleteFile, MODSYM_FNORMAL, DOC("(lpFileName:?Dstring)") },
#define LIBWIN32_DELETEFILE_DEF_DOC(doc) { "DeleteFile", (DeeObject *)&libwin32_DeleteFile, MODSYM_FNORMAL, DOC("(lpFileName:?Dstring)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_DeleteFile, libwin32_DeleteFile_f);
#ifndef LIBWIN32_KWDS_LPFILENAME_DEFINED
#define LIBWIN32_KWDS_LPFILENAME_DEFINED 1
PRIVATE DEFINE_KWLIST(libwin32_kwds_lpFileName, { K(lpFileName), KEND });
#endif /* !LIBWIN32_KWDS_LPFILENAME_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_DeleteFile_f(size_t argc, DeeObject **argv, DeeObject *kw) {
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
//[[[end]]]
{
	BOOL bResult;
again:
	DBG_ALIGNMENT_DISABLE();
	bResult = DeleteFileW(lpFileName);
	if (!bResult) {
		DWORD dwError = GetLastError();
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

/*[[[deemon import("_dexutils").gw("SetEndOfFile", "hFile:nt:HANDLE" ERROR_OR_BOOL); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_SetEndOfFile_f_impl(HANDLE hFile);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_SetEndOfFile_f(size_t argc, DeeObject **argv, DeeObject *kw);
#define LIBWIN32_SETENDOFFILE_DEF { "SetEndOfFile", (DeeObject *)&libwin32_SetEndOfFile, MODSYM_FNORMAL, DOC("(hFile:?X3?Dint?DFile?Ewin32:HANDLE)") },
#define LIBWIN32_SETENDOFFILE_DEF_DOC(doc) { "SetEndOfFile", (DeeObject *)&libwin32_SetEndOfFile, MODSYM_FNORMAL, DOC("(hFile:?X3?Dint?DFile?Ewin32:HANDLE)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_SetEndOfFile, libwin32_SetEndOfFile_f);
#ifndef LIBWIN32_KWDS_HFILE_DEFINED
#define LIBWIN32_KWDS_HFILE_DEFINED 1
PRIVATE DEFINE_KWLIST(libwin32_kwds_hFile, { K(hFile), KEND });
#endif /* !LIBWIN32_KWDS_HFILE_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_SetEndOfFile_f(size_t argc, DeeObject **argv, DeeObject *kw) {
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
//[[[end]]]
{
	BOOL bResult;
again:
	DBG_ALIGNMENT_DISABLE();
	bResult = SetEndOfFile(hFile);
	if (!bResult) {
		DWORD dwError;
		dwError = GetLastError();
		if (DeeNTSystem_IsIntr(dwError)) {
			DBG_ALIGNMENT_ENABLE();
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

/*[[[deemon import("_dexutils").gw("SetFileAttributesW", "lpFileName:nt:LPCWSTR,dwFileAttributes:nt:DWORD" ERROR_OR_BOOL); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_SetFileAttributesW_f_impl(LPCWSTR lpFileName, DWORD dwFileAttributes);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_SetFileAttributesW_f(size_t argc, DeeObject **argv, DeeObject *kw);
#define LIBWIN32_SETFILEATTRIBUTESW_DEF { "SetFileAttributesW", (DeeObject *)&libwin32_SetFileAttributesW, MODSYM_FNORMAL, DOC("(lpFileName:?Dstring,dwFileAttributes:?Dint)") },
#define LIBWIN32_SETFILEATTRIBUTESW_DEF_DOC(doc) { "SetFileAttributesW", (DeeObject *)&libwin32_SetFileAttributesW, MODSYM_FNORMAL, DOC("(lpFileName:?Dstring,dwFileAttributes:?Dint)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_SetFileAttributesW, libwin32_SetFileAttributesW_f);
#ifndef LIBWIN32_KWDS_LPFILENAME_DWFILEATTRIBUTES_DEFINED
#define LIBWIN32_KWDS_LPFILENAME_DWFILEATTRIBUTES_DEFINED 1
PRIVATE DEFINE_KWLIST(libwin32_kwds_lpFileName_dwFileAttributes, { K(lpFileName), K(dwFileAttributes), KEND });
#endif /* !LIBWIN32_KWDS_LPFILENAME_DWFILEATTRIBUTES_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_SetFileAttributesW_f(size_t argc, DeeObject **argv, DeeObject *kw) {
	LPCWSTR lpFileName_str;
	DeeStringObject *lpFileName;
	DWORD dwFileAttributes;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_lpFileName_dwFileAttributes, "oI32u:SetFileAttributesW", &lpFileName, &dwFileAttributes))
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
//[[[end]]]
{
	BOOL bResult;
again:
	DBG_ALIGNMENT_DISABLE();
	bResult = SetFileAttributesW(lpFileName, dwFileAttributes);
	if (!bResult) {
		DWORD dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(dwError)) {
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		RETURN_ERROR_OR_FALSE(dwError,
		                      "Failed to set file attribute of %lq to %#I32x",
		                      lpFileName, dwFileAttributes);
	}
	DBG_ALIGNMENT_ENABLE();
	RETURN_SUCCESS_OR_TRUE;
err:
	return NULL;
}


/*[[[deemon import("_dexutils").gw("SetFilePointer", "hFile:nt:HANDLE,lDistanceToMove:I64d,dwMoveMethod:nt:DWORD=FILE_BEGIN->?Dint"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_SetFilePointer_f_impl(HANDLE hFile, int64_t lDistanceToMove, DWORD dwMoveMethod);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_SetFilePointer_f(size_t argc, DeeObject **argv, DeeObject *kw);
#define LIBWIN32_SETFILEPOINTER_DEF { "SetFilePointer", (DeeObject *)&libwin32_SetFilePointer, MODSYM_FNORMAL, DOC("(hFile:?X3?Dint?DFile?Ewin32:HANDLE,lDistanceToMove:?Dint,dwMoveMethod:?Dint=!GFILE_BEGIN)->?Dint") },
#define LIBWIN32_SETFILEPOINTER_DEF_DOC(doc) { "SetFilePointer", (DeeObject *)&libwin32_SetFilePointer, MODSYM_FNORMAL, DOC("(hFile:?X3?Dint?DFile?Ewin32:HANDLE,lDistanceToMove:?Dint,dwMoveMethod:?Dint=!GFILE_BEGIN)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_SetFilePointer, libwin32_SetFilePointer_f);
#ifndef LIBWIN32_KWDS_HFILE_LDISTANCETOMOVE_DWMOVEMETHOD_DEFINED
#define LIBWIN32_KWDS_HFILE_LDISTANCETOMOVE_DWMOVEMETHOD_DEFINED 1
PRIVATE DEFINE_KWLIST(libwin32_kwds_hFile_lDistanceToMove_dwMoveMethod, { K(hFile), K(lDistanceToMove), K(dwMoveMethod), KEND });
#endif /* !LIBWIN32_KWDS_HFILE_LDISTANCETOMOVE_DWMOVEMETHOD_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_SetFilePointer_f(size_t argc, DeeObject **argv, DeeObject *kw) {
	HANDLE hhFile;
	DeeObject *hFile;
	int64_t lDistanceToMove;
	DWORD dwMoveMethod = FILE_BEGIN;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_hFile_lDistanceToMove_dwMoveMethod, "oI64d|I32u:SetFilePointer", &hFile, &lDistanceToMove, &dwMoveMethod))
		goto err;
	if (DeeNTSystem_TryGetHandle(hFile, (void **)&hhFile))
		goto err;
	return libwin32_SetFilePointer_f_impl(hhFile, lDistanceToMove, dwMoveMethod);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_SetFilePointer_f_impl(HANDLE hFile, int64_t lDistanceToMove, DWORD dwMoveMethod)
//[[[end]]]
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
		DWORD dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (dwError != NO_ERROR) {
			if (DeeNTSystem_IsIntr(dwError)) {
				if (DeeThread_CheckInterrupt())
					goto err;
				goto again;
			}
			DBG_ALIGNMENT_ENABLE();
			RETURN_ERROR(dwError,
			             "Failed to seek file %p (offset: %I64d, whence: %I32u)",
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


/*[[[deemon import("_dexutils").gw("GetFileTime", "hFile:nt:HANDLE->" MAYBE_NONE("?T3?Dint?Dint?Dint")); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetFileTime_f_impl(HANDLE hFile);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetFileTime_f(size_t argc, DeeObject **argv, DeeObject *kw);
#define LIBWIN32_GETFILETIME_DEF { "GetFileTime", (DeeObject *)&libwin32_GetFileTime, MODSYM_FNORMAL, DOC("(hFile:?X3?Dint?DFile?Ewin32:HANDLE)->?T3?Dint?Dint?Dint") },
#define LIBWIN32_GETFILETIME_DEF_DOC(doc) { "GetFileTime", (DeeObject *)&libwin32_GetFileTime, MODSYM_FNORMAL, DOC("(hFile:?X3?Dint?DFile?Ewin32:HANDLE)->?T3?Dint?Dint?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_GetFileTime, libwin32_GetFileTime_f);
#ifndef LIBWIN32_KWDS_HFILE_DEFINED
#define LIBWIN32_KWDS_HFILE_DEFINED 1
PRIVATE DEFINE_KWLIST(libwin32_kwds_hFile, { K(hFile), KEND });
#endif /* !LIBWIN32_KWDS_HFILE_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetFileTime_f(size_t argc, DeeObject **argv, DeeObject *kw) {
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
//[[[end]]]
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
		DWORD dwError = GetLastError();
		if (DeeNTSystem_IsIntr(dwError)) {
			DBG_ALIGNMENT_ENABLE();
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		DBG_ALIGNMENT_ENABLE();
		RETURN_ERROR(dwError,
		             "Failed to lookup file times of %p",
		             hFile);
	}
	DBG_ALIGNMENT_ENABLE();
	return DeeTuple_Newf(DEE_FMT_UINT64
	                     DEE_FMT_UINT64
	                     DEE_FMT_UINT64,
	                     ftCreationTime.u64,
	                     ftLastAccessTime.u64,
	                     ftLastWriteTime.u64);
err:
	return NULL;
}



/*[[[deemon import("_dexutils").gw("SetFileTime", "hFile:nt:HANDLE,lpCreationTime:?Dint=NULL,lpLastAccessTime:?Dint=NULL,lpLastWriteTime:?Dint=NULL" ERROR_OR_BOOL); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_SetFileTime_f_impl(HANDLE hFile, DeeObject *lpCreationTime, DeeObject *lpLastAccessTime, DeeObject *lpLastWriteTime);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_SetFileTime_f(size_t argc, DeeObject **argv, DeeObject *kw);
#define LIBWIN32_SETFILETIME_DEF { "SetFileTime", (DeeObject *)&libwin32_SetFileTime, MODSYM_FNORMAL, DOC("(hFile:?X3?Dint?DFile?Ewin32:HANDLE,lpCreationTime?:?Dint,lpLastAccessTime?:?Dint,lpLastWriteTime?:?Dint)") },
#define LIBWIN32_SETFILETIME_DEF_DOC(doc) { "SetFileTime", (DeeObject *)&libwin32_SetFileTime, MODSYM_FNORMAL, DOC("(hFile:?X3?Dint?DFile?Ewin32:HANDLE,lpCreationTime?:?Dint,lpLastAccessTime?:?Dint,lpLastWriteTime?:?Dint)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_SetFileTime, libwin32_SetFileTime_f);
#ifndef LIBWIN32_KWDS_HFILE_LPCREATIONTIME_LPLASTACCESSTIME_LPLASTWRITETIME_DEFINED
#define LIBWIN32_KWDS_HFILE_LPCREATIONTIME_LPLASTACCESSTIME_LPLASTWRITETIME_DEFINED 1
PRIVATE DEFINE_KWLIST(libwin32_kwds_hFile_lpCreationTime_lpLastAccessTime_lpLastWriteTime, { K(hFile), K(lpCreationTime), K(lpLastAccessTime), K(lpLastWriteTime), KEND });
#endif /* !LIBWIN32_KWDS_HFILE_LPCREATIONTIME_LPLASTACCESSTIME_LPLASTWRITETIME_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_SetFileTime_f(size_t argc, DeeObject **argv, DeeObject *kw) {
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
//[[[end]]]
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
		DWORD dwError = GetLastError();
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



/*[[[deemon import("_dexutils").gw("SetFileValidData", "hFile:nt:HANDLE,ValidDataLength:I64u" ERROR_OR_BOOL); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_SetFileValidData_f_impl(HANDLE hFile, uint64_t ValidDataLength);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_SetFileValidData_f(size_t argc, DeeObject **argv, DeeObject *kw);
#define LIBWIN32_SETFILEVALIDDATA_DEF { "SetFileValidData", (DeeObject *)&libwin32_SetFileValidData, MODSYM_FNORMAL, DOC("(hFile:?X3?Dint?DFile?Ewin32:HANDLE,ValidDataLength:?Dint)") },
#define LIBWIN32_SETFILEVALIDDATA_DEF_DOC(doc) { "SetFileValidData", (DeeObject *)&libwin32_SetFileValidData, MODSYM_FNORMAL, DOC("(hFile:?X3?Dint?DFile?Ewin32:HANDLE,ValidDataLength:?Dint)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_SetFileValidData, libwin32_SetFileValidData_f);
#ifndef LIBWIN32_KWDS_HFILE_VALIDDATALENGTH_DEFINED
#define LIBWIN32_KWDS_HFILE_VALIDDATALENGTH_DEFINED 1
PRIVATE DEFINE_KWLIST(libwin32_kwds_hFile_ValidDataLength, { K(hFile), K(ValidDataLength), KEND });
#endif /* !LIBWIN32_KWDS_HFILE_VALIDDATALENGTH_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_SetFileValidData_f(size_t argc, DeeObject **argv, DeeObject *kw) {
	HANDLE hhFile;
	DeeObject *hFile;
	uint64_t ValidDataLength;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_hFile_ValidDataLength, "oI64u:SetFileValidData", &hFile, &ValidDataLength))
		goto err;
	if (DeeNTSystem_TryGetHandle(hFile, (void **)&hhFile))
		goto err;
	return libwin32_SetFileValidData_f_impl(hhFile, ValidDataLength);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_SetFileValidData_f_impl(HANDLE hFile, uint64_t ValidDataLength)
//[[[end]]]
{
	BOOL bResult;
again:
	DBG_ALIGNMENT_DISABLE();
	bResult = SetFileValidData(hFile, ValidDataLength);
	if (!bResult) {
		DWORD dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(dwError)) {
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		RETURN_ERROR_OR_FALSE(dwError,
		                      "Failed to set valid file data for %p to %I64u",
		                      hFile, ValidDataLength);
	}
	DBG_ALIGNMENT_ENABLE();
	RETURN_SUCCESS_OR_TRUE;
err:
	return NULL;
}


/*[[[deemon import("_dexutils").gw("GetTempPath", "->" MAYBE_NONE("?Dstring")); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetTempPath_f_impl(void);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetTempPath_f(size_t argc, DeeObject **argv);
#define LIBWIN32_GETTEMPPATH_DEF { "GetTempPath", (DeeObject *)&libwin32_GetTempPath, MODSYM_FNORMAL, DOC("->?Dstring") },
#define LIBWIN32_GETTEMPPATH_DEF_DOC(doc) { "GetTempPath", (DeeObject *)&libwin32_GetTempPath, MODSYM_FNORMAL, DOC("->?Dstring\n" doc) },
PRIVATE DEFINE_CMETHOD(libwin32_GetTempPath, libwin32_GetTempPath_f);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetTempPath_f(size_t argc, DeeObject **argv) {
	if (DeeArg_Unpack(argc, argv, ":GetTempPath"))
		goto err;
	return libwin32_GetTempPath_f_impl();
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetTempPath_f_impl(void)
//[[[end]]]
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

/*[[[deemon import("_dexutils").gw("GetDllDirectory", "->" MAYBE_NONE("?Dstring")); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetDllDirectory_f_impl(void);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetDllDirectory_f(size_t argc, DeeObject **argv);
#define LIBWIN32_GETDLLDIRECTORY_DEF { "GetDllDirectory", (DeeObject *)&libwin32_GetDllDirectory, MODSYM_FNORMAL, DOC("->?Dstring") },
#define LIBWIN32_GETDLLDIRECTORY_DEF_DOC(doc) { "GetDllDirectory", (DeeObject *)&libwin32_GetDllDirectory, MODSYM_FNORMAL, DOC("->?Dstring\n" doc) },
PRIVATE DEFINE_CMETHOD(libwin32_GetDllDirectory, libwin32_GetDllDirectory_f);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetDllDirectory_f(size_t argc, DeeObject **argv) {
	if (DeeArg_Unpack(argc, argv, ":GetDllDirectory"))
		goto err;
	return libwin32_GetDllDirectory_f_impl();
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetDllDirectory_f_impl(void)
//[[[end]]]
{
	LPWSTR lpBuffer, lpNewBuffer;
	DWORD dwBufSize = PATH_MAX, dwError;
	typedef DWORD(WINAPI * LPGETDLLDIRECTORYW)(DWORD nBufferLength, LPWSTR lpBuffer);
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

/*[[[deemon import("_dexutils").gw("SetDllDirectory", "lpPathName:nt:LPCWSTR" ERROR_OR_BOOL); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_SetDllDirectory_f_impl(LPCWSTR lpPathName);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_SetDllDirectory_f(size_t argc, DeeObject **argv, DeeObject *kw);
#define LIBWIN32_SETDLLDIRECTORY_DEF { "SetDllDirectory", (DeeObject *)&libwin32_SetDllDirectory, MODSYM_FNORMAL, DOC("(lpPathName:?Dstring)") },
#define LIBWIN32_SETDLLDIRECTORY_DEF_DOC(doc) { "SetDllDirectory", (DeeObject *)&libwin32_SetDllDirectory, MODSYM_FNORMAL, DOC("(lpPathName:?Dstring)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_SetDllDirectory, libwin32_SetDllDirectory_f);
#ifndef LIBWIN32_KWDS_LPPATHNAME_DEFINED
#define LIBWIN32_KWDS_LPPATHNAME_DEFINED 1
PRIVATE DEFINE_KWLIST(libwin32_kwds_lpPathName, { K(lpPathName), KEND });
#endif /* !LIBWIN32_KWDS_LPPATHNAME_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_SetDllDirectory_f(size_t argc, DeeObject **argv, DeeObject *kw) {
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
//[[[end]]]
{
	BOOL bResult;
	typedef BOOL(WINAPI * LPSETDLLDIRECTORYW)(LPCWSTR lpPathName);
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
		DWORD dwError = GetLastError();
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

/*[[[deemon import("_dexutils").gw("GetDiskFreeSpace", "lpRootPathName:nt:LPCWSTR->" MAYBE_NONE("?T4?Dint?Dint?Dint?Dint")); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetDiskFreeSpace_f_impl(LPCWSTR lpRootPathName);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetDiskFreeSpace_f(size_t argc, DeeObject **argv, DeeObject *kw);
#define LIBWIN32_GETDISKFREESPACE_DEF { "GetDiskFreeSpace", (DeeObject *)&libwin32_GetDiskFreeSpace, MODSYM_FNORMAL, DOC("(lpRootPathName:?Dstring)->?T4?Dint?Dint?Dint?Dint") },
#define LIBWIN32_GETDISKFREESPACE_DEF_DOC(doc) { "GetDiskFreeSpace", (DeeObject *)&libwin32_GetDiskFreeSpace, MODSYM_FNORMAL, DOC("(lpRootPathName:?Dstring)->?T4?Dint?Dint?Dint?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_GetDiskFreeSpace, libwin32_GetDiskFreeSpace_f);
#ifndef LIBWIN32_KWDS_LPROOTPATHNAME_DEFINED
#define LIBWIN32_KWDS_LPROOTPATHNAME_DEFINED 1
PRIVATE DEFINE_KWLIST(libwin32_kwds_lpRootPathName, { K(lpRootPathName), KEND });
#endif /* !LIBWIN32_KWDS_LPROOTPATHNAME_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetDiskFreeSpace_f(size_t argc, DeeObject **argv, DeeObject *kw) {
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
//[[[end]]]
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
		DWORD dwError = GetLastError();
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
	return DeeTuple_Newf(DEE_FMT_UINT32
	                     DEE_FMT_UINT32
	                     DEE_FMT_UINT32
	                     DEE_FMT_UINT32,
	                     dwSectorsPerCluster,
	                     dwBytesPerSector,
	                     dwNumberOfFreeClusters,
	                     dwTotalNumberOfClusters);
err:
	return NULL;
}



/*[[[deemon import("_dexutils").gw("GetDiskFreeSpaceEx", "lpDirectoryName:nt:LPCWSTR->" MAYBE_NONE("?T3?Dint?Dint?Dint")); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetDiskFreeSpaceEx_f_impl(LPCWSTR lpDirectoryName);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetDiskFreeSpaceEx_f(size_t argc, DeeObject **argv, DeeObject *kw);
#define LIBWIN32_GETDISKFREESPACEEX_DEF { "GetDiskFreeSpaceEx", (DeeObject *)&libwin32_GetDiskFreeSpaceEx, MODSYM_FNORMAL, DOC("(lpDirectoryName:?Dstring)->?T3?Dint?Dint?Dint") },
#define LIBWIN32_GETDISKFREESPACEEX_DEF_DOC(doc) { "GetDiskFreeSpaceEx", (DeeObject *)&libwin32_GetDiskFreeSpaceEx, MODSYM_FNORMAL, DOC("(lpDirectoryName:?Dstring)->?T3?Dint?Dint?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_GetDiskFreeSpaceEx, libwin32_GetDiskFreeSpaceEx_f);
#ifndef LIBWIN32_KWDS_LPDIRECTORYNAME_DEFINED
#define LIBWIN32_KWDS_LPDIRECTORYNAME_DEFINED 1
PRIVATE DEFINE_KWLIST(libwin32_kwds_lpDirectoryName, { K(lpDirectoryName), KEND });
#endif /* !LIBWIN32_KWDS_LPDIRECTORYNAME_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetDiskFreeSpaceEx_f(size_t argc, DeeObject **argv, DeeObject *kw) {
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
//[[[end]]]
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
		DWORD dwError = GetLastError();
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
	return DeeTuple_Newf(DEE_FMT_UINT64
	                     DEE_FMT_UINT64
	                     DEE_FMT_UINT64,
	                     uFreeBytesAvailableToCaller.QuadPart,
	                     uTotalNumberOfBytes.QuadPart,
	                     uTotalNumberOfFreeBytes.QuadPart);
err:
	return NULL;
}


/*[[[deemon import("_dexutils").gw("GetDriveType", "lpRootPathName:nt:LPCWSTR->" MAYBE_NONE("?Dint")); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetDriveType_f_impl(LPCWSTR lpRootPathName);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetDriveType_f(size_t argc, DeeObject **argv, DeeObject *kw);
#define LIBWIN32_GETDRIVETYPE_DEF { "GetDriveType", (DeeObject *)&libwin32_GetDriveType, MODSYM_FNORMAL, DOC("(lpRootPathName:?Dstring)->?Dint") },
#define LIBWIN32_GETDRIVETYPE_DEF_DOC(doc) { "GetDriveType", (DeeObject *)&libwin32_GetDriveType, MODSYM_FNORMAL, DOC("(lpRootPathName:?Dstring)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_GetDriveType, libwin32_GetDriveType_f);
#ifndef LIBWIN32_KWDS_LPROOTPATHNAME_DEFINED
#define LIBWIN32_KWDS_LPROOTPATHNAME_DEFINED 1
PRIVATE DEFINE_KWLIST(libwin32_kwds_lpRootPathName, { K(lpRootPathName), KEND });
#endif /* !LIBWIN32_KWDS_LPROOTPATHNAME_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetDriveType_f(size_t argc, DeeObject **argv, DeeObject *kw) {
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
//[[[end]]]
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
	}
	return DeeInt_NewUInt(uResult);
err:
	return NULL;
}




/*[[[deemon import("_dexutils").gw("GetModuleFileName", "hModule:nt:HANDLE->" MAYBE_NONE("?Dstring")); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetModuleFileName_f_impl(HANDLE hModule);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetModuleFileName_f(size_t argc, DeeObject **argv, DeeObject *kw);
#define LIBWIN32_GETMODULEFILENAME_DEF { "GetModuleFileName", (DeeObject *)&libwin32_GetModuleFileName, MODSYM_FNORMAL, DOC("(hModule:?X3?Dint?DFile?Ewin32:HANDLE)->?Dstring") },
#define LIBWIN32_GETMODULEFILENAME_DEF_DOC(doc) { "GetModuleFileName", (DeeObject *)&libwin32_GetModuleFileName, MODSYM_FNORMAL, DOC("(hModule:?X3?Dint?DFile?Ewin32:HANDLE)->?Dstring\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_GetModuleFileName, libwin32_GetModuleFileName_f);
#ifndef LIBWIN32_KWDS_HMODULE_DEFINED
#define LIBWIN32_KWDS_HMODULE_DEFINED 1
PRIVATE DEFINE_KWLIST(libwin32_kwds_hModule, { K(hModule), KEND });
#endif /* !LIBWIN32_KWDS_HMODULE_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetModuleFileName_f(size_t argc, DeeObject **argv, DeeObject *kw) {
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
//[[[end]]]
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



/*[[[deemon import("_dexutils").gw("GetSystemDirectory", "->" MAYBE_NONE("?Dstring")); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetSystemDirectory_f_impl(void);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetSystemDirectory_f(size_t argc, DeeObject **argv);
#define LIBWIN32_GETSYSTEMDIRECTORY_DEF { "GetSystemDirectory", (DeeObject *)&libwin32_GetSystemDirectory, MODSYM_FNORMAL, DOC("->?Dstring") },
#define LIBWIN32_GETSYSTEMDIRECTORY_DEF_DOC(doc) { "GetSystemDirectory", (DeeObject *)&libwin32_GetSystemDirectory, MODSYM_FNORMAL, DOC("->?Dstring\n" doc) },
PRIVATE DEFINE_CMETHOD(libwin32_GetSystemDirectory, libwin32_GetSystemDirectory_f);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetSystemDirectory_f(size_t argc, DeeObject **argv) {
	if (DeeArg_Unpack(argc, argv, ":GetSystemDirectory"))
		goto err;
	return libwin32_GetSystemDirectory_f_impl();
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetSystemDirectory_f_impl(void)
//[[[end]]]
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




/*[[[deemon import("_dexutils").gw("GetWindowsDirectory", "->" MAYBE_NONE("?Dstring")); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetWindowsDirectory_f_impl(void);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetWindowsDirectory_f(size_t argc, DeeObject **argv);
#define LIBWIN32_GETWINDOWSDIRECTORY_DEF { "GetWindowsDirectory", (DeeObject *)&libwin32_GetWindowsDirectory, MODSYM_FNORMAL, DOC("->?Dstring") },
#define LIBWIN32_GETWINDOWSDIRECTORY_DEF_DOC(doc) { "GetWindowsDirectory", (DeeObject *)&libwin32_GetWindowsDirectory, MODSYM_FNORMAL, DOC("->?Dstring\n" doc) },
PRIVATE DEFINE_CMETHOD(libwin32_GetWindowsDirectory, libwin32_GetWindowsDirectory_f);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetWindowsDirectory_f(size_t argc, DeeObject **argv) {
	if (DeeArg_Unpack(argc, argv, ":GetWindowsDirectory"))
		goto err;
	return libwin32_GetWindowsDirectory_f_impl();
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetWindowsDirectory_f_impl(void)
//[[[end]]]
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


/*[[[deemon import("_dexutils").gw("GetSystemWindowsDirectory", "->" MAYBE_NONE("?Dstring")); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetSystemWindowsDirectory_f_impl(void);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetSystemWindowsDirectory_f(size_t argc, DeeObject **argv);
#define LIBWIN32_GETSYSTEMWINDOWSDIRECTORY_DEF { "GetSystemWindowsDirectory", (DeeObject *)&libwin32_GetSystemWindowsDirectory, MODSYM_FNORMAL, DOC("->?Dstring") },
#define LIBWIN32_GETSYSTEMWINDOWSDIRECTORY_DEF_DOC(doc) { "GetSystemWindowsDirectory", (DeeObject *)&libwin32_GetSystemWindowsDirectory, MODSYM_FNORMAL, DOC("->?Dstring\n" doc) },
PRIVATE DEFINE_CMETHOD(libwin32_GetSystemWindowsDirectory, libwin32_GetSystemWindowsDirectory_f);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetSystemWindowsDirectory_f(size_t argc, DeeObject **argv) {
	if (DeeArg_Unpack(argc, argv, ":GetSystemWindowsDirectory"))
		goto err;
	return libwin32_GetSystemWindowsDirectory_f_impl();
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetSystemWindowsDirectory_f_impl(void)
//[[[end]]]
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




/*[[[deemon import("_dexutils").gw("GetSystemWow64Directory", "->" MAYBE_NONE("?Dstring")); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetSystemWow64Directory_f_impl(void);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetSystemWow64Directory_f(size_t argc, DeeObject **argv);
#define LIBWIN32_GETSYSTEMWOW64DIRECTORY_DEF { "GetSystemWow64Directory", (DeeObject *)&libwin32_GetSystemWow64Directory, MODSYM_FNORMAL, DOC("->?Dstring") },
#define LIBWIN32_GETSYSTEMWOW64DIRECTORY_DEF_DOC(doc) { "GetSystemWow64Directory", (DeeObject *)&libwin32_GetSystemWow64Directory, MODSYM_FNORMAL, DOC("->?Dstring\n" doc) },
PRIVATE DEFINE_CMETHOD(libwin32_GetSystemWow64Directory, libwin32_GetSystemWow64Directory_f);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetSystemWow64Directory_f(size_t argc, DeeObject **argv) {
	if (DeeArg_Unpack(argc, argv, ":GetSystemWow64Directory"))
		goto err;
	return libwin32_GetSystemWow64Directory_f_impl();
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetSystemWow64Directory_f_impl(void)
//[[[end]]]
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


PRIVATE DEFINE_STRING(str_nul, "\0");

/* Split a given `str' at each instance of a NUL-character,
 * returning the sequence of resulting strings. */
PRIVATE DREF DeeObject *DCALL
split_nul_string(/*inherit(always)*/ DREF DeeObject *str) {
	DREF DeeObject *result;
	DeeObject *argv[1];
	if unlikely(!str)
		return NULL;
	argv[0] = (DeeObject *)&str_nul;
	result = DeeObject_CallAttrString(str, "split", 1, argv);
	Dee_Decref(str);
	return result;
}


/*[[[deemon import("_dexutils").gw("GetLogicalDriveStrings", "->" MAYBE_NONE("?S?Dstring")); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetLogicalDriveStrings_f_impl(void);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetLogicalDriveStrings_f(size_t argc, DeeObject **argv);
#define LIBWIN32_GETLOGICALDRIVESTRINGS_DEF { "GetLogicalDriveStrings", (DeeObject *)&libwin32_GetLogicalDriveStrings, MODSYM_FNORMAL, DOC("->?S?Dstring") },
#define LIBWIN32_GETLOGICALDRIVESTRINGS_DEF_DOC(doc) { "GetLogicalDriveStrings", (DeeObject *)&libwin32_GetLogicalDriveStrings, MODSYM_FNORMAL, DOC("->?S?Dstring\n" doc) },
PRIVATE DEFINE_CMETHOD(libwin32_GetLogicalDriveStrings, libwin32_GetLogicalDriveStrings_f);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetLogicalDriveStrings_f(size_t argc, DeeObject **argv) {
	if (DeeArg_Unpack(argc, argv, ":GetLogicalDriveStrings"))
		goto err;
	return libwin32_GetLogicalDriveStrings_f_impl();
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetLogicalDriveStrings_f_impl(void)
//[[[end]]]
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




/*[[[deemon import("_dexutils").gw("QueryDosDevice", "lpDeviceName:nt:LPCWSTR->" MAYBE_NONE("?S?Dstring")); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_QueryDosDevice_f_impl(LPCWSTR lpDeviceName);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_QueryDosDevice_f(size_t argc, DeeObject **argv, DeeObject *kw);
#define LIBWIN32_QUERYDOSDEVICE_DEF { "QueryDosDevice", (DeeObject *)&libwin32_QueryDosDevice, MODSYM_FNORMAL, DOC("(lpDeviceName:?Dstring)->?S?Dstring") },
#define LIBWIN32_QUERYDOSDEVICE_DEF_DOC(doc) { "QueryDosDevice", (DeeObject *)&libwin32_QueryDosDevice, MODSYM_FNORMAL, DOC("(lpDeviceName:?Dstring)->?S?Dstring\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_QueryDosDevice, libwin32_QueryDosDevice_f);
#ifndef LIBWIN32_KWDS_LPDEVICENAME_DEFINED
#define LIBWIN32_KWDS_LPDEVICENAME_DEFINED 1
PRIVATE DEFINE_KWLIST(libwin32_kwds_lpDeviceName, { K(lpDeviceName), KEND });
#endif /* !LIBWIN32_KWDS_LPDEVICENAME_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_QueryDosDevice_f(size_t argc, DeeObject **argv, DeeObject *kw) {
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
//[[[end]]]
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



/*[[[deemon import("_dexutils").gw("GetFileType", "hFile:nt:HANDLE->" MAYBE_NONE("?Dint")); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetFileType_f_impl(HANDLE hFile);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetFileType_f(size_t argc, DeeObject **argv, DeeObject *kw);
#define LIBWIN32_GETFILETYPE_DEF { "GetFileType", (DeeObject *)&libwin32_GetFileType, MODSYM_FNORMAL, DOC("(hFile:?X3?Dint?DFile?Ewin32:HANDLE)->?Dint") },
#define LIBWIN32_GETFILETYPE_DEF_DOC(doc) { "GetFileType", (DeeObject *)&libwin32_GetFileType, MODSYM_FNORMAL, DOC("(hFile:?X3?Dint?DFile?Ewin32:HANDLE)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_GetFileType, libwin32_GetFileType_f);
#ifndef LIBWIN32_KWDS_HFILE_DEFINED
#define LIBWIN32_KWDS_HFILE_DEFINED 1
PRIVATE DEFINE_KWLIST(libwin32_kwds_hFile, { K(hFile), KEND });
#endif /* !LIBWIN32_KWDS_HFILE_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetFileType_f(size_t argc, DeeObject **argv, DeeObject *kw) {
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
//[[[end]]]
{
	DWORD dwType;
again:
	DBG_ALIGNMENT_DISABLE();
	dwType = GetFileType(hFile);
	if (dwType == FILE_TYPE_UNKNOWN) {
		DWORD dwError = GetLastError();
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




/*[[[deemon import("_dexutils").gw("GetFileSize", "hFile:nt:HANDLE->" MAYBE_NONE("?Dint")); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetFileSize_f_impl(HANDLE hFile);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetFileSize_f(size_t argc, DeeObject **argv, DeeObject *kw);
#define LIBWIN32_GETFILESIZE_DEF { "GetFileSize", (DeeObject *)&libwin32_GetFileSize, MODSYM_FNORMAL, DOC("(hFile:?X3?Dint?DFile?Ewin32:HANDLE)->?Dint") },
#define LIBWIN32_GETFILESIZE_DEF_DOC(doc) { "GetFileSize", (DeeObject *)&libwin32_GetFileSize, MODSYM_FNORMAL, DOC("(hFile:?X3?Dint?DFile?Ewin32:HANDLE)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_GetFileSize, libwin32_GetFileSize_f);
#ifndef LIBWIN32_KWDS_HFILE_DEFINED
#define LIBWIN32_KWDS_HFILE_DEFINED 1
PRIVATE DEFINE_KWLIST(libwin32_kwds_hFile, { K(hFile), KEND });
#endif /* !LIBWIN32_KWDS_HFILE_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetFileSize_f(size_t argc, DeeObject **argv, DeeObject *kw) {
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
//[[[end]]]
{
	DWORD dwSizeLow, dwSizeHigh;
again:
	DBG_ALIGNMENT_DISABLE();
	dwSizeLow = GetFileSize(hFile, &dwSizeHigh);
	if (dwSizeLow == INVALID_FILE_SIZE) {
		DWORD dwError = GetLastError();
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



/*[[[deemon import("_dexutils").gw("GetFileAttributes", "lpFileName:nt:LPCWSTR->" MAYBE_NONE("?Dint")); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetFileAttributes_f_impl(LPCWSTR lpFileName);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetFileAttributes_f(size_t argc, DeeObject **argv, DeeObject *kw);
#define LIBWIN32_GETFILEATTRIBUTES_DEF { "GetFileAttributes", (DeeObject *)&libwin32_GetFileAttributes, MODSYM_FNORMAL, DOC("(lpFileName:?Dstring)->?Dint") },
#define LIBWIN32_GETFILEATTRIBUTES_DEF_DOC(doc) { "GetFileAttributes", (DeeObject *)&libwin32_GetFileAttributes, MODSYM_FNORMAL, DOC("(lpFileName:?Dstring)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_GetFileAttributes, libwin32_GetFileAttributes_f);
#ifndef LIBWIN32_KWDS_LPFILENAME_DEFINED
#define LIBWIN32_KWDS_LPFILENAME_DEFINED 1
PRIVATE DEFINE_KWLIST(libwin32_kwds_lpFileName, { K(lpFileName), KEND });
#endif /* !LIBWIN32_KWDS_LPFILENAME_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetFileAttributes_f(size_t argc, DeeObject **argv, DeeObject *kw) {
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
//[[[end]]]
{
	DWORD dwResult;
again:
	DBG_ALIGNMENT_DISABLE();
	dwResult = GetFileAttributesW(lpFileName);
	if (dwResult == INVALID_FILE_ATTRIBUTES) {
		DWORD dwError = GetLastError();
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



/*[[[deemon import("_dexutils").gw("SetFileAttributes", "lpFileName:nt:LPCWSTR,dwFileAttributes:nt:DWORD" ERROR_OR_BOOL); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_SetFileAttributes_f_impl(LPCWSTR lpFileName, DWORD dwFileAttributes);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_SetFileAttributes_f(size_t argc, DeeObject **argv, DeeObject *kw);
#define LIBWIN32_SETFILEATTRIBUTES_DEF { "SetFileAttributes", (DeeObject *)&libwin32_SetFileAttributes, MODSYM_FNORMAL, DOC("(lpFileName:?Dstring,dwFileAttributes:?Dint)") },
#define LIBWIN32_SETFILEATTRIBUTES_DEF_DOC(doc) { "SetFileAttributes", (DeeObject *)&libwin32_SetFileAttributes, MODSYM_FNORMAL, DOC("(lpFileName:?Dstring,dwFileAttributes:?Dint)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_SetFileAttributes, libwin32_SetFileAttributes_f);
#ifndef LIBWIN32_KWDS_LPFILENAME_DWFILEATTRIBUTES_DEFINED
#define LIBWIN32_KWDS_LPFILENAME_DWFILEATTRIBUTES_DEFINED 1
PRIVATE DEFINE_KWLIST(libwin32_kwds_lpFileName_dwFileAttributes, { K(lpFileName), K(dwFileAttributes), KEND });
#endif /* !LIBWIN32_KWDS_LPFILENAME_DWFILEATTRIBUTES_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_SetFileAttributes_f(size_t argc, DeeObject **argv, DeeObject *kw) {
	LPCWSTR lpFileName_str;
	DeeStringObject *lpFileName;
	DWORD dwFileAttributes;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_lpFileName_dwFileAttributes, "oI32u:SetFileAttributes", &lpFileName, &dwFileAttributes))
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
//[[[end]]]
{
	BOOL bResult;
again:
	DBG_ALIGNMENT_DISABLE();
	bResult = SetFileAttributesW(lpFileName,
	                             dwFileAttributes);
	if (!bResult) {
		DWORD dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(dwError)) {
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		RETURN_ERROR_OR_FALSE(dwError,
		                      "Failed to set attributes of file %lq to %#I32x",
		                      lpFileName, dwFileAttributes);
	}
	DBG_ALIGNMENT_ENABLE();
	RETURN_SUCCESS_OR_TRUE;
err:
	return NULL;
}



/*[[[deemon import("_dexutils").gw("GetCompressedFileSize", "lpFileName:nt:LPCWSTR->" MAYBE_NONE("?Dint")); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetCompressedFileSize_f_impl(LPCWSTR lpFileName);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetCompressedFileSize_f(size_t argc, DeeObject **argv, DeeObject *kw);
#define LIBWIN32_GETCOMPRESSEDFILESIZE_DEF { "GetCompressedFileSize", (DeeObject *)&libwin32_GetCompressedFileSize, MODSYM_FNORMAL, DOC("(lpFileName:?Dstring)->?Dint") },
#define LIBWIN32_GETCOMPRESSEDFILESIZE_DEF_DOC(doc) { "GetCompressedFileSize", (DeeObject *)&libwin32_GetCompressedFileSize, MODSYM_FNORMAL, DOC("(lpFileName:?Dstring)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_GetCompressedFileSize, libwin32_GetCompressedFileSize_f);
#ifndef LIBWIN32_KWDS_LPFILENAME_DEFINED
#define LIBWIN32_KWDS_LPFILENAME_DEFINED 1
PRIVATE DEFINE_KWLIST(libwin32_kwds_lpFileName, { K(lpFileName), KEND });
#endif /* !LIBWIN32_KWDS_LPFILENAME_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetCompressedFileSize_f(size_t argc, DeeObject **argv, DeeObject *kw) {
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
//[[[end]]]
{
	DWORD dwSizeLow, dwSizeHigh;
again:
	DBG_ALIGNMENT_DISABLE();
	dwSizeLow = GetCompressedFileSizeW(lpFileName, &dwSizeHigh);
	if (dwSizeLow == INVALID_FILE_SIZE) {
		DWORD dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(dwError)) {
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		if (dwError != NO_ERROR)
			RETURN_ERROR(dwError, "Failed to determine the compressed size of %lq", lpFileName);
	}
	DBG_ALIGNMENT_ENABLE();
	return DeeInt_NewU64((uint64_t)dwSizeLow |
	                     (uint64_t)dwSizeHigh << 32);
err:
	return NULL;
}



/*[[[deemon import("_dexutils").gw("FlushFileBuffers", "hFile:nt:HANDLE" ERROR_OR_BOOL); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_FlushFileBuffers_f_impl(HANDLE hFile);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_FlushFileBuffers_f(size_t argc, DeeObject **argv, DeeObject *kw);
#define LIBWIN32_FLUSHFILEBUFFERS_DEF { "FlushFileBuffers", (DeeObject *)&libwin32_FlushFileBuffers, MODSYM_FNORMAL, DOC("(hFile:?X3?Dint?DFile?Ewin32:HANDLE)") },
#define LIBWIN32_FLUSHFILEBUFFERS_DEF_DOC(doc) { "FlushFileBuffers", (DeeObject *)&libwin32_FlushFileBuffers, MODSYM_FNORMAL, DOC("(hFile:?X3?Dint?DFile?Ewin32:HANDLE)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_FlushFileBuffers, libwin32_FlushFileBuffers_f);
#ifndef LIBWIN32_KWDS_HFILE_DEFINED
#define LIBWIN32_KWDS_HFILE_DEFINED 1
PRIVATE DEFINE_KWLIST(libwin32_kwds_hFile, { K(hFile), KEND });
#endif /* !LIBWIN32_KWDS_HFILE_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_FlushFileBuffers_f(size_t argc, DeeObject **argv, DeeObject *kw) {
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
//[[[end]]]
{
	BOOL bResult;
again:
	DBG_ALIGNMENT_DISABLE();
	bResult = FlushFileBuffers(hFile);
	if (!bResult) {
		DWORD dwError = GetLastError();
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



/*[[[deemon import("_dexutils").gw("GetFinalPathNameByHandle", "hFile:nt:HANDLE,dwFlags:nt:DWORD=0->" MAYBE_NONE("?Dstring")); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetFinalPathNameByHandle_f_impl(HANDLE hFile, DWORD dwFlags);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetFinalPathNameByHandle_f(size_t argc, DeeObject **argv, DeeObject *kw);
#define LIBWIN32_GETFINALPATHNAMEBYHANDLE_DEF { "GetFinalPathNameByHandle", (DeeObject *)&libwin32_GetFinalPathNameByHandle, MODSYM_FNORMAL, DOC("(hFile:?X3?Dint?DFile?Ewin32:HANDLE,dwFlags:?Dint=!0)->?Dstring") },
#define LIBWIN32_GETFINALPATHNAMEBYHANDLE_DEF_DOC(doc) { "GetFinalPathNameByHandle", (DeeObject *)&libwin32_GetFinalPathNameByHandle, MODSYM_FNORMAL, DOC("(hFile:?X3?Dint?DFile?Ewin32:HANDLE,dwFlags:?Dint=!0)->?Dstring\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_GetFinalPathNameByHandle, libwin32_GetFinalPathNameByHandle_f);
#ifndef LIBWIN32_KWDS_HFILE_DWFLAGS_DEFINED
#define LIBWIN32_KWDS_HFILE_DWFLAGS_DEFINED 1
PRIVATE DEFINE_KWLIST(libwin32_kwds_hFile_dwFlags, { K(hFile), K(dwFlags), KEND });
#endif /* !LIBWIN32_KWDS_HFILE_DWFLAGS_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetFinalPathNameByHandle_f(size_t argc, DeeObject **argv, DeeObject *kw) {
	HANDLE hhFile;
	DeeObject *hFile;
	DWORD dwFlags = 0;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_hFile_dwFlags, "o|I32u:GetFinalPathNameByHandle", &hFile, &dwFlags))
		goto err;
	if (DeeNTSystem_TryGetHandle(hFile, (void **)&hhFile))
		goto err;
	return libwin32_GetFinalPathNameByHandle_f_impl(hhFile, dwFlags);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetFinalPathNameByHandle_f_impl(HANDLE hFile, DWORD dwFlags)
//[[[end]]]
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



/*[[[deemon import("_dexutils").gw("GetFilenameOfHandle", "hFile:nt:HANDLE->" MAYBE_NONE("?Dstring")); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetFilenameOfHandle_f_impl(HANDLE hFile);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetFilenameOfHandle_f(size_t argc, DeeObject **argv, DeeObject *kw);
#define LIBWIN32_GETFILENAMEOFHANDLE_DEF { "GetFilenameOfHandle", (DeeObject *)&libwin32_GetFilenameOfHandle, MODSYM_FNORMAL, DOC("(hFile:?X3?Dint?DFile?Ewin32:HANDLE)->?Dstring") },
#define LIBWIN32_GETFILENAMEOFHANDLE_DEF_DOC(doc) { "GetFilenameOfHandle", (DeeObject *)&libwin32_GetFilenameOfHandle, MODSYM_FNORMAL, DOC("(hFile:?X3?Dint?DFile?Ewin32:HANDLE)->?Dstring\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_GetFilenameOfHandle, libwin32_GetFilenameOfHandle_f);
#ifndef LIBWIN32_KWDS_HFILE_DEFINED
#define LIBWIN32_KWDS_HFILE_DEFINED 1
PRIVATE DEFINE_KWLIST(libwin32_kwds_hFile, { K(hFile), KEND });
#endif /* !LIBWIN32_KWDS_HFILE_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetFilenameOfHandle_f(size_t argc, DeeObject **argv, DeeObject *kw) {
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
//[[[end]]]
{
	DREF DeeObject *result;
	result = DeeNTSystem_TryGetFilenameOfHandle((void *)hFile);
	if (result != ITER_DONE)
		return result;
	RETURN_ERROR(GetLastError(),
	             "Failed to determine the filename of handle %p",
	             hFile);
}



/*[[[deemon import("_dexutils").gw("FormatErrorMessage", "dwError:nt:DWORD->" MAYBE_NONE("?Dstring")); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_FormatErrorMessage_f_impl(DWORD dwError);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_FormatErrorMessage_f(size_t argc, DeeObject **argv, DeeObject *kw);
#define LIBWIN32_FORMATERRORMESSAGE_DEF { "FormatErrorMessage", (DeeObject *)&libwin32_FormatErrorMessage, MODSYM_FNORMAL, DOC("(dwError:?Dint)->?Dstring") },
#define LIBWIN32_FORMATERRORMESSAGE_DEF_DOC(doc) { "FormatErrorMessage", (DeeObject *)&libwin32_FormatErrorMessage, MODSYM_FNORMAL, DOC("(dwError:?Dint)->?Dstring\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_FormatErrorMessage, libwin32_FormatErrorMessage_f);
#ifndef LIBWIN32_KWDS_DWERROR_DEFINED
#define LIBWIN32_KWDS_DWERROR_DEFINED 1
PRIVATE DEFINE_KWLIST(libwin32_kwds_dwError, { K(dwError), KEND });
#endif /* !LIBWIN32_KWDS_DWERROR_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_FormatErrorMessage_f(size_t argc, DeeObject **argv, DeeObject *kw) {
	DWORD dwError;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_dwError, "I32u:FormatErrorMessage", &dwError))
		goto err;
	return libwin32_FormatErrorMessage_f_impl(dwError);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_FormatErrorMessage_f_impl(DWORD dwError)
//[[[end]]]
{
	DREF DeeObject *result;
	result = DeeNTSystem_FormatErrorMessage(dwError);
	if (result != ITER_DONE)
		return result;
	RETURN_ERROR(GetLastError(),
	             "Failed to format the message for error %#lx",
	             dwError);
}



/*[[[deemon import("_dexutils").gw("GetMappedFileName", "hProcess:nt:HANDLE,lpv:c:ptr->" MAYBE_NONE("?Dstring")); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetMappedFileName_f_impl(HANDLE hProcess, void *lpv);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetMappedFileName_f(size_t argc, DeeObject **argv, DeeObject *kw);
#define LIBWIN32_GETMAPPEDFILENAME_DEF { "GetMappedFileName", (DeeObject *)&libwin32_GetMappedFileName, MODSYM_FNORMAL, DOC("(hProcess:?X3?Dint?DFile?Ewin32:HANDLE,lpv:?Aptr?Ectypes:void)->?Dstring") },
#define LIBWIN32_GETMAPPEDFILENAME_DEF_DOC(doc) { "GetMappedFileName", (DeeObject *)&libwin32_GetMappedFileName, MODSYM_FNORMAL, DOC("(hProcess:?X3?Dint?DFile?Ewin32:HANDLE,lpv:?Aptr?Ectypes:void)->?Dstring\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_GetMappedFileName, libwin32_GetMappedFileName_f);
#ifndef LIBWIN32_KWDS_HPROCESS_LPV_DEFINED
#define LIBWIN32_KWDS_HPROCESS_LPV_DEFINED 1
PRIVATE DEFINE_KWLIST(libwin32_kwds_hProcess_lpv, { K(hProcess), K(lpv), KEND });
#endif /* !LIBWIN32_KWDS_HPROCESS_LPV_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetMappedFileName_f(size_t argc, DeeObject **argv, DeeObject *kw) {
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
//[[[end]]]
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



/*[[[deemon import("_dexutils").gw("MapViewOfFile",
     "hFileMappingObject:nt:HANDLE"
    ",dwDesiredAccess:nt:DWORD=FILE_MAP_READ"
    ",dwFileOffset:I64u=0"
    ",dwNumberOfBytesToMap:Iu=0"
    "->" MAYBE_NONE("?Aptr?Ectypes:void")); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_MapViewOfFile_f_impl(HANDLE hFileMappingObject, DWORD dwDesiredAccess, uint64_t dwFileOffset, size_t dwNumberOfBytesToMap);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_MapViewOfFile_f(size_t argc, DeeObject **argv, DeeObject *kw);
#define LIBWIN32_MAPVIEWOFFILE_DEF { "MapViewOfFile", (DeeObject *)&libwin32_MapViewOfFile, MODSYM_FNORMAL, DOC("(hFileMappingObject:?X3?Dint?DFile?Ewin32:HANDLE,dwDesiredAccess:?Dint=!GFILE_MAP_READ,dwFileOffset:?Dint=!0,dwNumberOfBytesToMap:?Dint=!0)->?Aptr?Ectypes:void") },
#define LIBWIN32_MAPVIEWOFFILE_DEF_DOC(doc) { "MapViewOfFile", (DeeObject *)&libwin32_MapViewOfFile, MODSYM_FNORMAL, DOC("(hFileMappingObject:?X3?Dint?DFile?Ewin32:HANDLE,dwDesiredAccess:?Dint=!GFILE_MAP_READ,dwFileOffset:?Dint=!0,dwNumberOfBytesToMap:?Dint=!0)->?Aptr?Ectypes:void\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_MapViewOfFile, libwin32_MapViewOfFile_f);
#ifndef LIBWIN32_KWDS_HFILEMAPPINGOBJECT_DWDESIREDACCESS_DWFILEOFFSET_DWNUMBEROFBYTESTOMAP_DEFINED
#define LIBWIN32_KWDS_HFILEMAPPINGOBJECT_DWDESIREDACCESS_DWFILEOFFSET_DWNUMBEROFBYTESTOMAP_DEFINED 1
PRIVATE DEFINE_KWLIST(libwin32_kwds_hFileMappingObject_dwDesiredAccess_dwFileOffset_dwNumberOfBytesToMap, { K(hFileMappingObject), K(dwDesiredAccess), K(dwFileOffset), K(dwNumberOfBytesToMap), KEND });
#endif /* !LIBWIN32_KWDS_HFILEMAPPINGOBJECT_DWDESIREDACCESS_DWFILEOFFSET_DWNUMBEROFBYTESTOMAP_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_MapViewOfFile_f(size_t argc, DeeObject **argv, DeeObject *kw) {
	HANDLE hhFileMappingObject;
	DeeObject *hFileMappingObject;
	DWORD dwDesiredAccess = FILE_MAP_READ;
	uint64_t dwFileOffset = 0;
	size_t dwNumberOfBytesToMap = 0;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_hFileMappingObject_dwDesiredAccess_dwFileOffset_dwNumberOfBytesToMap, "o|I32uI64uIu:MapViewOfFile", &hFileMappingObject, &dwDesiredAccess, &dwFileOffset, &dwNumberOfBytesToMap))
		goto err;
	if (DeeNTSystem_TryGetHandle(hFileMappingObject, (void **)&hhFileMappingObject))
		goto err;
	return libwin32_MapViewOfFile_f_impl(hhFileMappingObject, dwDesiredAccess, dwFileOffset, dwNumberOfBytesToMap);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_MapViewOfFile_f_impl(HANDLE hFileMappingObject, DWORD dwDesiredAccess, uint64_t dwFileOffset, size_t dwNumberOfBytesToMap)
//[[[end]]]
{
	DREF DeeObject *result;
	LPVOID pResult;
again:
	pResult = MapViewOfFile(hFileMappingObject,
	                        dwDesiredAccess,
	                        (DWORD)(dwFileOffset >> 32),
	                        (DWORD)(dwFileOffset),
	                        dwNumberOfBytesToMap);
	if (pResult == NULL) {
		DWORD dwError;
		dwError = GetLastError();
		if (DeeNTSystem_IsIntr(dwError)) {
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		RETURN_ERROR(dwError,
		             "Failed to map a view of %p into memory "
		             "(dwDesiredAccess: %#I32x, dwFileOffset: %I64u, dwNumberOfBytesToMap: %Iu)",
		             hFileMappingObject, dwDesiredAccess,
		             dwFileOffset, dwNumberOfBytesToMap);
	}
	result = DeeCTypes_CreateVoidPointer(pResult);
	if unlikely(!result) {
		DWORD dwError;
		dwError = GetLastError();
		UnmapViewOfFile(pResult);
		SetLastError(dwError);
	}
	return result;
err:
	return NULL;
}



/*[[[deemon import("_dexutils").gw("UnmapViewOfFile", "lpBaseAddress:c:ptr" ERROR_OR_BOOL); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_UnmapViewOfFile_f_impl(void *lpBaseAddress);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_UnmapViewOfFile_f(size_t argc, DeeObject **argv, DeeObject *kw);
#define LIBWIN32_UNMAPVIEWOFFILE_DEF { "UnmapViewOfFile", (DeeObject *)&libwin32_UnmapViewOfFile, MODSYM_FNORMAL, DOC("(lpBaseAddress:?Aptr?Ectypes:void)") },
#define LIBWIN32_UNMAPVIEWOFFILE_DEF_DOC(doc) { "UnmapViewOfFile", (DeeObject *)&libwin32_UnmapViewOfFile, MODSYM_FNORMAL, DOC("(lpBaseAddress:?Aptr?Ectypes:void)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_UnmapViewOfFile, libwin32_UnmapViewOfFile_f);
#ifndef LIBWIN32_KWDS_LPBASEADDRESS_DEFINED
#define LIBWIN32_KWDS_LPBASEADDRESS_DEFINED 1
PRIVATE DEFINE_KWLIST(libwin32_kwds_lpBaseAddress, { K(lpBaseAddress), KEND });
#endif /* !LIBWIN32_KWDS_LPBASEADDRESS_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_UnmapViewOfFile_f(size_t argc, DeeObject **argv, DeeObject *kw) {
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
//[[[end]]]
{
	BOOL bOk;
again:
	bOk = UnmapViewOfFile((LPCVOID)lpBaseAddress);
	if (!bOk) {
		DWORD dwError = GetLastError();
		if (DeeNTSystem_IsIntr(dwError)) {
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		RETURN_ERROR_OR_FALSE(dwError,
		                      "failed to unmap view of file at %p",
		                      lpBaseAddress);
	}
	RETURN_SUCCESS_OR_TRUE;
err:
	return NULL;
}



/*[[[deemon import("_dexutils").gw("CreateFileMapping",
     "hFile:nt:HANDLE"
    ",lpFileMappingAttributes:?GSECURITY_ATTRIBUTES=NULL"
    ",flProtect:nt:DWORD=PAGE_READONLY"
    ",dwMaximumSize:I64u=0"
    ",lpName:nt:LPCWSTR=NULL"
    "->" MAYBE_NONE("?GHANDLE")); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_CreateFileMapping_f_impl(HANDLE hFile, DeeObject *lpFileMappingAttributes, DWORD flProtect, uint64_t dwMaximumSize, LPCWSTR lpName);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_CreateFileMapping_f(size_t argc, DeeObject **argv, DeeObject *kw);
#define LIBWIN32_CREATEFILEMAPPING_DEF { "CreateFileMapping", (DeeObject *)&libwin32_CreateFileMapping, MODSYM_FNORMAL, DOC("(hFile:?X3?Dint?DFile?Ewin32:HANDLE,lpFileMappingAttributes?:?GSECURITY_ATTRIBUTES,flProtect:?Dint=!GPAGE_READONLY,dwMaximumSize:?Dint=!0,lpName?:?Dstring)->?GHANDLE") },
#define LIBWIN32_CREATEFILEMAPPING_DEF_DOC(doc) { "CreateFileMapping", (DeeObject *)&libwin32_CreateFileMapping, MODSYM_FNORMAL, DOC("(hFile:?X3?Dint?DFile?Ewin32:HANDLE,lpFileMappingAttributes?:?GSECURITY_ATTRIBUTES,flProtect:?Dint=!GPAGE_READONLY,dwMaximumSize:?Dint=!0,lpName?:?Dstring)->?GHANDLE\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_CreateFileMapping, libwin32_CreateFileMapping_f);
#ifndef LIBWIN32_KWDS_HFILE_LPFILEMAPPINGATTRIBUTES_FLPROTECT_DWMAXIMUMSIZE_LPNAME_DEFINED
#define LIBWIN32_KWDS_HFILE_LPFILEMAPPINGATTRIBUTES_FLPROTECT_DWMAXIMUMSIZE_LPNAME_DEFINED 1
PRIVATE DEFINE_KWLIST(libwin32_kwds_hFile_lpFileMappingAttributes_flProtect_dwMaximumSize_lpName, { K(hFile), K(lpFileMappingAttributes), K(flProtect), K(dwMaximumSize), K(lpName), KEND });
#endif /* !LIBWIN32_KWDS_HFILE_LPFILEMAPPINGATTRIBUTES_FLPROTECT_DWMAXIMUMSIZE_LPNAME_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_CreateFileMapping_f(size_t argc, DeeObject **argv, DeeObject *kw) {
	HANDLE hhFile;
	DeeObject *hFile;
	DeeObject *lpFileMappingAttributes = NULL;
	DWORD flProtect = PAGE_READONLY;
	uint64_t dwMaximumSize = 0;
	LPCWSTR lpName_str = NULL;
	DeeStringObject *lpName = (DeeStringObject *)Dee_None;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_hFile_lpFileMappingAttributes_flProtect_dwMaximumSize_lpName, "o|oI32uI64uo:CreateFileMapping", &hFile, &lpFileMappingAttributes, &flProtect, &dwMaximumSize, &lpName))
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
//[[[end]]]
{
	DREF DeeObject *result;
	HANDLE hResult;
again:
	(void)lpFileMappingAttributes; /* TODO */
	hResult = CreateFileMappingW(hFile,
	                             NULL,
	                             flProtect,
	                             (DWORD)(dwMaximumSize >> 32),
	                             (DWORD)(dwMaximumSize),
	                             lpName);
	if (hResult == NULL) {
		DWORD dwError;
		dwError = GetLastError();
		if (DeeNTSystem_IsIntr(dwError)) {
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		RETURN_ERROR(dwError,
		             "Failed to create file mapping for %p "
		             "(flProtect: %#I32x, dwMaximumSize: %I64u, lpName: %lq)",
		             hFile, flProtect, dwMaximumSize, lpName);
	}
	result = libwin32_CreateHandle(hResult);
	if unlikely(!result) {
		DWORD dwError;
		dwError = GetLastError();
		CloseHandle(hResult);
		SetLastError(dwError);
	}
	return result;
err:
	return NULL;
}



/*[[[deemon import("_dexutils").gw("GetCurrentProcess", "->?GHANDLE"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetCurrentProcess_f_impl(void);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetCurrentProcess_f(size_t argc, DeeObject **argv);
#define LIBWIN32_GETCURRENTPROCESS_DEF { "GetCurrentProcess", (DeeObject *)&libwin32_GetCurrentProcess, MODSYM_FNORMAL, DOC("->?GHANDLE") },
#define LIBWIN32_GETCURRENTPROCESS_DEF_DOC(doc) { "GetCurrentProcess", (DeeObject *)&libwin32_GetCurrentProcess, MODSYM_FNORMAL, DOC("->?GHANDLE\n" doc) },
PRIVATE DEFINE_CMETHOD(libwin32_GetCurrentProcess, libwin32_GetCurrentProcess_f);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetCurrentProcess_f(size_t argc, DeeObject **argv) {
	if (DeeArg_Unpack(argc, argv, ":GetCurrentProcess"))
		goto err;
	return libwin32_GetCurrentProcess_f_impl();
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetCurrentProcess_f_impl(void)
//[[[end]]]
{
	return libwin32_CreateHandle(GetCurrentProcess());
}



/*[[[deemon import("_dexutils").gw("GetCurrentThread", "->?GHANDLE"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetCurrentThread_f_impl(void);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetCurrentThread_f(size_t argc, DeeObject **argv);
#define LIBWIN32_GETCURRENTTHREAD_DEF { "GetCurrentThread", (DeeObject *)&libwin32_GetCurrentThread, MODSYM_FNORMAL, DOC("->?GHANDLE") },
#define LIBWIN32_GETCURRENTTHREAD_DEF_DOC(doc) { "GetCurrentThread", (DeeObject *)&libwin32_GetCurrentThread, MODSYM_FNORMAL, DOC("->?GHANDLE\n" doc) },
PRIVATE DEFINE_CMETHOD(libwin32_GetCurrentThread, libwin32_GetCurrentThread_f);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetCurrentThread_f(size_t argc, DeeObject **argv) {
	if (DeeArg_Unpack(argc, argv, ":GetCurrentThread"))
		goto err;
	return libwin32_GetCurrentThread_f_impl();
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetCurrentThread_f_impl(void)
//[[[end]]]
{
	return libwin32_CreateHandle(GetCurrentThread());
}



/*[[[deemon import("_dexutils").gw("GetCurrentProcessId", "->?Dint"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetCurrentProcessId_f_impl(void);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetCurrentProcessId_f(size_t argc, DeeObject **argv);
#define LIBWIN32_GETCURRENTPROCESSID_DEF { "GetCurrentProcessId", (DeeObject *)&libwin32_GetCurrentProcessId, MODSYM_FNORMAL, DOC("->?Dint") },
#define LIBWIN32_GETCURRENTPROCESSID_DEF_DOC(doc) { "GetCurrentProcessId", (DeeObject *)&libwin32_GetCurrentProcessId, MODSYM_FNORMAL, DOC("->?Dint\n" doc) },
PRIVATE DEFINE_CMETHOD(libwin32_GetCurrentProcessId, libwin32_GetCurrentProcessId_f);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetCurrentProcessId_f(size_t argc, DeeObject **argv) {
	if (DeeArg_Unpack(argc, argv, ":GetCurrentProcessId"))
		goto err;
	return libwin32_GetCurrentProcessId_f_impl();
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetCurrentProcessId_f_impl(void)
//[[[end]]]
{
	return DeeInt_NewU32(GetCurrentProcessId());
}



/*[[[deemon import("_dexutils").gw("GetCurrentThreadId", "->?Dint"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetCurrentThreadId_f_impl(void);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetCurrentThreadId_f(size_t argc, DeeObject **argv);
#define LIBWIN32_GETCURRENTTHREADID_DEF { "GetCurrentThreadId", (DeeObject *)&libwin32_GetCurrentThreadId, MODSYM_FNORMAL, DOC("->?Dint") },
#define LIBWIN32_GETCURRENTTHREADID_DEF_DOC(doc) { "GetCurrentThreadId", (DeeObject *)&libwin32_GetCurrentThreadId, MODSYM_FNORMAL, DOC("->?Dint\n" doc) },
PRIVATE DEFINE_CMETHOD(libwin32_GetCurrentThreadId, libwin32_GetCurrentThreadId_f);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetCurrentThreadId_f(size_t argc, DeeObject **argv) {
	if (DeeArg_Unpack(argc, argv, ":GetCurrentThreadId"))
		goto err;
	return libwin32_GetCurrentThreadId_f_impl();
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetCurrentThreadId_f_impl(void)
//[[[end]]]
{
	return DeeInt_NewU32(GetCurrentThreadId());
}



/*[[[deemon import("_dexutils").gw("GetStdHandle", "nStdHandle:nt:DWORD->?GHANDLE"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetStdHandle_f_impl(DWORD nStdHandle);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetStdHandle_f(size_t argc, DeeObject **argv, DeeObject *kw);
#define LIBWIN32_GETSTDHANDLE_DEF { "GetStdHandle", (DeeObject *)&libwin32_GetStdHandle, MODSYM_FNORMAL, DOC("(nStdHandle:?Dint)->?GHANDLE") },
#define LIBWIN32_GETSTDHANDLE_DEF_DOC(doc) { "GetStdHandle", (DeeObject *)&libwin32_GetStdHandle, MODSYM_FNORMAL, DOC("(nStdHandle:?Dint)->?GHANDLE\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_GetStdHandle, libwin32_GetStdHandle_f);
#ifndef LIBWIN32_KWDS_NSTDHANDLE_DEFINED
#define LIBWIN32_KWDS_NSTDHANDLE_DEFINED 1
PRIVATE DEFINE_KWLIST(libwin32_kwds_nStdHandle, { K(nStdHandle), KEND });
#endif /* !LIBWIN32_KWDS_NSTDHANDLE_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_GetStdHandle_f(size_t argc, DeeObject **argv, DeeObject *kw) {
	DWORD nStdHandle;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_nStdHandle, "I32u:GetStdHandle", &nStdHandle))
		goto err;
	return libwin32_GetStdHandle_f_impl(nStdHandle);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_GetStdHandle_f_impl(DWORD nStdHandle)
//[[[end]]]
{
	HANDLE hResult;
	DBG_ALIGNMENT_DISABLE();
again:
	hResult = GetStdHandle(nStdHandle);
	if unlikely(!hResult || hResult == INVALID_HANDLE_VALUE) {
		DWORD dwError = GetLastError();
		SetLastError(NO_ERROR);
		if (DeeNTSystem_IsIntr(dwError)) {
			DBG_ALIGNMENT_ENABLE();
check_interrupt:
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		hResult = GetStdHandle(nStdHandle);
		if (!hResult || hResult == INVALID_HANDLE_VALUE) {
			dwError = GetLastError();
			DBG_ALIGNMENT_ENABLE();
			if (dwError != NO_ERROR) {
				if (DeeNTSystem_IsIntr(dwError))
					goto check_interrupt;
				RETURN_ERROR(dwError,
				             "Failed to get STD handle %I32u",
				             nStdHandle);
			}
		}
	}
	DBG_ALIGNMENT_ENABLE();
	return libwin32_CreateHandle(hResult);
err:
	return NULL;
}



/*[[[deemon import("_dexutils").gw("SetStdHandle", "nStdHandle:nt:DWORD,hHandle:nt:HANDLE" ERROR_OR_BOOL); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_SetStdHandle_f_impl(DWORD nStdHandle, HANDLE hHandle);
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_SetStdHandle_f(size_t argc, DeeObject **argv, DeeObject *kw);
#define LIBWIN32_SETSTDHANDLE_DEF { "SetStdHandle", (DeeObject *)&libwin32_SetStdHandle, MODSYM_FNORMAL, DOC("(nStdHandle:?Dint,hHandle:?X3?Dint?DFile?Ewin32:HANDLE)") },
#define LIBWIN32_SETSTDHANDLE_DEF_DOC(doc) { "SetStdHandle", (DeeObject *)&libwin32_SetStdHandle, MODSYM_FNORMAL, DOC("(nStdHandle:?Dint,hHandle:?X3?Dint?DFile?Ewin32:HANDLE)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_SetStdHandle, libwin32_SetStdHandle_f);
#ifndef LIBWIN32_KWDS_NSTDHANDLE_HHANDLE_DEFINED
#define LIBWIN32_KWDS_NSTDHANDLE_HHANDLE_DEFINED 1
PRIVATE DEFINE_KWLIST(libwin32_kwds_nStdHandle_hHandle, { K(nStdHandle), K(hHandle), KEND });
#endif /* !LIBWIN32_KWDS_NSTDHANDLE_HHANDLE_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL libwin32_SetStdHandle_f(size_t argc, DeeObject **argv, DeeObject *kw) {
	DWORD nStdHandle;
	HANDLE hhHandle;
	DeeObject *hHandle;
	if (DeeArg_UnpackKw(argc, argv, kw, libwin32_kwds_nStdHandle_hHandle, "I32uo:SetStdHandle", &nStdHandle, &hHandle))
		goto err;
	if (DeeNTSystem_TryGetHandle(hHandle, (void **)&hhHandle))
		goto err;
	return libwin32_SetStdHandle_f_impl(nStdHandle, hhHandle);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL libwin32_SetStdHandle_f_impl(DWORD nStdHandle, HANDLE hHandle)
//[[[end]]]
{
	BOOL bOk;
again:
	DBG_ALIGNMENT_DISABLE();
	bOk = SetStdHandle(nStdHandle, hHandle);
	if (!bOk) {
		DWORD dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(dwError)) {
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		RETURN_ERROR_OR_FALSE(dwError,
		                      "Failed to set STD handle %I32u to %p",
		                      nStdHandle, hHandle);
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
	                           " - When :none is given, always return ${HANDLE(0)}\n"
	                           " - When an :int is given, return the result of ${get_osfhandle(ob)}\n"
	                           " - When the given object has an attribute $" DeeSysFD_HANDLE_GETSET ", return ${ob." DeeSysFD_HANDLE_GETSET "}\n"
	                           " - When the given object has an attribute $" DeeSysFD_INT_GETSET ", return ${get_osfhandle(ob." DeeSysFD_HANDLE_GETSET ")}\n"
	                           " - When another :HANDLE object is given, re-return that object\n"
	                           "Note that these sames steps are also performed by all other functions taking :HANDLE input arguments")

	/* Error-related functions. */
	LIBWIN32_GETLASTERROR_DEF
	LIBWIN32_SETLASTERROR_DEF
	LIBWIN32_FORMATERRORMESSAGE_DEF_DOC("Return a human-readable error message associated with the "
	                                    "given @dwError (as returned by #GetLastError)")

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
	LIBWIN32_SETDLLDIRECTORY_DEF_DOC("Set the windows DLL directory, as used when loading dynamic libraries, and as returned by #GetDllDirectory")
	LIBWIN32_GETFILETYPE_DEF_DOC("Return one of `FILE_TYPE_*'")
	LIBWIN32_GETFILESIZE_DEF_DOC("Return the size of the given @hFile")
	LIBWIN32_GETDRIVETYPE_DEF_DOC("Returns the type of drive of @lpRootPathName (one of `DRIVE_*')")
	LIBWIN32_GETFILEATTRIBUTES_DEF_DOC("Returns attributes for @lpFileName (set of `FILE_ATTRIBUTE_*')")
	LIBWIN32_SETFILEATTRIBUTES_DEF_DOC("Set attributes for @lpFileName to @dwFileAttributes (set of `FILE_ATTRIBUTE_*')")
	LIBWIN32_GETCOMPRESSEDFILESIZE_DEF
	LIBWIN32_FLUSHFILEBUFFERS_DEF
	LIBWIN32_GETFINALPATHNAMEBYHANDLE_DEF
	LIBWIN32_GETFILENAMEOFHANDLE_DEF_DOC("Convenience wrapper for #GetFinalPathNameByHandle that also supports the "
	                                     "${GetMappedFileName(MapViewOfFile(CreateFileMapping(hFile)))} workaround "
	                                     "that is required on Windows XP, and always tries to return a canonically "
	                                     "correct filename without any $\"\\\\.\\\"-like prefix")

	/* STD handle control */
	LIBWIN32_GETSTDHANDLE_DEF
	LIBWIN32_SETSTDHANDLE_DEF

	/* Memory functions */
	LIBWIN32_MAPVIEWOFFILE_DEF
	LIBWIN32_UNMAPVIEWOFFILE_DEF
	LIBWIN32_CREATEFILEMAPPING_DEF

	/* Process control */
	LIBWIN32_GETCURRENTPROCESS_DEF
	LIBWIN32_GETCURRENTTHREAD_DEF
	LIBWIN32_GETCURRENTPROCESSID_DEF
	LIBWIN32_GETCURRENTTHREADID_DEF

	/* Filesystem functions */
	LIBWIN32_REMOVEDIRECTORY_DEF
	LIBWIN32_CREATEDIRECTORY_DEF
	LIBWIN32_DELETEFILE_DEF
	LIBWIN32_GETSYSTEMDIRECTORY_DEF_DOC("Returns the windows system directory ($\"C:\\\\Windows\\\\system32\")")
	LIBWIN32_GETWINDOWSDIRECTORY_DEF_DOC("Returns the windows system directory ($\"C:\\\\Windows\")")
	LIBWIN32_GETSYSTEMWINDOWSDIRECTORY_DEF_DOC("Returns the system windows directory ($\"C:\\\\Windows\")")
	LIBWIN32_GETSYSTEMWOW64DIRECTORY_DEF_DOC("Returns the windows SysWOW64 directory ($\"C:\\\\Windows\\\\SysWOW64\")")
	LIBWIN32_GETLOGICALDRIVESTRINGS_DEF_DOC("Returns a list of known system drives ($({ r\"C:\\\", r\"D:\\\", r\"E:\\\" }))")
	LIBWIN32_QUERYDOSDEVICE_DEF_DOC("Returns a list of DOS devices mounted under the given drive (which should be one of #GetLogicalDriveStrings)")

	/* DLL functions */
	LIBWIN32_GETMODULEFILENAME_DEF_DOC("Returns :none upon error, or the name of the module")
	LIBWIN32_GETMAPPEDFILENAME_DEF

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
