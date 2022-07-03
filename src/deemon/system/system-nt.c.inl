/* Copyright (c) 2018-2022 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2022 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_SYSTEM_SYSTEM_NT_C_INL
#define GUARD_DEEMON_SYSTEM_SYSTEM_NT_C_INL 1

#include <deemon/api.h>

/* NT-specific system functions */

#ifdef CONFIG_HOST_WINDOWS

#include <deemon/alloc.h>
#include <deemon/error.h>
#include <deemon/error_types.h>
#include <deemon/file.h>
#include <deemon/int.h>
#include <deemon/module.h>
#include <deemon/none.h>
#include <deemon/string.h>
#include <deemon/system-features.h>
#include <deemon/system.h>
#include <deemon/thread.h>

#include <hybrid/atomic.h>
#include <hybrid/unaligned.h>
#include <hybrid/wordbits.h>

#include <Windows.h>

#include "../runtime/strings.h"

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
#endif /* !PATH_MAX */

DECL_BEGIN

#ifdef _WIN32_WCE
#undef GetProcAddress
#define GetProcAddress GetProcAddressA
#endif /* _WIN32_WCE */


#if defined(DeeSysFD_GETSET) && defined(DeeSysFS_IS_HANDLE)
#define GETATTR_osfhandle(ob) DeeObject_GetAttr(ob, (DeeObject *)&str_getsysfd)
#else /* DeeSysFD_GETSET && DeeSysFS_IS_HANDLE */
#define GETATTR_osfhandle(ob) DeeObject_GetAttrString(ob, DeeSysFD_HANDLE_GETSET)
#endif /* !DeeSysFD_GETSET || !DeeSysFS_IS_HANDLE */

#ifndef GETATTR_fileno
#if defined(DeeSysFD_GETSET) && defined(DeeSysFS_IS_FILE)
#define GETATTR_fileno(ob) DeeObject_GetAttr(ob, (DeeObject *)&str_getsysfd)
#else /* DeeSysFD_GETSET && DeeSysFS_IS_FILE */
#define GETATTR_fileno(ob) DeeObject_GetAttrString(ob, DeeSysFD_INT_GETSET)
#endif /* !DeeSysFD_GETSET || !DeeSysFS_IS_FILE */
#endif /* !GETATTR_fileno */


/* Retrieve the Windows handle associated with a given object.
 * The translation is done by performing the following:
 * >> #ifdef DeeSysFS_IS_HANDLE
 * >> if (DeeFile_Check(ob))
 * >>     return DeeFile_GetSysFD(ob);
 * >> #endif
 * >> #ifdef DeeSysFS_IS_INT
 * >> if (DeeFile_Check(ob))
 * >>     return get_osfhandle(DeeFile_GetSysFD(ob));
 * >> #endif
 * >> if (DeeNone_Check(ob))
 * >>     return (void *)(HANDLE)NULL;
 * >> if (DeeInt_Check(ob))
 * >>     return get_osfhandle(DeeInt_AsInt(ob));
 * >> try return DeeObject_AsInt(DeeObject_GetAttr(ob, DeeSysFD_HANDLE_GETSET)); catch (AttributeError);
 * >> try return get_osfhandle(DeeObject_AsInt(DeeObject_GetAttr(ob, DeeSysFD_INT_GETSET))); catch (AttributeError);
 * >> return get_osfhandle(DeeObject_AsInt(ob));
 * Note that both msvc, as well as cygwin define `get_osfhandle()' as one
 * of the available functions, meaning that in both scenarios we are able
 * to get access to the underlying HANDLE. However, should deemon ever be
 * linked against a windows libc without this function, then only the
 * `DeeSysFD_HANDLE_GETSET' variant will be usable.
 * @return: * :                   Success (the actual handle value)
 * @return: INVALID_HANDLE_VALUE: Error (handle translation failed)
 *                                In case the actual handle value stored inside
 *                                of `ob' was `INVALID_HANDLE_VALUE', then an
 *                                `DeeError_FileClosed' error is thrown. */
PUBLIC WUNUSED /*HANDLE*/ void *DCALL
DeeNTSystem_GetHandle(DeeObject *__restrict ob) {
	return DeeNTSystem_GetHandleEx(ob, NULL);
}

/* Same as `DeeNTSystem_GetHandleEx()', but also writes to `p_fd' (when non-NULL):
 * - `-1': If `get_osfhandle()' wasn't used
 * - `*':  The file descriptor number passed to `get_osfhandle()' */
DFUNDEF WUNUSED /*HANDLE*/ void *DCALL
DeeNTSystem_GetHandleEx(DeeObject *__restrict ob, int *p_fd) {
	DREF DeeObject *attr;
#if (defined(DeeSysFS_IS_HANDLE) || \
     (defined(DeeSysFS_IS_INT) && defined(CONFIG_HAVE_get_osfhandle)))
	if (DeeFile_Check(ob)) {
		DeeSysFD sysfd;
		if (p_fd)
			*p_fd = -1;
		sysfd = DeeFile_GetSysFD(ob);
		if (sysfd == DeeSysFD_INVALID)
			return INVALID_HANDLE_VALUE;
#ifdef DeeSysFS_IS_HANDLE
		return (void *)(HANDLE)sysfd;
#endif /* DeeSysFS_IS_HANDLE */

#if defined(DeeSysFS_IS_INT) && defined(CONFIG_HAVE_get_osfhandle)
		{
			HANDLE hResult;
			hResult = (HANDLE)get_osfhandle(sysfd);
			if unlikely(hResult == INVALID_HANDLE_VALUE) {
				DeeError_Throwf(&DeeError_FileClosed,
				                "File descriptor %d bound by %r was closed",
				                sysfd, ob);
			}
			return (void *)hResult;
		}
#endif /* DeeSysFS_IS_INT && CONFIG_HAVE_get_osfhandle */
	}
#endif /* DeeSysFS_IS_HANDLE || DeeSysFS_IS_INT */

#ifdef CONFIG_HAVE_get_osfhandle
	if (DeeInt_Check(ob)) {
		HANDLE hResult;
		int fd;
		if (DeeInt_AsInt(ob, &fd)) {
			if (p_fd)
				*p_fd = -1;
			return INVALID_HANDLE_VALUE;
		}
		if (p_fd)
			*p_fd = fd;
		hResult = (HANDLE)get_osfhandle(fd);
		if unlikely(hResult == INVALID_HANDLE_VALUE) {
			DeeError_Throwf(&DeeError_FileClosed,
			                "File descriptor %d was closed",
			                fd);
		}
		return (void *)hResult;
	}
#endif /* CONFIG_HAVE_get_osfhandle */

	if (DeeNone_Check(ob)) {
		if (p_fd)
			*p_fd = -1;
		return (void *)(HANDLE)NULL;
	}

	attr = GETATTR_osfhandle(ob);
	if (attr) {
		HANDLE hResult;
		int error;
		error = DeeObject_AsUIntptr(attr, (uintptr_t *)&hResult);
		Dee_Decref(attr);
		if unlikely(error)
			goto err;
		if unlikely(hResult == INVALID_HANDLE_VALUE) {
			DeeError_Throwf(&DeeError_FileClosed,
			                "Handle pointed-to by %r was closed",
			                ob);
		}
		if (p_fd)
			*p_fd = -1;
		return hResult;
	}

#ifdef CONFIG_HAVE_get_osfhandle
	if (DeeError_Catch(&DeeError_AttributeError) ||
	    DeeError_Catch(&DeeError_NotImplemented)) {
		HANDLE hResult;
		int fd, error;
		attr = GETATTR_fileno(ob);
		if (attr) {
			error = DeeObject_AsInt(attr, &fd);
			Dee_Decref(attr);
		} else {
			if (!DeeError_Catch(&DeeError_AttributeError) &&
			    !DeeError_Catch(&DeeError_NotImplemented))
				goto err;
			/* Fallback: Convert an `int'-object into a unix file descriptor. */
			error = DeeObject_AsInt(ob, &fd);
		}
		if unlikely(error)
			goto err;
		if (p_fd)
			*p_fd = fd;
		hResult = (HANDLE)get_osfhandle(fd);
		if unlikely(hResult == INVALID_HANDLE_VALUE) {
			DeeError_Throwf(&DeeError_FileClosed,
			                "File descriptor %d bound by %r was closed",
			                fd, ob);
		}
		return (void *)hResult;
	}
#endif /* CONFIG_HAVE_get_osfhandle */

err:
	if (p_fd)
		*p_fd = -1;
	return INVALID_HANDLE_VALUE;
}


/* Similar to `DeeNTSystem_GetHandle()', but allow `ob' to refer to INVALID_HANDLE_VALUE,
 * instead of unconditionally throwing an `DeeError_FileClosed' error when such a handle
 * value is encountered.
 * @return: 0:  Success (the handle value was stored in `*pHandle', and is allowed to be `INVALID_HANDLE_VALUE')
 * @return: -1: Error (a deemon error was thrown; s.a. `DeeError_Throw()') */
PUBLIC WUNUSED int
(DCALL DeeNTSystem_TryGetHandle)(DeeObject *__restrict ob,
                                 /*PHANDLE*/ void **pHandle) {
	DREF DeeObject *attr;
#if (defined(DeeSysFS_IS_HANDLE) || \
     (defined(DeeSysFS_IS_INT) && defined(CONFIG_HAVE_get_osfhandle)))
	if (DeeFile_Check(ob)) {
		DeeSysFD sysfd;
		sysfd = DeeFile_GetSysFD(ob);
		if (sysfd == DeeSysFD_INVALID) {
			if (!DeeError_Catch(&DeeError_FileClosed))
				goto err;
			*pHandle = INVALID_HANDLE_VALUE;
			return 0;
		}
#ifdef DeeSysFS_IS_HANDLE
		*pHandle = (void *)(HANDLE)sysfd;
		return 0;
#endif /* DeeSysFS_IS_HANDLE */

#if defined(DeeSysFS_IS_INT) && defined(CONFIG_HAVE_get_osfhandle)
		*pHandle = (void *)(HANDLE)get_osfhandle(sysfd);
		return 0;
#endif /* DeeSysFS_IS_INT && CONFIG_HAVE_get_osfhandle */
	}
#endif /* DeeSysFS_IS_HANDLE || DeeSysFS_IS_INT */

#ifdef CONFIG_HAVE_get_osfhandle
	if (DeeInt_Check(ob)) {
		int fd;
		if (DeeInt_AsInt(ob, &fd))
			goto err;
		*pHandle = (void *)(HANDLE)get_osfhandle(fd);
		return 0;
	}
#endif /* CONFIG_HAVE_get_osfhandle */

	if (DeeNone_Check(ob)) {
		*pHandle = (void *)(HANDLE)NULL;
		return 0;
	}

	attr = GETATTR_osfhandle(ob);
	if (attr) {
		int result;
		result = DeeObject_AsUIntptr(attr, (uintptr_t *)pHandle);
		Dee_Decref(attr);
		return result;
	}

#ifdef CONFIG_HAVE_get_osfhandle
	if (DeeError_Catch(&DeeError_AttributeError) ||
	    DeeError_Catch(&DeeError_NotImplemented)) {
		int fd, error;
		attr = GETATTR_fileno(ob);
		if (attr) {
			error = DeeObject_AsInt(attr, &fd);
			Dee_Decref(attr);
		} else {
			if (!DeeError_Catch(&DeeError_AttributeError) &&
			    !DeeError_Catch(&DeeError_NotImplemented))
				goto err;
			/* Fallback: Convert an `int'-object into a unix file descriptor. */
			error = DeeObject_AsInt(ob, &fd);
		}
		if unlikely(error)
			goto err;
		*pHandle = (void *)(HANDLE)get_osfhandle(fd);
		return 0;
	}
#endif /* CONFIG_HAVE_get_osfhandle */

err:
	return -1;
}



/* Fix the given filename and extend it to an absolute UNC path. */
PUBLIC WUNUSED NONNULL((1)) DREF /*String*/ DeeObject *DCALL
DeeNTSystem_FixUncPath(/*String*/ DeeObject *__restrict filename) {
	DREF DeeObject *result;
	size_t filename_size;
	ASSERT_OBJECT_TYPE_EXACT(filename, &DeeString_Type);
	filename = DeeSystem_MakeAbsolute(filename);
	if unlikely(!filename)
		goto err;
	filename_size = DeeString_SIZE(filename);
	if (filename_size < 4 ||
	    UNALIGNED_GET32((uint32_t *)DeeString_STR(filename)) != ENCODE_INT32('\\', '\\', '.', '\\')) {
		if (!DeeObject_IsShared(filename) &&
		    DeeString_WIDTH(filename) == STRING_WIDTH_1BYTE) {
			DeeString_FreeWidth(filename);
			result = DeeString_ResizeBuffer(filename, 4 + filename_size);
			if unlikely(!result)
				goto err_filename;
			memmoveupc(DeeString_STR(result) + 4,
			           DeeString_STR(result),
			           filename_size,
			           sizeof(char));
			/* Set the prefix. */
			UNALIGNED_SET32((uint32_t *)DeeString_STR(result),
			                ENCODE_INT32('\\', '\\', '.', '\\'));
			return result;
		} else {
			struct unicode_printer printer = UNICODE_PRINTER_INIT;
			/* Prepend "\\.\". */
			if unlikely(unicode_printer_print8(&printer, (uint8_t *)"\\\\.\\", 4) < 0)
				goto err_printer;
			if unlikely(unicode_printer_printstring(&printer, filename) < 0)
				goto err_printer;
			result = unicode_printer_pack(&printer);
			Dee_Decref(filename);
			return result;
err_printer:
			unicode_printer_fini(&printer);
			Dee_Decref(filename);
			return NULL;
		}
	}
	return filename;
err_filename:
	Dee_Decref(filename);
err:
	return NULL;
}


/* Check if a given error code indicates a UNC-path problem that should be
 * addressed by fixing the path using `DeeNTSystem_FixUncPath()', then trying again. */
PUBLIC ATTR_CONST WUNUSED bool DCALL
DeeNTSystem_IsUncError(DeeNT_DWORD error) {
	switch (error) {

		/* TODO: Figure out the real UNC error codes. */
#ifdef ERROR_FILE_NOT_FOUND
	case ERROR_FILE_NOT_FOUND:
#endif /* ERROR_FILE_NOT_FOUND */
#ifdef ERROR_PATH_NOT_FOUND
	case ERROR_PATH_NOT_FOUND:
#endif /* ERROR_PATH_NOT_FOUND */
#ifdef ERROR_ACCESS_DENIED
	case ERROR_ACCESS_DENIED:
#endif /* ERROR_ACCESS_DENIED */
#ifdef ERROR_INVALID_ACCESS
	/*case ERROR_INVALID_ACCESS:*/
#endif /* ERROR_INVALID_ACCESS */
#ifdef ERROR_INVALID_DRIVE
	case ERROR_INVALID_DRIVE:
#endif /* ERROR_INVALID_DRIVE */
		return true;

	default: break;
	}
	return false;
}

PUBLIC ATTR_CONST WUNUSED bool DCALL
DeeNTSystem_IsFileNotFoundError(DeeNT_DWORD error) {
	switch (error) {

		/* XXX: Check if these are all the possible
		 *      invalid-path / file-not-found errors. */
#ifdef ERROR_FILE_NOT_FOUND
	case ERROR_FILE_NOT_FOUND:
#endif /* ERROR_FILE_NOT_FOUND */
#ifdef ERROR_PATH_NOT_FOUND
	case ERROR_PATH_NOT_FOUND:
#endif /* ERROR_PATH_NOT_FOUND */
#ifdef ERROR_INVALID_DRIVE
	case ERROR_INVALID_DRIVE:
#endif /* ERROR_INVALID_DRIVE */
#ifdef ERROR_BAD_NETPATH
	case ERROR_BAD_NETPATH:
#endif /* ERROR_BAD_NETPATH */
#ifdef ERROR_BAD_PATHNAME
	case ERROR_BAD_PATHNAME:
#endif /* ERROR_BAD_PATHNAME */
#ifdef ERROR_INVALID_NAME
	case ERROR_INVALID_NAME:
#endif /* ERROR_INVALID_NAME */
		return true;

	default: break;
	}
	return false;
}

PUBLIC ATTR_CONST WUNUSED bool DCALL
DeeNTSystem_IsAccessDeniedError(DeeNT_DWORD error) {
	switch (error) {

#ifdef ERROR_ACCESS_DENIED
	case ERROR_ACCESS_DENIED:
#endif /* ERROR_ACCESS_DENIED */
#ifdef ERROR_CANT_ACCESS_FILE
	case ERROR_CANT_ACCESS_FILE:
#endif /* ERROR_CANT_ACCESS_FILE */
#ifdef ERROR_CTX_WINSTATION_ACCESS_DENIED
	case ERROR_CTX_WINSTATION_ACCESS_DENIED:
#endif /* ERROR_CTX_WINSTATION_ACCESS_DENIED */
#ifdef ERROR_DS_DRA_ACCESS_DENIED
	case ERROR_DS_DRA_ACCESS_DENIED:
#endif /* ERROR_DS_DRA_ACCESS_DENIED */
#ifdef ERROR_DS_INSUFF_ACCESS_RIGHTS
	case ERROR_DS_INSUFF_ACCESS_RIGHTS:
#endif /* ERROR_DS_INSUFF_ACCESS_RIGHTS */
#ifdef ERROR_EA_ACCESS_DENIED
	case ERROR_EA_ACCESS_DENIED:
#endif /* ERROR_EA_ACCESS_DENIED */
#ifdef ERROR_HV_ACCESS_DENIED
	case ERROR_HV_ACCESS_DENIED:
#endif /* ERROR_HV_ACCESS_DENIED */
#ifdef ERROR_NETWORK_ACCESS_DENIED
	case ERROR_NETWORK_ACCESS_DENIED:
#endif /* ERROR_NETWORK_ACCESS_DENIED */
#ifdef ERROR_NO_ADMIN_ACCESS_POINT
	case ERROR_NO_ADMIN_ACCESS_POINT:
#endif /* ERROR_NO_ADMIN_ACCESS_POINT */
#ifdef ERROR_VHD_PARENT_VHD_ACCESS_DENIED
	case ERROR_VHD_PARENT_VHD_ACCESS_DENIED:
#endif /* ERROR_VHD_PARENT_VHD_ACCESS_DENIED */
#ifdef ERROR_ACCESS_DISABLED_BY_POLICY
	case ERROR_ACCESS_DISABLED_BY_POLICY:
#endif /* ERROR_ACCESS_DISABLED_BY_POLICY */
#ifdef ERROR_ACCESS_DISABLED_NO_SAFER_UI_BY_POLICY
	case ERROR_ACCESS_DISABLED_NO_SAFER_UI_BY_POLICY:
#endif /* ERROR_ACCESS_DISABLED_NO_SAFER_UI_BY_POLICY */
#ifdef ERROR_ACCESS_DISABLED_WEBBLADE
	case ERROR_ACCESS_DISABLED_WEBBLADE:
#endif /* ERROR_ACCESS_DISABLED_WEBBLADE */
#ifdef ERROR_ACCESS_DISABLED_WEBBLADE_TAMPER
	case ERROR_ACCESS_DISABLED_WEBBLADE_TAMPER:
#endif /* ERROR_ACCESS_DISABLED_WEBBLADE_TAMPER */
#ifdef ERROR_CANT_ACCESS_DOMAIN_INFO
	case ERROR_CANT_ACCESS_DOMAIN_INFO:
#endif /* ERROR_CANT_ACCESS_DOMAIN_INFO */
		return true;

	default: break;
	}
	return false;
}

PUBLIC ATTR_CONST WUNUSED bool DCALL
DeeNTSystem_IsBadAllocError(DeeNT_DWORD error) {
	switch (error) {

#ifdef ERROR_OUTOFMEMORY
	case ERROR_OUTOFMEMORY:
#endif /* !ERROR_OUTOFMEMORY */
#ifdef ERROR_NOT_ENOUGH_SERVER_MEMORY
	case ERROR_NOT_ENOUGH_SERVER_MEMORY:
#endif /* !ERROR_NOT_ENOUGH_SERVER_MEMORY */
#ifdef ERROR_IPSEC_IKE_OUT_OF_MEMORY
	case ERROR_IPSEC_IKE_OUT_OF_MEMORY:
#endif /* !ERROR_IPSEC_IKE_OUT_OF_MEMORY */
#ifdef E_OUTOFMEMORY
	case E_OUTOFMEMORY:
#endif /* !E_OUTOFMEMORY */
#ifdef CO_E_INIT_MEMORY_ALLOCATOR
	case CO_E_INIT_MEMORY_ALLOCATOR:
#endif /* !CO_E_INIT_MEMORY_ALLOCATOR */
#ifdef STG_E_INSUFFICIENTMEMORY
	case STG_E_INSUFFICIENTMEMORY:
#endif /* !STG_E_INSUFFICIENTMEMORY */
#ifdef NTE_NO_MEMORY
	case NTE_NO_MEMORY:
#endif /* !NTE_NO_MEMORY */
#ifdef SEC_E_INSUFFICIENT_MEMORY
	case SEC_E_INSUFFICIENT_MEMORY:
#endif /* !SEC_E_INSUFFICIENT_MEMORY */
#ifdef OSS_OUT_MEMORY
	case OSS_OUT_MEMORY:
#endif /* !OSS_OUT_MEMORY */
#ifdef CRYPT_E_ASN1_MEMORY
	case CRYPT_E_ASN1_MEMORY:
#endif /* !CRYPT_E_ASN1_MEMORY */
#ifdef SCARD_E_NO_MEMORY
	case SCARD_E_NO_MEMORY:
#endif /* !SCARD_E_NO_MEMORY */
#ifdef ERROR_GRAPHICS_NO_VIDEO_MEMORY
	case ERROR_GRAPHICS_NO_VIDEO_MEMORY:
#endif /* !ERROR_GRAPHICS_NO_VIDEO_MEMORY */
#ifdef TPMAPI_E_OUT_OF_MEMORY
	case TPMAPI_E_OUT_OF_MEMORY:
#endif /* !TPMAPI_E_OUT_OF_MEMORY */
#ifdef TBSIMP_E_OUT_OF_MEMORY
	case TBSIMP_E_OUT_OF_MEMORY:
#endif /* !TBSIMP_E_OUT_OF_MEMORY */
#ifdef ERROR_HV_INSUFFICIENT_MEMORY
	case ERROR_HV_INSUFFICIENT_MEMORY:
#endif /* !ERROR_HV_INSUFFICIENT_MEMORY */
#ifdef E_MBN_SMS_MEMORY_FULL
	case E_MBN_SMS_MEMORY_FULL:
#endif /* !E_MBN_SMS_MEMORY_FULL */
#ifdef DXGI_ERROR_REMOTE_OUTOFMEMORY
	case DXGI_ERROR_REMOTE_OUTOFMEMORY:
#endif /* !DXGI_ERROR_REMOTE_OUTOFMEMORY */
#ifdef UCEERR_MEMORYFAILURE
	case UCEERR_MEMORYFAILURE:
#endif /* !UCEERR_MEMORYFAILURE */
#if defined(DNS_ERROR_NO_MEMORY) && DNS_ERROR_NO_MEMORY != ERROR_OUTOFMEMORY
	case DNS_ERROR_NO_MEMORY:
#endif /* !ERROR_OUTOFMEMORY */
		return true;

	default: break;
	}
	return false;
}




PUBLIC ATTR_CONST WUNUSED bool DCALL
DeeNTSystem_IsBusy(DeeNT_DWORD dwError) {
	switch (dwError) {

#ifdef ERROR_CURRENT_DIRECTORY
	case ERROR_CURRENT_DIRECTORY:
#endif /* !ERROR_CURRENT_DIRECTORY */
#ifdef ERROR_BUSY
	case ERROR_BUSY:
#endif /* !ERROR_BUSY */
#ifdef ERROR_NETWORK_BUSY
	case ERROR_NETWORK_BUSY:
#endif /* !ERROR_NETWORK_BUSY */
#ifdef ERROR_DEVICE_SUPPORT_IN_PROGRESS
	case ERROR_DEVICE_SUPPORT_IN_PROGRESS:
#endif /* !ERROR_DEVICE_SUPPORT_IN_PROGRESS */
#ifdef DXGI_ERROR_MODE_CHANGE_IN_PROGRESS
	case DXGI_ERROR_MODE_CHANGE_IN_PROGRESS:
#endif /* !DXGI_ERROR_MODE_CHANGE_IN_PROGRESS */
#ifdef ERROR_CLUSTER_BACKUP_IN_PROGRESS
	case ERROR_CLUSTER_BACKUP_IN_PROGRESS:
#endif /* !ERROR_CLUSTER_BACKUP_IN_PROGRESS */
#ifdef ERROR_CLUSTER_DATABASE_TRANSACTION_IN_PROGRESS
	case ERROR_CLUSTER_DATABASE_TRANSACTION_IN_PROGRESS:
#endif /* !ERROR_CLUSTER_DATABASE_TRANSACTION_IN_PROGRESS */
#ifdef ERROR_CLUSTER_JOIN_IN_PROGRESS
	case ERROR_CLUSTER_JOIN_IN_PROGRESS:
#endif /* !ERROR_CLUSTER_JOIN_IN_PROGRESS */
#ifdef ERROR_CLUSTER_NODE_DRAIN_IN_PROGRESS
	case ERROR_CLUSTER_NODE_DRAIN_IN_PROGRESS:
#endif /* !ERROR_CLUSTER_NODE_DRAIN_IN_PROGRESS */
#ifdef ERROR_DS_DOMAIN_RENAME_IN_PROGRESS
	case ERROR_DS_DOMAIN_RENAME_IN_PROGRESS:
#endif /* !ERROR_DS_DOMAIN_RENAME_IN_PROGRESS */
#ifdef ERROR_DS_PDC_OPERATION_IN_PROGRESS
	case ERROR_DS_PDC_OPERATION_IN_PROGRESS:
#endif /* !ERROR_DS_PDC_OPERATION_IN_PROGRESS */
#ifdef ERROR_FTP_TRANSFER_IN_PROGRESS
	case ERROR_FTP_TRANSFER_IN_PROGRESS:
#endif /* !ERROR_FTP_TRANSFER_IN_PROGRESS */
#ifdef ERROR_GRAPHICS_OPM_SESSION_TYPE_CHANGE_IN_PROGRESS
	case ERROR_GRAPHICS_OPM_SESSION_TYPE_CHANGE_IN_PROGRESS:
#endif /* !ERROR_GRAPHICS_OPM_SESSION_TYPE_CHANGE_IN_PROGRESS */
#ifdef ERROR_GRAPHICS_SESSION_TYPE_CHANGE_IN_PROGRESS
	case ERROR_GRAPHICS_SESSION_TYPE_CHANGE_IN_PROGRESS:
#endif /* !ERROR_GRAPHICS_SESSION_TYPE_CHANGE_IN_PROGRESS */
#ifdef ERROR_JOURNAL_DELETE_IN_PROGRESS
	case ERROR_JOURNAL_DELETE_IN_PROGRESS:
#endif /* !ERROR_JOURNAL_DELETE_IN_PROGRESS */
#ifdef ERROR_LOG_ARCHIVE_IN_PROGRESS
	case ERROR_LOG_ARCHIVE_IN_PROGRESS:
#endif /* !ERROR_LOG_ARCHIVE_IN_PROGRESS */
#ifdef ERROR_LOG_FULL_HANDLER_IN_PROGRESS
	case ERROR_LOG_FULL_HANDLER_IN_PROGRESS:
#endif /* !ERROR_LOG_FULL_HANDLER_IN_PROGRESS */
#ifdef ERROR_NDIS_RESET_IN_PROGRESS
	case ERROR_NDIS_RESET_IN_PROGRESS:
#endif /* !ERROR_NDIS_RESET_IN_PROGRESS */
#ifdef ERROR_OPERATION_IN_PROGRESS
	case ERROR_OPERATION_IN_PROGRESS:
#endif /* !ERROR_OPERATION_IN_PROGRESS */
#ifdef ERROR_OPLOCK_BREAK_IN_PROGRESS
	case ERROR_OPLOCK_BREAK_IN_PROGRESS:
#endif /* !ERROR_OPLOCK_BREAK_IN_PROGRESS */
#ifdef ERROR_RUNLEVEL_SWITCH_IN_PROGRESS
	case ERROR_RUNLEVEL_SWITCH_IN_PROGRESS:
#endif /* !ERROR_RUNLEVEL_SWITCH_IN_PROGRESS */
#ifdef ERROR_SERVER_SHUTDOWN_IN_PROGRESS
	case ERROR_SERVER_SHUTDOWN_IN_PROGRESS:
#endif /* !ERROR_SERVER_SHUTDOWN_IN_PROGRESS */
#ifdef ERROR_SHUTDOWN_IN_PROGRESS
	case ERROR_SHUTDOWN_IN_PROGRESS:
#endif /* !ERROR_SHUTDOWN_IN_PROGRESS */
#ifdef ERROR_TIERING_VOLUME_DISMOUNT_IN_PROGRESS
	case ERROR_TIERING_VOLUME_DISMOUNT_IN_PROGRESS:
#endif /* !ERROR_TIERING_VOLUME_DISMOUNT_IN_PROGRESS */
#ifdef ERROR_TRANSACTION_FREEZE_IN_PROGRESS
	case ERROR_TRANSACTION_FREEZE_IN_PROGRESS:
#endif /* !ERROR_TRANSACTION_FREEZE_IN_PROGRESS */
#ifdef ERROR_VOLMGR_TRANSACTION_IN_PROGRESS
	case ERROR_VOLMGR_TRANSACTION_IN_PROGRESS:
#endif /* !ERROR_VOLMGR_TRANSACTION_IN_PROGRESS */
		return true;

	default: break;
	}
	return false;
}

PUBLIC ATTR_CONST WUNUSED bool DCALL
DeeNTSystem_IsExists(DeeNT_DWORD dwError) {
	switch (dwError) {

#ifdef ERROR_FILE_EXISTS
	case ERROR_FILE_EXISTS:
#endif /* !ERROR_FILE_EXISTS */
#ifdef ERROR_ALREADY_EXISTS
	case ERROR_ALREADY_EXISTS:
#endif /* !ERROR_ALREADY_EXISTS */
#ifdef ERROR_ALIAS_EXISTS
	case ERROR_ALIAS_EXISTS:
#endif /* !ERROR_ALIAS_EXISTS */
#ifdef ERROR_OBJECT_NAME_EXISTS
	case ERROR_OBJECT_NAME_EXISTS:
#endif /* !ERROR_OBJECT_NAME_EXISTS */
		return true;

	default: break;
	}
	return false;
}

PUBLIC ATTR_CONST WUNUSED bool DCALL
DeeNTSystem_IsNotDir(DeeNT_DWORD dwError) {
	switch (dwError) {

#ifdef ERROR_DIRECTORY
	case ERROR_DIRECTORY:
#endif /* !ERROR_DIRECTORY */
#ifdef ERROR_PATH_NOT_FOUND
	case ERROR_PATH_NOT_FOUND:
#endif /* !ERROR_PATH_NOT_FOUND */
#ifdef ERROR_INVALID_DRIVE
	case ERROR_INVALID_DRIVE:
#endif /* !ERROR_INVALID_DRIVE */
		return true;

	default: break;
	}
	return false;
}

PUBLIC ATTR_CONST WUNUSED bool DCALL
DeeNTSystem_IsNotEmpty(DeeNT_DWORD dwError) {
	switch (dwError) {

#ifdef ERROR_DIR_NOT_EMPTY
	case ERROR_DIR_NOT_EMPTY:
#endif /* !ERROR_DIR_NOT_EMPTY */
#ifdef ERROR_NOT_EMPTY
	case ERROR_NOT_EMPTY:
#endif /* !ERROR_NOT_EMPTY */
#ifdef ERROR_TXF_DIR_NOT_EMPTY
	case ERROR_TXF_DIR_NOT_EMPTY:
#endif /* !ERROR_TXF_DIR_NOT_EMPTY */
#ifdef ERROR_VOLMGR_DISK_NOT_EMPTY
	case ERROR_VOLMGR_DISK_NOT_EMPTY:
#endif /* !ERROR_VOLMGR_DISK_NOT_EMPTY */
		return true;

	default: break;
	}
	return false;
}

PUBLIC ATTR_CONST WUNUSED bool DCALL
DeeNTSystem_IsBadF(DeeNT_DWORD dwError) {
	switch (dwError) {

#ifdef ERROR_INVALID_HANDLE
	case ERROR_INVALID_HANDLE:
#endif /* !ERROR_INVALID_HANDLE */
		return true;

	default: break;
	}
	return false;
}

PUBLIC ATTR_CONST WUNUSED bool DCALL
DeeNTSystem_IsXDev(DeeNT_DWORD dwError) {
	switch (dwError) {

#ifdef ERROR_NOT_SAME_DEVICE
	case ERROR_NOT_SAME_DEVICE:
#endif /* !ERROR_NOT_SAME_DEVICE */
		return true;

	default: break;
	}
	return false;
}

PUBLIC ATTR_CONST WUNUSED bool DCALL
DeeNTSystem_IsUnsupportedError(DeeNT_DWORD dwError) {
	switch (dwError) {

#ifdef ERROR_NOT_SUPPORTED
	case ERROR_NOT_SUPPORTED:
#endif /* !ERROR_NOT_SUPPORTED */
#ifdef ERROR_INVALID_FUNCTION
	case ERROR_INVALID_FUNCTION:
#endif /* !ERROR_INVALID_FUNCTION */
#ifdef ERROR_ATOMIC_LOCKS_NOT_SUPPORTED
	case ERROR_ATOMIC_LOCKS_NOT_SUPPORTED:
#endif /* !ERROR_ATOMIC_LOCKS_NOT_SUPPORTED */
#ifdef ERROR_EAS_NOT_SUPPORTED
	case ERROR_EAS_NOT_SUPPORTED:
#endif /* !ERROR_EAS_NOT_SUPPORTED */
#ifdef ERROR_DEVICE_FEATURE_NOT_SUPPORTED
	case ERROR_DEVICE_FEATURE_NOT_SUPPORTED:
#endif /* !ERROR_DEVICE_FEATURE_NOT_SUPPORTED */
#ifdef ERROR_FILE_LEVEL_TRIM_NOT_SUPPORTED
	case ERROR_FILE_LEVEL_TRIM_NOT_SUPPORTED:
#endif /* !ERROR_FILE_LEVEL_TRIM_NOT_SUPPORTED */
#ifdef ERROR_RESIDENT_FILE_NOT_SUPPORTED
	case ERROR_RESIDENT_FILE_NOT_SUPPORTED:
#endif /* !ERROR_RESIDENT_FILE_NOT_SUPPORTED */
#ifdef ERROR_DIRECTORY_NOT_SUPPORTED
	case ERROR_DIRECTORY_NOT_SUPPORTED:
#endif /* !ERROR_DIRECTORY_NOT_SUPPORTED */
#ifdef ERROR_UNSUPPORTED_COMPRESSION
	case ERROR_UNSUPPORTED_COMPRESSION:
#endif /* !ERROR_UNSUPPORTED_COMPRESSION */
#ifdef ERROR_CARDBUS_NOT_SUPPORTED
	case ERROR_CARDBUS_NOT_SUPPORTED:
#endif /* !ERROR_CARDBUS_NOT_SUPPORTED */
#ifdef ERROR_NOT_SUPPORTED_ON_SBS
	case ERROR_NOT_SUPPORTED_ON_SBS:
#endif /* !ERROR_NOT_SUPPORTED_ON_SBS */
#ifdef ERROR_SYMLINK_NOT_SUPPORTED
	case ERROR_SYMLINK_NOT_SUPPORTED:
#endif /* !ERROR_SYMLINK_NOT_SUPPORTED */
#ifdef ERROR_INSTALL_LANGUAGE_UNSUPPORTED
	case ERROR_INSTALL_LANGUAGE_UNSUPPORTED:
#endif /* !ERROR_INSTALL_LANGUAGE_UNSUPPORTED */
#ifdef ERROR_UNSUPPORTED_TYPE
	case ERROR_UNSUPPORTED_TYPE:
#endif /* !ERROR_UNSUPPORTED_TYPE */
#ifdef ERROR_INSTALL_PLATFORM_UNSUPPORTED
	case ERROR_INSTALL_PLATFORM_UNSUPPORTED:
#endif /* !ERROR_INSTALL_PLATFORM_UNSUPPORTED */
#ifdef ERROR_PATCH_PACKAGE_UNSUPPORTED
	case ERROR_PATCH_PACKAGE_UNSUPPORTED:
#endif /* !ERROR_PATCH_PACKAGE_UNSUPPORTED */
#ifdef ERROR_PATCH_REMOVAL_UNSUPPORTED
	case ERROR_PATCH_REMOVAL_UNSUPPORTED:
#endif /* !ERROR_PATCH_REMOVAL_UNSUPPORTED */
#ifdef RPC_S_PROTSEQ_NOT_SUPPORTED
	case RPC_S_PROTSEQ_NOT_SUPPORTED:
#endif /* !RPC_S_PROTSEQ_NOT_SUPPORTED */
#ifdef RPC_S_UNSUPPORTED_TRANS_SYN
	case RPC_S_UNSUPPORTED_TRANS_SYN:
#endif /* !RPC_S_UNSUPPORTED_TRANS_SYN */
#ifdef RPC_S_UNSUPPORTED_TYPE
	case RPC_S_UNSUPPORTED_TYPE:
#endif /* !RPC_S_UNSUPPORTED_TYPE */
#ifdef RPC_S_UNSUPPORTED_NAME_SYNTAX
	case RPC_S_UNSUPPORTED_NAME_SYNTAX:
#endif /* !RPC_S_UNSUPPORTED_NAME_SYNTAX */
#ifdef RPC_S_CANNOT_SUPPORT
	case RPC_S_CANNOT_SUPPORT:
#endif /* !RPC_S_CANNOT_SUPPORT */
#ifdef RPC_S_UNSUPPORTED_AUTHN_LEVEL
	case RPC_S_UNSUPPORTED_AUTHN_LEVEL:
#endif /* !RPC_S_UNSUPPORTED_AUTHN_LEVEL */
#ifdef ERROR_METAFILE_NOT_SUPPORTED
	case ERROR_METAFILE_NOT_SUPPORTED:
#endif /* !ERROR_METAFILE_NOT_SUPPORTED */
#ifdef ERROR_TRANSFORM_NOT_SUPPORTED
	case ERROR_TRANSFORM_NOT_SUPPORTED:
#endif /* !ERROR_TRANSFORM_NOT_SUPPORTED */
#ifdef ERROR_CLIPPING_NOT_SUPPORTED
	case ERROR_CLIPPING_NOT_SUPPORTED:
#endif /* !ERROR_CLIPPING_NOT_SUPPORTED */
#ifdef PEERDIST_ERROR_CONTENTINFO_VERSION_UNSUPPORTED
	case PEERDIST_ERROR_CONTENTINFO_VERSION_UNSUPPORTED:
#endif /* !PEERDIST_ERROR_CONTENTINFO_VERSION_UNSUPPORTED */
#ifdef PEERDIST_ERROR_VERSION_UNSUPPORTED
	case PEERDIST_ERROR_VERSION_UNSUPPORTED:
#endif /* !PEERDIST_ERROR_VERSION_UNSUPPORTED */
#ifdef ERROR_NOT_SUPPORTED_IN_APPCONTAINER
	case ERROR_NOT_SUPPORTED_IN_APPCONTAINER:
#endif /* !ERROR_NOT_SUPPORTED_IN_APPCONTAINER */
#ifdef ERROR_NO_SUPPORTING_DRIVES
	case ERROR_NO_SUPPORTING_DRIVES:
#endif /* !ERROR_NO_SUPPORTING_DRIVES */
#ifdef ERROR_OFFLOAD_READ_FLT_NOT_SUPPORTED
	case ERROR_OFFLOAD_READ_FLT_NOT_SUPPORTED:
#endif /* !ERROR_OFFLOAD_READ_FLT_NOT_SUPPORTED */
#ifdef ERROR_OFFLOAD_WRITE_FLT_NOT_SUPPORTED
	case ERROR_OFFLOAD_WRITE_FLT_NOT_SUPPORTED:
#endif /* !ERROR_OFFLOAD_WRITE_FLT_NOT_SUPPORTED */
#ifdef ERROR_OFFLOAD_READ_FILE_NOT_SUPPORTED
	case ERROR_OFFLOAD_READ_FILE_NOT_SUPPORTED:
#endif /* !ERROR_OFFLOAD_READ_FILE_NOT_SUPPORTED */
#ifdef ERROR_OFFLOAD_WRITE_FILE_NOT_SUPPORTED
	case ERROR_OFFLOAD_WRITE_FILE_NOT_SUPPORTED:
#endif /* !ERROR_OFFLOAD_WRITE_FILE_NOT_SUPPORTED */
#ifdef ERROR_CLUSTER_RESTYPE_NOT_SUPPORTED
	case ERROR_CLUSTER_RESTYPE_NOT_SUPPORTED:
#endif /* !ERROR_CLUSTER_RESTYPE_NOT_SUPPORTED */
#ifdef ERROR_CLUSTER_RESOURCE_CONTAINS_UNSUPPORTED_DIFF_AREA_FOR_SHARED_VOLUMES
	case ERROR_CLUSTER_RESOURCE_CONTAINS_UNSUPPORTED_DIFF_AREA_FOR_SHARED_VOLUMES:
#endif /* !ERROR_CLUSTER_RESOURCE_CONTAINS_UNSUPPORTED_DIFF_AREA_FOR_SHARED_VOLUMES */
#ifdef ERROR_VOLUME_NOT_SUPPORT_EFS
	case ERROR_VOLUME_NOT_SUPPORT_EFS:
#endif /* !ERROR_VOLUME_NOT_SUPPORT_EFS */
#ifdef ERROR_EFS_VERSION_NOT_SUPPORT
	case ERROR_EFS_VERSION_NOT_SUPPORT:
#endif /* !ERROR_EFS_VERSION_NOT_SUPPORT */
#ifdef ERROR_CS_ENCRYPTION_UNSUPPORTED_SERVER
	case ERROR_CS_ENCRYPTION_UNSUPPORTED_SERVER:
#endif /* !ERROR_CS_ENCRYPTION_UNSUPPORTED_SERVER */
#ifdef ERROR_IMPLICIT_TRANSACTION_NOT_SUPPORTED
	case ERROR_IMPLICIT_TRANSACTION_NOT_SUPPORTED:
#endif /* !ERROR_IMPLICIT_TRANSACTION_NOT_SUPPORTED */
#ifdef ERROR_TRANSACTIONS_UNSUPPORTED_REMOTE
	case ERROR_TRANSACTIONS_UNSUPPORTED_REMOTE:
#endif /* !ERROR_TRANSACTIONS_UNSUPPORTED_REMOTE */
#ifdef ERROR_TRANSACTED_MAPPING_UNSUPPORTED_REMOTE
	case ERROR_TRANSACTED_MAPPING_UNSUPPORTED_REMOTE:
#endif /* !ERROR_TRANSACTED_MAPPING_UNSUPPORTED_REMOTE */
#ifdef ERROR_OPERATION_NOT_SUPPORTED_IN_TRANSACTION
	case ERROR_OPERATION_NOT_SUPPORTED_IN_TRANSACTION:
#endif /* !ERROR_OPERATION_NOT_SUPPORTED_IN_TRANSACTION */
#ifdef ERROR_DS_AUTH_METHOD_NOT_SUPPORTED
	case ERROR_DS_AUTH_METHOD_NOT_SUPPORTED:
#endif /* !ERROR_DS_AUTH_METHOD_NOT_SUPPORTED */
#ifdef ERROR_DS_NOT_SUPPORTED
	case ERROR_DS_NOT_SUPPORTED:
#endif /* !ERROR_DS_NOT_SUPPORTED */
#ifdef ERROR_DS_DRA_NOT_SUPPORTED
	case ERROR_DS_DRA_NOT_SUPPORTED:
#endif /* !ERROR_DS_DRA_NOT_SUPPORTED */
#ifdef ERROR_DS_NOT_SUPPORTED_SORT_ORDER
	case ERROR_DS_NOT_SUPPORTED_SORT_ORDER:
#endif /* !ERROR_DS_NOT_SUPPORTED_SORT_ORDER */
#ifdef ERROR_NOT_SUPPORTED_ON_STANDARD_SERVER
	case ERROR_NOT_SUPPORTED_ON_STANDARD_SERVER:
#endif /* !ERROR_NOT_SUPPORTED_ON_STANDARD_SERVER */
#ifdef DNS_ERROR_UNSUPPORTED_ALGORITHM
	case DNS_ERROR_UNSUPPORTED_ALGORITHM:
#endif /* !DNS_ERROR_UNSUPPORTED_ALGORITHM */
#ifdef DNS_ERROR_KSP_DOES_NOT_SUPPORT_PROTECTION
	case DNS_ERROR_KSP_DOES_NOT_SUPPORT_PROTECTION:
#endif /* !DNS_ERROR_KSP_DOES_NOT_SUPPORT_PROTECTION */
#ifdef WSAENOPROTOOPT
	case WSAENOPROTOOPT:
#endif /* !WSAENOPROTOOPT */
#ifdef WSAEPROTONOSUPPORT
	case WSAEPROTONOSUPPORT:
#endif /* !WSAEPROTONOSUPPORT */
#ifdef WSAESOCKTNOSUPPORT
	case WSAESOCKTNOSUPPORT:
#endif /* !WSAESOCKTNOSUPPORT */
#ifdef WSAEOPNOTSUPP
	case WSAEOPNOTSUPP:
#endif /* !WSAEOPNOTSUPP */
#ifdef WSAEPFNOSUPPORT
	case WSAEPFNOSUPPORT:
#endif /* !WSAEPFNOSUPPORT */
#ifdef WSAEAFNOSUPPORT
	case WSAEAFNOSUPPORT:
#endif /* !WSAEAFNOSUPPORT */
#ifdef WSAVERNOTSUPPORTED
	case WSAVERNOTSUPPORTED:
#endif /* !WSAVERNOTSUPPORTED */
#ifdef ERROR_IPSEC_IKE_UNSUPPORTED_ID
	case ERROR_IPSEC_IKE_UNSUPPORTED_ID:
#endif /* !ERROR_IPSEC_IKE_UNSUPPORTED_ID */
#ifdef ERROR_IPSEC_IKE_PEER_DOESNT_SUPPORT_MOBIKE
	case ERROR_IPSEC_IKE_PEER_DOESNT_SUPPORT_MOBIKE:
#endif /* !ERROR_IPSEC_IKE_PEER_DOESNT_SUPPORT_MOBIKE */
#ifdef ERROR_EVT_FILTER_UNSUPPORTEDOP
	case ERROR_EVT_FILTER_UNSUPPORTEDOP:
#endif /* !ERROR_EVT_FILTER_UNSUPPORTEDOP */
#ifdef ERROR_MRM_UNSUPPORTED_DIRECTORY_TYPE
	case ERROR_MRM_UNSUPPORTED_DIRECTORY_TYPE:
#endif /* !ERROR_MRM_UNSUPPORTED_DIRECTORY_TYPE */
#ifdef ERROR_MRM_UNSUPPORTED_PROFILE_TYPE
	case ERROR_MRM_UNSUPPORTED_PROFILE_TYPE:
#endif /* !ERROR_MRM_UNSUPPORTED_PROFILE_TYPE */
#ifdef ERROR_MRM_UNSUPPORTED_FILE_TYPE_FOR_MERGE
	case ERROR_MRM_UNSUPPORTED_FILE_TYPE_FOR_MERGE:
#endif /* !ERROR_MRM_UNSUPPORTED_FILE_TYPE_FOR_MERGE */
#ifdef ERROR_MRM_UNSUPPORTED_FILE_TYPE_FOR_LOAD_UNLOAD_PRI_FILE
	case ERROR_MRM_UNSUPPORTED_FILE_TYPE_FOR_LOAD_UNLOAD_PRI_FILE:
#endif /* !ERROR_MRM_UNSUPPORTED_FILE_TYPE_FOR_LOAD_UNLOAD_PRI_FILE */
#ifdef ERROR_MCA_UNSUPPORTED_MCCS_VERSION
	case ERROR_MCA_UNSUPPORTED_MCCS_VERSION:
#endif /* !ERROR_MCA_UNSUPPORTED_MCCS_VERSION */
#ifdef ERROR_MCA_UNSUPPORTED_COLOR_TEMPERATURE
	case ERROR_MCA_UNSUPPORTED_COLOR_TEMPERATURE:
#endif /* !ERROR_MCA_UNSUPPORTED_COLOR_TEMPERATURE */
#ifdef ERROR_HASH_NOT_SUPPORTED
	case ERROR_HASH_NOT_SUPPORTED:
#endif /* !ERROR_HASH_NOT_SUPPORTED */
#ifdef ERROR_GPIO_VERSION_NOT_SUPPORTED
	case ERROR_GPIO_VERSION_NOT_SUPPORTED:
#endif /* !ERROR_GPIO_VERSION_NOT_SUPPORTED */
#ifdef RO_E_UNSUPPORTED_FROM_MTA
	case RO_E_UNSUPPORTED_FROM_MTA:
#endif /* !RO_E_UNSUPPORTED_FROM_MTA */
#ifdef CO_E_NOT_SUPPORTED
	case CO_E_NOT_SUPPORTED:
#endif /* !CO_E_NOT_SUPPORTED */
#ifdef OLE_E_ADVISENOTSUPPORTED
	case OLE_E_ADVISENOTSUPPORTED:
#endif /* !OLE_E_ADVISENOTSUPPORTED */
#ifdef CACHE_S_FORMATETC_NOTSUPPORTED
	case CACHE_S_FORMATETC_NOTSUPPORTED:
#endif /* !CACHE_S_FORMATETC_NOTSUPPORTED */
#ifdef SCHED_E_UNSUPPORTED_ACCOUNT_OPTION
	case SCHED_E_UNSUPPORTED_ACCOUNT_OPTION:
#endif /* !SCHED_E_UNSUPPORTED_ACCOUNT_OPTION */
#ifdef NTE_NOT_SUPPORTED
	case NTE_NOT_SUPPORTED:
#endif /* !NTE_NOT_SUPPORTED */
#ifdef NTE_HMAC_NOT_SUPPORTED
	case NTE_HMAC_NOT_SUPPORTED:
#endif /* !NTE_HMAC_NOT_SUPPORTED */
#ifdef SEC_E_UNSUPPORTED_FUNCTION
	case SEC_E_UNSUPPORTED_FUNCTION:
#endif /* !SEC_E_UNSUPPORTED_FUNCTION */
#ifdef SEC_E_QOP_NOT_SUPPORTED
	case SEC_E_QOP_NOT_SUPPORTED:
#endif /* !SEC_E_QOP_NOT_SUPPORTED */
#ifdef SEC_E_STRONG_CRYPTO_NOT_SUPPORTED
	case SEC_E_STRONG_CRYPTO_NOT_SUPPORTED:
#endif /* !SEC_E_STRONG_CRYPTO_NOT_SUPPORTED */
#ifdef SEC_E_UNSUPPORTED_PREAUTH
	case SEC_E_UNSUPPORTED_PREAUTH:
#endif /* !SEC_E_UNSUPPORTED_PREAUTH */
#ifdef SEC_E_NO_S4U_PROT_SUPPORT
	case SEC_E_NO_S4U_PROT_SUPPORT:
#endif /* !SEC_E_NO_S4U_PROT_SUPPORT */
#ifdef OSS_INDEFINITE_NOT_SUPPORTED
	case OSS_INDEFINITE_NOT_SUPPORTED:
#endif /* !OSS_INDEFINITE_NOT_SUPPORTED */
#ifdef CERTSRV_E_KEY_ATTESTATION_NOT_SUPPORTED
	case CERTSRV_E_KEY_ATTESTATION_NOT_SUPPORTED:
#endif /* !CERTSRV_E_KEY_ATTESTATION_NOT_SUPPORTED */
#ifdef CERTSRV_E_UNSUPPORTED_CERT_TYPE
	case CERTSRV_E_UNSUPPORTED_CERT_TYPE:
#endif /* !CERTSRV_E_UNSUPPORTED_CERT_TYPE */
#ifdef SPAPI_E_REMOTE_REQUEST_UNSUPPORTED
	case SPAPI_E_REMOTE_REQUEST_UNSUPPORTED:
#endif /* !SPAPI_E_REMOTE_REQUEST_UNSUPPORTED */
#ifdef SCARD_E_READER_UNSUPPORTED
	case SCARD_E_READER_UNSUPPORTED:
#endif /* !SCARD_E_READER_UNSUPPORTED */
#ifdef SCARD_E_CARD_UNSUPPORTED
	case SCARD_E_CARD_UNSUPPORTED:
#endif /* !SCARD_E_CARD_UNSUPPORTED */
#ifdef SCARD_E_UNSUPPORTED_FEATURE
	case SCARD_E_UNSUPPORTED_FEATURE:
#endif /* !SCARD_E_UNSUPPORTED_FEATURE */
#ifdef SCARD_W_UNSUPPORTED_CARD
	case SCARD_W_UNSUPPORTED_CARD:
#endif /* !SCARD_W_UNSUPPORTED_CARD */
		return true;

	default: break;
	}
	return false;
}

PUBLIC ATTR_CONST WUNUSED bool DCALL
DeeNTSystem_IsIntr(/*DWORD*/ DeeNT_DWORD dwError) {
	switch (dwError) {

#ifdef ERROR_OPERATION_ABORTED
	case ERROR_OPERATION_ABORTED:
#endif /* !ERROR_OPERATION_ABORTED */
		return true;

	default: break;
	}
	return false;
}

PUBLIC ATTR_CONST WUNUSED bool DCALL
DeeNTSystem_IsBufferTooSmall(/*DWORD*/ DeeNT_DWORD dwError) {
	switch (dwError) {

#ifdef ERROR_BUFFER_OVERFLOW
	case ERROR_BUFFER_OVERFLOW:
#endif /* ERROR_BUFFER_OVERFLOW */
#ifdef ERROR_INSUFFICIENT_BUFFER
	case ERROR_INSUFFICIENT_BUFFER:
#endif /* ERROR_INSUFFICIENT_BUFFER */
#ifdef ERROR_MORE_DATA
	case ERROR_MORE_DATA:
#endif /* ERROR_MORE_DATA */
		return true;

	default: break;
	}
	return false;
}

PUBLIC ATTR_CONST WUNUSED bool DCALL
DeeNTSystem_IsInvalidArgument(/*DWORD*/ DeeNT_DWORD dwError) {
	switch (dwError) {

#ifdef ERROR_BAD_USERNAME
	case ERROR_BAD_USERNAME:
#endif /* ERROR_BAD_USERNAME */
#ifdef ERROR_EA_LIST_INCONSISTENT
	case ERROR_EA_LIST_INCONSISTENT:
#endif /* ERROR_EA_LIST_INCONSISTENT */
#ifdef ERROR_INVALID_ADDRESS
	case ERROR_INVALID_ADDRESS:
#endif /* ERROR_INVALID_ADDRESS */
#ifdef ERROR_INVALID_EA_NAME
	case ERROR_INVALID_EA_NAME:
#endif /* ERROR_INVALID_EA_NAME */
#ifdef ERROR_INVALID_SIGNAL_NUMBER
	case ERROR_INVALID_SIGNAL_NUMBER:
#endif /* ERROR_INVALID_SIGNAL_NUMBER */
#ifdef ERROR_META_EXPANSION_TOO_LONG
	case ERROR_META_EXPANSION_TOO_LONG:
#endif /* ERROR_META_EXPANSION_TOO_LONG */
#ifdef ERROR_NONE_MAPPED
	case ERROR_NONE_MAPPED:
#endif /* ERROR_NONE_MAPPED */
#ifdef ERROR_NO_TOKEN
	case ERROR_NO_TOKEN:
#endif /* ERROR_NO_TOKEN */
#ifdef ERROR_SECTOR_NOT_FOUND
	case ERROR_SECTOR_NOT_FOUND:
#endif /* ERROR_SECTOR_NOT_FOUND */
#ifdef ERROR_SEEK
	case ERROR_SEEK:
#endif /* ERROR_SEEK */
#ifdef ERROR_THREAD_1_INACTIVE
	case ERROR_THREAD_1_INACTIVE:
#endif /* ERROR_THREAD_1_INACTIVE */
#ifdef ERROR_INVALID_ACCESS
	case ERROR_INVALID_ACCESS:
#endif /* ERROR_INVALID_ACCESS */
#ifdef ERROR_INVALID_DATA
	case ERROR_INVALID_DATA:
#endif /* ERROR_INVALID_DATA */
#ifdef ERROR_INVALID_PARAMETER
	case ERROR_INVALID_PARAMETER:
#endif /* ERROR_INVALID_PARAMETER */
#ifdef ERROR_NEGATIVE_SEEK
	case ERROR_NEGATIVE_SEEK:
#endif /* ERROR_NEGATIVE_SEEK */
#ifdef ERROR_INVALID_FUNCTION
	case ERROR_INVALID_FUNCTION:
#endif /* ERROR_INVALID_FUNCTION */
		return true;

	default: break;
	}
	return false;
}

PUBLIC ATTR_CONST WUNUSED bool DCALL
DeeNTSystem_IsNoLink(/*DWORD*/ DeeNT_DWORD dwError) {
	switch (dwError) {

#ifdef ERROR_NOT_A_REPARSE_POINT
	case ERROR_NOT_A_REPARSE_POINT:
#endif /* ERROR_NOT_A_REPARSE_POINT */
		return true;

	default: break;
	}
	return false;
}



/* Figure out how to implement `DeeNTSystem_TranslateErrno()' */
#undef DeeNTSystem_TranslateErrno_USE_ERRNO_NT2KOS
#undef DeeNTSystem_TranslateErrno_USE_DOSMAPERR
#undef DeeNTSystem_TranslateErrno_USE_FALLBACK
#if defined(CONFIG_HAVE_errno_nt2kos)
#define DeeNTSystem_TranslateErrno_USE_ERRNO_NT2KOS 1
#elif defined(CONFIG_HAVE__dosmaperr) && defined(CONFIG_HAVE_errno) && 0
#define DeeNTSystem_TranslateErrno_USE_DOSMAPERR 1
#else
#define DeeNTSystem_TranslateErrno_USE_FALLBACK 1
#endif


/* Figure out how to implement `DeeNTSystem_TranslateErrno()' */
#undef DeeNTSystem_TranslateNtError_USE_ERRNO_KOS2NT
#undef DeeNTSystem_TranslateNtError_USE_FALLBACK
#if defined(CONFIG_HAVE_errno_kos2nt)
#define DeeNTSystem_TranslateNtError_USE_ERRNO_KOS2NT 1
#else
#define DeeNTSystem_TranslateNtError_USE_FALLBACK 1
#endif


#ifdef DeeNTSystem_TranslateErrno_USE_ERRNO_NT2KOS
#ifndef __errno_nt2kos_defined
#ifndef __LIBCCALL
#define __LIBCCALL /* nothing */
#endif /* !__LIBCCALL */
extern ATTR_DLLIMPORT /*errno_t*/ int __LIBCCALL errno_nt2kos(DeeNT_DWORD dwError);
#endif /* !__errno_nt2kos_defined */
#endif /* DeeNTSystem_TranslateErrno_USE_ERRNO_NT2KOS */

#ifdef DeeNTSystem_TranslateNtError_USE_ERRNO_KOS2NT
#ifndef __errno_kos2nt_defined
#ifndef __LIBCCALL
#define __LIBCCALL /* nothing */
#endif /* !__LIBCCALL */
extern ATTR_DLLIMPORT DeeNT_DWORD __LIBCCALL errno_kos2nt(/*errno_t*/ int errno_value);
#endif /* !__errno_kos2nt_defined */
#endif /* DeeNTSystem_TranslateNtError_USE_ERRNO_KOS2NT */

#ifdef DeeNTSystem_TranslateErrno_USE_DOSMAPERR
#ifdef CONFIG_HAVE__doserrno
#define IF_CONFIG_HAVE__doserrno(x) x
#else /* CONFIG_HAVE__doserrno */
#define IF_CONFIG_HAVE__doserrno(x) /* nothing */
#endif /* !CONFIG_HAVE__doserrno */
extern ATTR_DLLIMPORT void ATTR_CDECL _dosmaperr(DeeNT_DWORD dwError);
#endif /* DeeNTSystem_TranslateErrno_USE_DOSMAPERR */

#if (defined(DeeNTSystem_TranslateErrno_USE_FALLBACK) || \
     defined(DeeNTSystem_TranslateNtError_USE_FALLBACK))
struct nt2errno_ent {
	DeeNT_DWORD     etval; /* NT error code */
	/*errno_t*/ int erval; /* errno code */
};

#if defined(__CYGWIN32__) && !defined(__CYGWIN__)
#define __CYGWIN__  __CYGWIN32__
#endif /* __CYGWIN32__ && !__CYGWIN__ */

#ifdef _MSC_VER
#define NT2ERRNO_PREFER_MSVC 1
#elif defined(__CYGWIN__)
#undef NT2ERRNO_PREFER_MSVC
#elif 1 /* Fallback */
#undef NT2ERRNO_PREFER_MSVC
#else
#define NT2ERRNO_PREFER_MSVC 1
#endif
#define NT2ERRNO_SRC_CYGWIN 1 /* Include cygwin-specific translations */
#define NT2ERRNO_SRC_MSVC   1 /* Include msvc-specific translations */

PRIVATE struct nt2errno_ent const nt2errno[] = {

	/* NT error codes that only appear in CYGWIN's translation table. */
#ifdef NT2ERRNO_SRC_CYGWIN
#if defined(ERROR_ACTIVE_CONNECTIONS) && defined(EAGAIN)
	{ ERROR_ACTIVE_CONNECTIONS, EAGAIN },
#endif /* ERROR_ACTIVE_CONNECTIONS && EAGAIN */
#if defined(ERROR_BAD_DEVICE) && defined(ENODEV)
	{ ERROR_BAD_DEVICE, ENODEV },
#endif /* ERROR_BAD_DEVICE && ENODEV */
#if defined(ERROR_BAD_EXE_FORMAT) && defined(ENOEXEC)
	{ ERROR_BAD_EXE_FORMAT, ENOEXEC },
#endif /* ERROR_BAD_EXE_FORMAT && ENOEXEC */
#if defined(ERROR_BAD_NET_RESP) && defined(ENOSYS)
	{ ERROR_BAD_NET_RESP, ENOSYS },
#endif /* ERROR_BAD_NET_RESP && ENOSYS */
#if defined(ERROR_BAD_PIPE) && defined(EINVAL)
	{ ERROR_BAD_PIPE, EINVAL },
#endif /* ERROR_BAD_PIPE && EINVAL */
#if defined(ERROR_BAD_UNIT) && defined(ENODEV)
	{ ERROR_BAD_UNIT, ENODEV },
#endif /* ERROR_BAD_UNIT && ENODEV */
#if defined(ERROR_BAD_USERNAME) && defined(EINVAL)
	{ ERROR_BAD_USERNAME, EINVAL },
#endif /* ERROR_BAD_USERNAME && EINVAL */
#if defined(ERROR_BEGINNING_OF_MEDIA) && defined(EIO)
	{ ERROR_BEGINNING_OF_MEDIA, EIO },
#endif /* ERROR_BEGINNING_OF_MEDIA && EIO */
#if defined(ERROR_BUSY) && defined(EBUSY)
	{ ERROR_BUSY, EBUSY },
#endif /* ERROR_BUSY && EBUSY */
#if defined(ERROR_BUS_RESET) && defined(EIO)
	{ ERROR_BUS_RESET, EIO },
#endif /* ERROR_BUS_RESET && EIO */
#if defined(ERROR_CALL_NOT_IMPLEMENTED) && defined(ENOSYS)
	{ ERROR_CALL_NOT_IMPLEMENTED, ENOSYS },
#endif /* ERROR_CALL_NOT_IMPLEMENTED && ENOSYS */
#if defined(ERROR_CANCELLED) && defined(EINTR)
	{ ERROR_CANCELLED, EINTR },
#endif /* ERROR_CANCELLED && EINTR */
#if defined(ERROR_COMMITMENT_LIMIT) && defined(EAGAIN)
	{ ERROR_COMMITMENT_LIMIT, EAGAIN },
#endif /* ERROR_COMMITMENT_LIMIT && EAGAIN */
#if defined(ERROR_CONNECTION_REFUSED) && defined(ECONNREFUSED)
	{ ERROR_CONNECTION_REFUSED, ECONNREFUSED },
#endif /* ERROR_CONNECTION_REFUSED && ECONNREFUSED */
#if defined(ERROR_CRC) && defined(EIO)
	{ ERROR_CRC, EIO },
#endif /* ERROR_CRC && EIO */
#if defined(ERROR_DEVICE_DOOR_OPEN) && defined(EIO)
	{ ERROR_DEVICE_DOOR_OPEN, EIO },
#endif /* ERROR_DEVICE_DOOR_OPEN && EIO */
#if defined(ERROR_DEVICE_IN_USE) && defined(EAGAIN)
	{ ERROR_DEVICE_IN_USE, EAGAIN },
#endif /* ERROR_DEVICE_IN_USE && EAGAIN */
#if defined(ERROR_DEVICE_REQUIRES_CLEANING) && defined(EIO)
	{ ERROR_DEVICE_REQUIRES_CLEANING, EIO },
#endif /* ERROR_DEVICE_REQUIRES_CLEANING && EIO */
#if defined(ERROR_DEV_NOT_EXIST) && defined(ENOENT)
	{ ERROR_DEV_NOT_EXIST, ENOENT },
#endif /* ERROR_DEV_NOT_EXIST && ENOENT */
#if defined(ERROR_DIRECTORY) && defined(ENOTDIR)
	{ ERROR_DIRECTORY, ENOTDIR },
#endif /* ERROR_DIRECTORY && ENOTDIR */
#if defined(ERROR_DISK_CORRUPT) && defined(EIO)
	{ ERROR_DISK_CORRUPT, EIO },
#endif /* ERROR_DISK_CORRUPT && EIO */
#if defined(ERROR_DS_GENERIC_ERROR) && defined(EIO)
	{ ERROR_DS_GENERIC_ERROR, EIO },
#endif /* ERROR_DS_GENERIC_ERROR && EIO */
#if defined(ERROR_DUP_NAME) && defined(ENOTUNIQ)
	{ ERROR_DUP_NAME, ENOTUNIQ },
#endif /* ERROR_DUP_NAME && ENOTUNIQ */
#if defined(ERROR_EAS_DIDNT_FIT) && defined(ENOSPC)
	{ ERROR_EAS_DIDNT_FIT, ENOSPC },
#endif /* ERROR_EAS_DIDNT_FIT && ENOSPC */
#if defined(ERROR_EAS_NOT_SUPPORTED) && defined(ENOTSUP)
	{ ERROR_EAS_NOT_SUPPORTED, ENOTSUP },
#endif /* ERROR_EAS_NOT_SUPPORTED && ENOTSUP */
#if defined(ERROR_EA_LIST_INCONSISTENT) && defined(EINVAL)
	{ ERROR_EA_LIST_INCONSISTENT, EINVAL },
#endif /* ERROR_EA_LIST_INCONSISTENT && EINVAL */
#if defined(ERROR_EA_TABLE_FULL) && defined(ENOSPC)
	{ ERROR_EA_TABLE_FULL, ENOSPC },
#endif /* ERROR_EA_TABLE_FULL && ENOSPC */
#if defined(ERROR_END_OF_MEDIA) && defined(ENOSPC)
	{ ERROR_END_OF_MEDIA, ENOSPC },
#endif /* ERROR_END_OF_MEDIA && ENOSPC */
#if defined(ERROR_EOM_OVERFLOW) && defined(EIO)
	{ ERROR_EOM_OVERFLOW, EIO },
#endif /* ERROR_EOM_OVERFLOW && EIO */
#if defined(ERROR_EXE_MACHINE_TYPE_MISMATCH) && defined(ENOEXEC)
	{ ERROR_EXE_MACHINE_TYPE_MISMATCH, ENOEXEC },
#endif /* ERROR_EXE_MACHINE_TYPE_MISMATCH && ENOEXEC */
#if defined(ERROR_EXE_MARKED_INVALID) && defined(ENOEXEC)
	{ ERROR_EXE_MARKED_INVALID, ENOEXEC },
#endif /* ERROR_EXE_MARKED_INVALID && ENOEXEC */
#if defined(ERROR_FILEMARK_DETECTED) && defined(EIO)
	{ ERROR_FILEMARK_DETECTED, EIO },
#endif /* ERROR_FILEMARK_DETECTED && EIO */
#if defined(ERROR_FILE_CORRUPT) && defined(EEXIST)
	{ ERROR_FILE_CORRUPT, EEXIST },
#endif /* ERROR_FILE_CORRUPT && EEXIST */
#if defined(ERROR_FILE_INVALID) && defined(ENXIO)
	{ ERROR_FILE_INVALID, ENXIO },
#endif /* ERROR_FILE_INVALID && ENXIO */
#if defined(ERROR_HANDLE_DISK_FULL) && defined(ENOSPC)
	{ ERROR_HANDLE_DISK_FULL, ENOSPC },
#endif /* ERROR_HANDLE_DISK_FULL && ENOSPC */
#if defined(ERROR_HANDLE_EOF) && defined(ENODATA)
	{ ERROR_HANDLE_EOF, ENODATA },
#endif /* ERROR_HANDLE_EOF && ENODATA */
#if defined(ERROR_INVALID_ADDRESS) && defined(EINVAL)
	{ ERROR_INVALID_ADDRESS, EINVAL },
#endif /* ERROR_INVALID_ADDRESS && EINVAL */
#if defined(ERROR_INVALID_AT_INTERRUPT_TIME) && defined(EINTR)
	{ ERROR_INVALID_AT_INTERRUPT_TIME, EINTR },
#endif /* ERROR_INVALID_AT_INTERRUPT_TIME && EINTR */
#if defined(ERROR_INVALID_BLOCK_LENGTH) && defined(EIO)
	{ ERROR_INVALID_BLOCK_LENGTH, EIO },
#endif /* ERROR_INVALID_BLOCK_LENGTH && EIO */
#if defined(ERROR_INVALID_EA_NAME) && defined(EINVAL)
	{ ERROR_INVALID_EA_NAME, EINVAL },
#endif /* ERROR_INVALID_EA_NAME && EINVAL */
#if defined(ERROR_INVALID_EXE_SIGNATURE) && defined(ENOEXEC)
	{ ERROR_INVALID_EXE_SIGNATURE, ENOEXEC },
#endif /* ERROR_INVALID_EXE_SIGNATURE && ENOEXEC */
#if defined(ERROR_INVALID_NAME) && defined(ENOENT)
	{ ERROR_INVALID_NAME, ENOENT },
#endif /* ERROR_INVALID_NAME && ENOENT */
#if defined(ERROR_INVALID_SIGNAL_NUMBER) && defined(EINVAL)
	{ ERROR_INVALID_SIGNAL_NUMBER, EINVAL },
#endif /* ERROR_INVALID_SIGNAL_NUMBER && EINVAL */
#if defined(ERROR_IOPL_NOT_ENABLED) && defined(ENOEXEC)
	{ ERROR_IOPL_NOT_ENABLED, ENOEXEC },
#endif /* ERROR_IOPL_NOT_ENABLED && ENOEXEC */
#if defined(ERROR_IO_DEVICE) && defined(EIO)
	{ ERROR_IO_DEVICE, EIO },
#endif /* ERROR_IO_DEVICE && EIO */
#if defined(ERROR_IO_INCOMPLETE) && defined(EAGAIN)
	{ ERROR_IO_INCOMPLETE, EAGAIN },
#endif /* ERROR_IO_INCOMPLETE && EAGAIN */
#if defined(ERROR_IO_PENDING) && defined(EAGAIN)
	{ ERROR_IO_PENDING, EAGAIN },
#endif /* ERROR_IO_PENDING && EAGAIN */
#if defined(ERROR_META_EXPANSION_TOO_LONG) && defined(EINVAL)
	{ ERROR_META_EXPANSION_TOO_LONG, EINVAL },
#endif /* ERROR_META_EXPANSION_TOO_LONG && EINVAL */
#if defined(ERROR_MOD_NOT_FOUND) && defined(ENOENT)
	{ ERROR_MOD_NOT_FOUND, ENOENT },
#endif /* ERROR_MOD_NOT_FOUND && ENOENT */
#if defined(ERROR_MORE_DATA) && defined(EMSGSIZE)
	{ ERROR_MORE_DATA, EMSGSIZE },
#endif /* ERROR_MORE_DATA && EMSGSIZE */
#if defined(ERROR_NETNAME_DELETED) && defined(ENOENT)
	{ ERROR_NETNAME_DELETED, ENOENT },
#endif /* ERROR_NETNAME_DELETED && ENOENT */
#if defined(ERROR_NOACCESS) && defined(EFAULT)
	{ ERROR_NOACCESS, EFAULT },
#endif /* ERROR_NOACCESS && EFAULT */
#if defined(ERROR_NONE_MAPPED) && defined(EINVAL)
	{ ERROR_NONE_MAPPED, EINVAL },
#endif /* ERROR_NONE_MAPPED && EINVAL */
#if defined(ERROR_NONPAGED_SYSTEM_RESOURCES) && defined(EAGAIN)
	{ ERROR_NONPAGED_SYSTEM_RESOURCES, EAGAIN },
#endif /* ERROR_NONPAGED_SYSTEM_RESOURCES && EAGAIN */
#if defined(ERROR_NOT_CONNECTED) && defined(ENOLINK)
	{ ERROR_NOT_CONNECTED, ENOLINK },
#endif /* ERROR_NOT_CONNECTED && ENOLINK */
#if defined(ERROR_NOT_OWNER) && defined(EPERM)
	{ ERROR_NOT_OWNER, EPERM },
#endif /* ERROR_NOT_OWNER && EPERM */
#if defined(ERROR_NOT_READY) && defined(ENOMEDIUM)
	{ ERROR_NOT_READY, ENOMEDIUM },
#endif /* ERROR_NOT_READY && ENOMEDIUM */
#if defined(ERROR_NOT_SUPPORTED) && defined(ENOSYS)
	{ ERROR_NOT_SUPPORTED, ENOSYS },
#endif /* ERROR_NOT_SUPPORTED && ENOSYS */
#if defined(ERROR_NO_DATA) && defined(EPIPE)
	{ ERROR_NO_DATA, EPIPE },
#endif /* ERROR_NO_DATA && EPIPE */
#if defined(ERROR_NO_DATA_DETECTED) && defined(EIO)
	{ ERROR_NO_DATA_DETECTED, EIO },
#endif /* ERROR_NO_DATA_DETECTED && EIO */
#if defined(ERROR_NO_MEDIA_IN_DRIVE) && defined(ENOMEDIUM)
	{ ERROR_NO_MEDIA_IN_DRIVE, ENOMEDIUM },
#endif /* ERROR_NO_MEDIA_IN_DRIVE && ENOMEDIUM */
#if defined(ERROR_NO_MORE_ITEMS) && defined(ENMFILE)
	{ ERROR_NO_MORE_ITEMS, ENMFILE },
#endif /* ERROR_NO_MORE_ITEMS && ENMFILE */
#if defined(ERROR_NO_MORE_SEARCH_HANDLES) && defined(ENFILE)
	{ ERROR_NO_MORE_SEARCH_HANDLES, ENFILE },
#endif /* ERROR_NO_MORE_SEARCH_HANDLES && ENFILE */
#if defined(ERROR_NO_SIGNAL_SENT) && defined(EIO)
	{ ERROR_NO_SIGNAL_SENT, EIO },
#endif /* ERROR_NO_SIGNAL_SENT && EIO */
#if defined(ERROR_NO_SYSTEM_RESOURCES) && defined(EFBIG)
	{ ERROR_NO_SYSTEM_RESOURCES, EFBIG },
#endif /* ERROR_NO_SYSTEM_RESOURCES && EFBIG */
#if defined(ERROR_NO_TOKEN) && defined(EINVAL)
	{ ERROR_NO_TOKEN, EINVAL },
#endif /* ERROR_NO_TOKEN && EINVAL */
#if defined(ERROR_OPEN_FAILED) && defined(EIO)
	{ ERROR_OPEN_FAILED, EIO },
#endif /* ERROR_OPEN_FAILED && EIO */
#if defined(ERROR_OPEN_FILES) && defined(EAGAIN)
	{ ERROR_OPEN_FILES, EAGAIN },
#endif /* ERROR_OPEN_FILES && EAGAIN */
#if defined(ERROR_OUTOFMEMORY) && defined(ENOMEM)
	{ ERROR_OUTOFMEMORY, ENOMEM },
#endif /* ERROR_OUTOFMEMORY && ENOMEM */
#if defined(ERROR_PAGED_SYSTEM_RESOURCES) && defined(EAGAIN)
	{ ERROR_PAGED_SYSTEM_RESOURCES, EAGAIN },
#endif /* ERROR_PAGED_SYSTEM_RESOURCES && EAGAIN */
#if defined(ERROR_PAGEFILE_QUOTA) && defined(EAGAIN)
	{ ERROR_PAGEFILE_QUOTA, EAGAIN },
#endif /* ERROR_PAGEFILE_QUOTA && EAGAIN */
#if defined(ERROR_PIPE_BUSY) && defined(EBUSY)
	{ ERROR_PIPE_BUSY, EBUSY },
#endif /* ERROR_PIPE_BUSY && EBUSY */
#if defined(ERROR_PIPE_CONNECTED) && defined(EBUSY)
	{ ERROR_PIPE_CONNECTED, EBUSY },
#endif /* ERROR_PIPE_CONNECTED && EBUSY */
#if defined(ERROR_PIPE_LISTENING) && defined(ECOMM)
	{ ERROR_PIPE_LISTENING, ECOMM },
#endif /* ERROR_PIPE_LISTENING && ECOMM */
#if defined(ERROR_PIPE_NOT_CONNECTED) && defined(ECOMM)
	{ ERROR_PIPE_NOT_CONNECTED, ECOMM },
#endif /* ERROR_PIPE_NOT_CONNECTED && ECOMM */
#if defined(ERROR_POSSIBLE_DEADLOCK) && defined(EDEADLOCK)
	{ ERROR_POSSIBLE_DEADLOCK, EDEADLOCK },
#endif /* ERROR_POSSIBLE_DEADLOCK && EDEADLOCK */
#if defined(ERROR_PRIVILEGE_NOT_HELD) && defined(EPERM)
	{ ERROR_PRIVILEGE_NOT_HELD, EPERM },
#endif /* ERROR_PRIVILEGE_NOT_HELD && EPERM */
#if defined(ERROR_PROCESS_ABORTED) && defined(EFAULT)
	{ ERROR_PROCESS_ABORTED, EFAULT },
#endif /* ERROR_PROCESS_ABORTED && EFAULT */
#if defined(ERROR_PROC_NOT_FOUND) && defined(ESRCH)
	{ ERROR_PROC_NOT_FOUND, ESRCH },
#endif /* ERROR_PROC_NOT_FOUND && ESRCH */
#if defined(ERROR_REM_NOT_LIST) && defined(ENONET)
	{ ERROR_REM_NOT_LIST, ENONET },
#endif /* ERROR_REM_NOT_LIST && ENONET */
#if defined(ERROR_SECTOR_NOT_FOUND) && defined(EINVAL)
	{ ERROR_SECTOR_NOT_FOUND, EINVAL },
#endif /* ERROR_SECTOR_NOT_FOUND && EINVAL */
#if defined(ERROR_SEEK) && defined(EINVAL)
	{ ERROR_SEEK, EINVAL },
#endif /* ERROR_SEEK && EINVAL */
#if defined(ERROR_SERVICE_REQUEST_TIMEOUT) && defined(EBUSY)
	{ ERROR_SERVICE_REQUEST_TIMEOUT, EBUSY },
#endif /* ERROR_SERVICE_REQUEST_TIMEOUT && EBUSY */
#if defined(ERROR_SETMARK_DETECTED) && defined(EIO)
	{ ERROR_SETMARK_DETECTED, EIO },
#endif /* ERROR_SETMARK_DETECTED && EIO */
#if defined(ERROR_SHARING_BUFFER_EXCEEDED) && defined(ENOLCK)
	{ ERROR_SHARING_BUFFER_EXCEEDED, ENOLCK },
#endif /* ERROR_SHARING_BUFFER_EXCEEDED && ENOLCK */
#if defined(ERROR_SHARING_VIOLATION) && defined(EBUSY)
	{ ERROR_SHARING_VIOLATION, EBUSY },
#endif /* ERROR_SHARING_VIOLATION && EBUSY */
#if defined(ERROR_SIGNAL_PENDING) && defined(EBUSY)
	{ ERROR_SIGNAL_PENDING, EBUSY },
#endif /* ERROR_SIGNAL_PENDING && EBUSY */
#if defined(ERROR_SIGNAL_REFUSED) && defined(EIO)
	{ ERROR_SIGNAL_REFUSED, EIO },
#endif /* ERROR_SIGNAL_REFUSED && EIO */
#if defined(ERROR_SXS_CANT_GEN_ACTCTX) && defined(ELIBBAD)
	{ ERROR_SXS_CANT_GEN_ACTCTX, ELIBBAD },
#endif /* ERROR_SXS_CANT_GEN_ACTCTX && ELIBBAD */
#if defined(ERROR_THREAD_1_INACTIVE) && defined(EINVAL)
	{ ERROR_THREAD_1_INACTIVE, EINVAL },
#endif /* ERROR_THREAD_1_INACTIVE && EINVAL */
#if defined(ERROR_TIMEOUT) && defined(EBUSY)
	{ ERROR_TIMEOUT, EBUSY },
#endif /* ERROR_TIMEOUT && EBUSY */
#if defined(ERROR_TOO_MANY_LINKS) && defined(EMLINK)
	{ ERROR_TOO_MANY_LINKS, EMLINK },
#endif /* ERROR_TOO_MANY_LINKS && EMLINK */
#if defined(ERROR_UNEXP_NET_ERR) && defined(EIO)
	{ ERROR_UNEXP_NET_ERR, EIO },
#endif /* ERROR_UNEXP_NET_ERR && EIO */
#if defined(ERROR_WORKING_SET_QUOTA) && defined(EAGAIN)
	{ ERROR_WORKING_SET_QUOTA, EAGAIN },
#endif /* ERROR_WORKING_SET_QUOTA && EAGAIN */
#if defined(ERROR_WRITE_PROTECT) && defined(EROFS)
	{ ERROR_WRITE_PROTECT, EROFS },
#endif /* ERROR_WRITE_PROTECT && EROFS */
#endif /* NT2ERRNO_SRC_CYGWIN */


	/* NT error codes that only appear in MSVC's translation table. */
#ifdef NT2ERRNO_SRC_MSVC
#if defined(ERROR_ARENA_TRASHED) && defined(ENOMEM)
	{ ERROR_ARENA_TRASHED, ENOMEM },
#endif /* ERROR_ARENA_TRASHED && ENOMEM */
#if defined(ERROR_INVALID_BLOCK) && defined(ENOMEM)
	{ ERROR_INVALID_BLOCK, ENOMEM },
#endif /* ERROR_INVALID_BLOCK && ENOMEM */
#if defined(ERROR_BAD_ENVIRONMENT) && defined(E2BIG)
	{ ERROR_BAD_ENVIRONMENT, E2BIG },
#endif /* ERROR_BAD_ENVIRONMENT && E2BIG */
#if defined(ERROR_BAD_FORMAT) && defined(ENOEXEC)
	{ ERROR_BAD_FORMAT, ENOEXEC },
#endif /* ERROR_BAD_FORMAT && ENOEXEC */
#if defined(ERROR_INVALID_ACCESS) && defined(EINVAL)
	{ ERROR_INVALID_ACCESS, EINVAL },
#endif /* ERROR_INVALID_ACCESS && EINVAL */
#if defined(ERROR_CURRENT_DIRECTORY) && defined(EACCES)
	{ ERROR_CURRENT_DIRECTORY, EACCES },
#endif /* ERROR_CURRENT_DIRECTORY && EACCES */
#if defined(ERROR_NETWORK_ACCESS_DENIED) && defined(EACCES)
	{ ERROR_NETWORK_ACCESS_DENIED, EACCES },
#endif /* ERROR_NETWORK_ACCESS_DENIED && EACCES */
#if defined(ERROR_FAIL_I24) && defined(EACCES)
	{ ERROR_FAIL_I24, EACCES },
#endif /* ERROR_FAIL_I24 && EACCES */
#if defined(ERROR_DRIVE_LOCKED) && defined(EACCES)
	{ ERROR_DRIVE_LOCKED, EACCES },
#endif /* ERROR_DRIVE_LOCKED && EACCES */
#if defined(ERROR_INVALID_TARGET_HANDLE) && defined(EBADF)
	{ ERROR_INVALID_TARGET_HANDLE, EBADF },
#endif /* ERROR_INVALID_TARGET_HANDLE && EBADF */
#if defined(ERROR_DIRECT_ACCESS_HANDLE) && defined(EBADF)
	{ ERROR_DIRECT_ACCESS_HANDLE, EBADF },
#endif /* ERROR_DIRECT_ACCESS_HANDLE && EBADF */
#if defined(ERROR_SEEK_ON_DEVICE) && defined(EACCES)
	{ ERROR_SEEK_ON_DEVICE, EACCES },
#endif /* ERROR_SEEK_ON_DEVICE && EACCES */
#if defined(ERROR_NOT_LOCKED) && defined(EACCES)
	{ ERROR_NOT_LOCKED, EACCES },
#endif /* ERROR_NOT_LOCKED && EACCES */
#if defined(ERROR_LOCK_FAILED) && defined(EACCES)
	{ ERROR_LOCK_FAILED, EACCES },
#endif /* ERROR_LOCK_FAILED && EACCES */
#if defined(ERROR_NESTING_NOT_ALLOWED) && defined(EAGAIN)
	{ ERROR_NESTING_NOT_ALLOWED, EAGAIN },
#endif /* ERROR_NESTING_NOT_ALLOWED && EAGAIN */
#endif /* NT2ERRNO_SRC_MSVC */

	/* NT error codes for which CYGWIN and MSVC have the same answer */
#if defined(NT2ERRNO_SRC_MSVC) || defined(NT2ERRNO_SRC_CYGWIN)
#if defined(ERROR_FILE_NOT_FOUND) && defined(ENOENT)
	{ ERROR_FILE_NOT_FOUND, ENOENT },
#endif /* ERROR_FILE_NOT_FOUND && ENOENT */
#if defined(ERROR_PATH_NOT_FOUND) && defined(ENOENT)
	{ ERROR_PATH_NOT_FOUND, ENOENT },
#endif /* ERROR_PATH_NOT_FOUND && ENOENT */
#if defined(ERROR_TOO_MANY_OPEN_FILES) && defined(EMFILE)
	{ ERROR_TOO_MANY_OPEN_FILES, EMFILE },
#endif /* ERROR_TOO_MANY_OPEN_FILES && EMFILE */
#if defined(ERROR_ACCESS_DENIED) && defined(EACCES)
	{ ERROR_ACCESS_DENIED, EACCES },
#endif /* ERROR_ACCESS_DENIED && EACCES */
#if defined(ERROR_NOT_ENOUGH_MEMORY) && defined(ENOMEM)
	{ ERROR_NOT_ENOUGH_MEMORY, ENOMEM },
#endif /* ERROR_NOT_ENOUGH_MEMORY && ENOMEM */
#if defined(ERROR_INVALID_DATA) && defined(EINVAL)
	{ ERROR_INVALID_DATA, EINVAL },
#endif /* ERROR_INVALID_DATA && EINVAL */
#if defined(ERROR_NOT_SAME_DEVICE) && defined(EXDEV)
	{ ERROR_NOT_SAME_DEVICE, EXDEV },
#endif /* ERROR_NOT_SAME_DEVICE && EXDEV */
#if defined(ERROR_BAD_NETPATH) && defined(ENOENT)
	{ ERROR_BAD_NETPATH, ENOENT },
#endif /* ERROR_BAD_NETPATH && ENOENT */
#if defined(ERROR_BAD_NET_NAME) && defined(ENOENT)
	{ ERROR_BAD_NET_NAME, ENOENT },
#endif /* ERROR_BAD_NET_NAME && ENOENT */
#if defined(ERROR_FILE_EXISTS) && defined(EEXIST)
	{ ERROR_FILE_EXISTS, EEXIST },
#endif /* ERROR_FILE_EXISTS && EEXIST */
#if defined(ERROR_INVALID_PARAMETER) && defined(EINVAL)
	{ ERROR_INVALID_PARAMETER, EINVAL },
#endif /* ERROR_INVALID_PARAMETER && EINVAL */
#if defined(ERROR_NO_PROC_SLOTS) && defined(EAGAIN)
	{ ERROR_NO_PROC_SLOTS, EAGAIN },
#endif /* ERROR_NO_PROC_SLOTS && EAGAIN */
#if defined(ERROR_BROKEN_PIPE) && defined(EPIPE)
	{ ERROR_BROKEN_PIPE, EPIPE },
#endif /* ERROR_BROKEN_PIPE && EPIPE */
#if defined(ERROR_DISK_FULL) && defined(ENOSPC)
	{ ERROR_DISK_FULL, ENOSPC },
#endif /* ERROR_DISK_FULL && ENOSPC */
#if defined(ERROR_WAIT_NO_CHILDREN) && defined(ECHILD)
	{ ERROR_WAIT_NO_CHILDREN, ECHILD },
#endif /* ERROR_WAIT_NO_CHILDREN && ECHILD */
#if defined(ERROR_NEGATIVE_SEEK) && defined(EINVAL)
	{ ERROR_NEGATIVE_SEEK, EINVAL },
#endif /* ERROR_NEGATIVE_SEEK && EINVAL */
#if defined(ERROR_DIR_NOT_EMPTY) && defined(ENOTEMPTY)
	{ ERROR_DIR_NOT_EMPTY, ENOTEMPTY },
#endif /* ERROR_DIR_NOT_EMPTY && ENOTEMPTY */
#if defined(ERROR_BAD_PATHNAME) && defined(ENOENT)
	{ ERROR_BAD_PATHNAME, ENOENT },
#endif /* ERROR_BAD_PATHNAME && ENOENT */
#if defined(ERROR_MAX_THRDS_REACHED) && defined(EAGAIN)
	{ ERROR_MAX_THRDS_REACHED, EAGAIN },
#endif /* ERROR_MAX_THRDS_REACHED && EAGAIN */
#if defined(ERROR_ALREADY_EXISTS) && defined(EEXIST)
	{ ERROR_ALREADY_EXISTS, EEXIST },
#endif /* ERROR_ALREADY_EXISTS && EEXIST */
#endif /* NT2ERRNO_SRC_MSVC || NT2ERRNO_SRC_CYGWIN */

	/* NT error codes for which CYGWIN and MSVC have different answers */
#if defined(NT2ERRNO_SRC_CYGWIN) || defined(NT2ERRNO_SRC_MSVC)
#ifdef ERROR_INVALID_FUNCTION
#if defined(EBADRQC) && !defined(NT2ERRNO_PREFER_MSVC)
	{ ERROR_INVALID_FUNCTION, EBADRQC },
#elif defined(EINVAL)
	{ ERROR_INVALID_FUNCTION, EINVAL },
#elif defined(EBADRQC)
	{ ERROR_INVALID_FUNCTION, EBADRQC },
#endif
#endif /* ERROR_INVALID_FUNCTION */

#ifdef ERROR_INVALID_DRIVE
#if defined(ENODEV) && !defined(NT2ERRNO_PREFER_MSVC)
	{ ERROR_INVALID_DRIVE, ENODEV },
#elif defined(ENOENT)
	{ ERROR_INVALID_DRIVE, ENOENT },
#elif defined(ENODEV)
	{ ERROR_INVALID_DRIVE, ENODEV },
#endif
#endif /* ERROR_INVALID_DRIVE */

#ifdef ERROR_NO_MORE_FILES
#if defined(ENMFILE) && !defined(NT2ERRNO_PREFER_MSVC)
	{ ERROR_NO_MORE_FILES, ENMFILE },
#elif defined(ENOENT)
	{ ERROR_NO_MORE_FILES, ENOENT },
#elif defined(ENMFILE)
	{ ERROR_NO_MORE_FILES, ENMFILE },
#endif
#endif /* ERROR_NO_MORE_FILES */

#ifdef ERROR_INVALID_DRIVE
#if defined(EBUSY) && !defined(NT2ERRNO_PREFER_MSVC)
	{ ERROR_LOCK_VIOLATION, EBUSY },
#elif defined(EACCES)
	{ ERROR_LOCK_VIOLATION, EACCES },
#elif defined(EBUSY)
	{ ERROR_LOCK_VIOLATION, EBUSY },
#endif
#endif /* ERROR_INVALID_DRIVE */

#ifdef ERROR_CANNOT_MAKE
#if defined(EPERM) && !defined(NT2ERRNO_PREFER_MSVC)
	{ ERROR_CANNOT_MAKE, EPERM },
#elif defined(EACCES)
	{ ERROR_CANNOT_MAKE, EACCES },
#elif defined(EPERM)
	{ ERROR_CANNOT_MAKE, EPERM },
#endif
#endif /* ERROR_CANNOT_MAKE */

#ifdef ERROR_INVALID_HANDLE
#if defined(EBADF) && !defined(NT2ERRNO_PREFER_MSVC)
	{ ERROR_INVALID_HANDLE, EBADF },
#elif defined(EINVAL)
	{ ERROR_INVALID_HANDLE, EINVAL },
#elif defined(EBADF)
	{ ERROR_INVALID_HANDLE, EBADF },
#endif
#endif /* ERROR_INVALID_HANDLE */

#ifdef ERROR_CHILD_NOT_COMPLETE
#if defined(EBUSY) && !defined(NT2ERRNO_PREFER_MSVC)
	{ ERROR_CHILD_NOT_COMPLETE, EBUSY },
#elif defined(ECHILD)
	{ ERROR_CHILD_NOT_COMPLETE, ECHILD },
#elif defined(EBUSY)
	{ ERROR_CHILD_NOT_COMPLETE, EBUSY },
#endif
#endif /* ERROR_CHILD_NOT_COMPLETE */

#ifdef ERROR_FILENAME_EXCED_RANGE
#if defined(ENAMETOOLONG) && !defined(NT2ERRNO_PREFER_MSVC)
	{ ERROR_FILENAME_EXCED_RANGE, ENAMETOOLONG },
#elif defined(ENOENT)
	{ ERROR_FILENAME_EXCED_RANGE, ENOENT },
#elif defined(ENAMETOOLONG)
	{ ERROR_FILENAME_EXCED_RANGE, ENAMETOOLONG },
#endif
#endif /* ERROR_FILENAME_EXCED_RANGE */

#ifdef ERROR_NOT_ENOUGH_QUOTA
#if defined(EIO) && !defined(NT2ERRNO_PREFER_MSVC)
	{ ERROR_NOT_ENOUGH_QUOTA, EIO },
#elif defined(ENOMEM)
	{ ERROR_NOT_ENOUGH_QUOTA, ENOMEM },
#elif defined(EIO)
	{ ERROR_NOT_ENOUGH_QUOTA, EIO },
#endif
#endif /* ERROR_NOT_ENOUGH_QUOTA */
#endif /* NT2ERRNO_SRC_CYGWIN || NT2ERRNO_SRC_MSVC */

	{ 0, 0 }
};
#endif /* DeeNTSystem_Translate(Errno|NtError)_USE_FALLBACK */

/* Translate a given `dwError' into the appropriate `errno' error code.
 * If the translation failed, return a fallback value.
 * Note that (if possible), the implementation of this function is handled by the
 * linked C library, using MSVC's `_dosmaperr()' (if available). Otherwise, the
 * translation is performed identical to what is known to be done by the linked
 * C library, or a combination of CYGWIN and MSVC if some other libc is hosting
 * deemon within a windows environment.
 * NOTE: This function is also used by `DeeNTSystem_ThrowErrorf()' to translate
 *       the given NT error code into an errno. */
PUBLIC ATTR_CONST WUNUSED /*errno_t*/ int DCALL
DeeNTSystem_TranslateErrno(/*DWORD*/ DeeNT_DWORD dwError) {
#ifdef DeeNTSystem_TranslateErrno_USE_ERRNO_NT2KOS
	return errno_nt2kos(dwError);
#endif /* DeeNTSystem_TranslateErrno_USE_ERRNO_NT2KOS */
#ifdef DeeNTSystem_TranslateErrno_USE_DOSMAPERR
	/*errno_t*/ int result, old_errno;
	IF_CONFIG_HAVE__doserrno(DeeNT_DWORD old_doserrno);
	if (dwError == 0)
		return 0; /* Special case: No error */
	old_errno = DeeSystem_GetErrno();
	IF_CONFIG_HAVE__doserrno(old_doserrno = _doserrno);
	_dosmaperr(dwError);
	result = DeeSystem_GetErrno();
	IF_CONFIG_HAVE__doserrno(_doserrno = old_doserrno);
	DeeSystem_SetErrno(old_errno);
	return result;
#endif /* DeeNTSystem_TranslateErrno_USE_DOSMAPERR */
#ifdef DeeNTSystem_TranslateErrno_USE_FALLBACK
	unsigned int i;
	if (dwError == 0)
		return 0; /* Special case: No error */
	for (i = 0; nt2errno[i].etval; ++i) {
		if (nt2errno[i].etval == dwError)
			return nt2errno[i].erval;
	}
	/* Fallback: Check if we recognize the error */
#ifdef ENOENT
	if (DeeNTSystem_IsFileNotFoundError(dwError))
		return ENOENT;
#endif /* ENOENT */
#ifdef EACCES
	if (DeeNTSystem_IsAccessDeniedError(dwError))
		return EACCES;
#endif /* EACCES */
#ifdef ENOMEM
	if (DeeNTSystem_IsBadAllocError(dwError))
		return ENOMEM;
#endif /* ENOMEM */
#ifdef EBUSY
	if (DeeNTSystem_IsBusy(dwError))
		return EBUSY;
#endif /* EBUSY */
#ifdef EEXIST
	if (DeeNTSystem_IsExists(dwError))
		return EEXIST;
#endif /* EEXIST */
#ifdef ENOTDIR
	if (DeeNTSystem_IsNotDir(dwError))
		return ENOTDIR;
#endif /* ENOTDIR */
#ifdef ENOTEMPTY
	if (DeeNTSystem_IsNotEmpty(dwError))
		return ENOTEMPTY;
#endif /* ENOTEMPTY */
#ifdef EBADF
	if (DeeNTSystem_IsBadF(dwError))
		return EBADF;
#endif /* EBADF */
#ifdef EXDEV
	if (DeeNTSystem_IsXDev(dwError))
		return EXDEV;
#endif /* EXDEV */
#ifdef ENOSYS
	if (DeeNTSystem_IsUnsupportedError(dwError))
		return ENOSYS;
#endif /* ENOSYS */
#ifdef EINVAL
	if (DeeNTSystem_IsInvalidArgument(dwError))
		return EINVAL;
#endif /* EINVAL */
	/* Fallback `EACCES' */
#undef HAVE_RETURN
#if defined(NT2ERRNO_SRC_CYGWIN) || defined(NT2ERRNO_SRC_MSVC)
#if defined(EACCES) && !defined(NT2ERRNO_PREFER_MSVC)
#define HAVE_RETURN 1
	return EACCES;
#elif defined(EINVAL)
#define HAVE_RETURN 1
	return EINVAL;
#elif defined(EACCES)
#define HAVE_RETURN 1
	return EACCES;
#endif /* ... */
#endif /* NT2ERRNO_SRC_CYGWIN || NT2ERRNO_SRC_MSVC */
#ifndef HAVE_RETURN
	return Dee_SYSTEM_ERROR_UNKNOWN;
#endif /* !HAVE_RETURN */
#endif /* DeeNTSystem_TranslateErrno_USE_FALLBACK */
}



/* Do the reverse of `DeeNTSystem_TranslateErrno()' */
DFUNDEF ATTR_CONST WUNUSED /*DWORD*/ DeeNT_DWORD DCALL
DeeNTSystem_TranslateNtError(/*errno_t*/ int errno_value) {
#ifdef DeeNTSystem_TranslateNtError_USE_ERRNO_KOS2NT
	return errno_kos2nt(errno_value);
#endif /* DeeNTSystem_TranslateNtError_USE_ERRNO_KOS2NT */

#ifdef DeeNTSystem_TranslateNtError_USE_FALLBACK
	unsigned int i;
	if (errno_value == 0)
		return 0; /* Special case: No error */
	for (i = 0; nt2errno[i].etval; ++i) {
		if (nt2errno[i].erval == errno_value)
			return nt2errno[i].etval;
	}
	/* Fallback... */
	return ERROR_INVALID_FUNCTION;
#endif /* DeeNTSystem_TranslateNtError_USE_FALLBACK */
}



/* Throw NT system errors, given an error code as returned by `GetLastError()'
 * When no error code is given, `GetLastError()' is called internally.
 * When `tp' is `NULL', the proper error type is automatically determined using the `DeeNTSystem_Is*' functions.
 * @return: -1: These functions always return -1 */
PUBLIC ATTR_COLD NONNULL((3)) int
(DCALL DeeNTSystem_VThrowErrorf)(DeeTypeObject *tp,
                                 DeeNT_DWORD dwError,
                                 char const *__restrict format,
                                 va_list args) {
	int result;
	/* Automatically select the proper error class. */
	if (!tp) {
		if (DeeNTSystem_IsFileNotFoundError(dwError)) {
			tp = &DeeError_FileNotFound;
		} else if (DeeNTSystem_IsExists(dwError)) {
			tp = &DeeError_FileExists;
		} else if (DeeNTSystem_IsBadF(dwError)) {
			tp = &DeeError_FileClosed;
		} else if (DeeNTSystem_IsAccessDeniedError(dwError)) {
			tp = &DeeError_FileAccessError;
		} else if (DeeNTSystem_IsBadAllocError(dwError)) {
			tp = &DeeError_NoMemory;
		} else if (DeeNTSystem_IsUnsupportedError(dwError)) {
			tp = &DeeError_UnsupportedAPI;
		} else if (DeeNTSystem_IsInvalidArgument(dwError)) {
			tp = &DeeError_ValueError;
		} else {
			char const *fs_error_name = NULL;
			/* Special handling for error types that are implemented by the `fs' module. */
			if (DeeNTSystem_IsBusy(dwError)) {
				fs_error_name = "BusyFile";
			} else if (DeeNTSystem_IsNotDir(dwError)) {
				fs_error_name = "NoDirectory";
			} else if (DeeNTSystem_IsNotEmpty(dwError)) {
				fs_error_name = "NotEmpty";
			} else if (DeeNTSystem_IsXDev(dwError)) {
				fs_error_name = "CrossDevice";
			} else if (DeeNTSystem_IsNoLink(dwError)) {
				fs_error_name = "NoLink";
			}
			if (fs_error_name) {
				DREF DeeTypeObject *fs_error_type;
				fs_error_type = (DREF DeeTypeObject *)DeeModule_GetExtern("fs", fs_error_name);
				if unlikely(!fs_error_type)
					goto err;
				result = DeeNTSystem_VThrowErrorf(fs_error_type,
				                                  dwError,
				                                  format,
				                                  args);
				Dee_Decref(fs_error_type);
				goto done;
			}
			/* Fallback: Just use a SystemError */
			tp = &DeeError_SystemError;
		}
	}
	/* Check for error types derived from `errors.SystemError' */
	if (DeeType_Check(tp) &&
	    DeeType_IsInherited(tp, &DeeError_SystemError)) {
		DREF DeeSystemErrorObject *error;
		DREF DeeStringObject *message;
		error = (DREF DeeSystemErrorObject *)DeeObject_MALLOC(DeeSystemErrorObject);
		if unlikely(!error) {
			result = -1;
			goto done;
		}
		message = (DREF DeeStringObject *)DeeString_VNewf(format, args);
		if unlikely(!message) {
			DeeObject_FREE(error);
			result = -1;
			goto done;
		}
		error->e_message    = message; /* Inherit reference */
		error->e_inner      = NULL;
		error->se_errno     = DeeNTSystem_TranslateErrno(dwError);
		error->se_lasterror = dwError;
		DeeObject_Init(error, tp);
		result = DeeError_Throw((DeeObject *)error);
		Dee_Decref(error);
	} else {
		/* Unlikely to happen: Just throw a regular, old error. */
		result = DeeError_VThrowf(tp, format, args);
	}
done:
	return result;
err:
	result = -1;
	goto done;
}

PUBLIC ATTR_COLD NONNULL((2)) int
(DCALL DeeNTSystem_VThrowLastErrorf)(DeeTypeObject *tp,
                                     char const *__restrict format,
                                     va_list args) {
	int result;
	DeeNT_DWORD dwError;
	dwError = GetLastError();
	result = DeeNTSystem_VThrowErrorf(tp,
	                                  dwError,
	                                  format,
	                                  args);
	return result;
}

PUBLIC ATTR_COLD NONNULL((3)) int
(DeeNTSystem_ThrowErrorf)(DeeTypeObject *tp,
                          DeeNT_DWORD dwError,
                          char const *__restrict format, ...) {
	int result;
	va_list args;
	va_start(args, format);
	result = DeeNTSystem_VThrowErrorf(tp, dwError, format, args);
	va_end(args);
	return result;
}

PUBLIC ATTR_COLD NONNULL((2)) int
(DeeNTSystem_ThrowLastErrorf)(DeeTypeObject *tp,
                              char const *__restrict format, ...) {
	int result;
	va_list args;
	va_start(args, format);
	result = DeeNTSystem_VThrowLastErrorf(tp, format, args);
	va_end(args);
	return result;
}


/* Work around a problem with long path names. (Note: also handles interrupts)
 * @return: * :                   The new handle.
 * @return: NULL:                 A deemon callback failed and an error was thrown.
 * @return: INVALID_HANDLE_VALUE: The system call failed (See GetLastError()) */
PUBLIC WUNUSED NONNULL((1)) /*HANDLE*/ void *DCALL
DeeNTSystem_CreateFile(/*String*/ DeeObject *__restrict lpFileName,
                       /*DWORD*/ DeeNT_DWORD dwDesiredAccess,
                       /*DWORD*/ DeeNT_DWORD dwShareMode,
                       /*LPSECURITY_ATTRIBUTES*/ void *lpSecurityAttributes,
                       /*DWORD*/ DeeNT_DWORD dwCreationDisposition,
                       /*DWORD*/ DeeNT_DWORD dwFlagsAndAttributes,
                       /*HANDLE*/ void *hTemplateFile) {
	HANDLE hResult;
	LPWSTR lpwName;
	ASSERT_OBJECT_TYPE_EXACT(lpFileName, &DeeString_Type);
#ifdef CONFIG_WANT_WINDOWS_STD_FILES
#define eqnocase(a, b_lower) ((a) == (b_lower) || (a) == ((b_lower) + ('A' - 'a')))
	if (DeeString_SIZE(lpFileName) >= 6 &&
	    eqnocase(DeeString_STR(lpFileName)[0], 's') &&
	    eqnocase(DeeString_STR(lpFileName)[1], 't') &&
	    eqnocase(DeeString_STR(lpFileName)[2], 'd')) {
		if (DeeString_SIZE(lpFileName) == 6 &&
		    eqnocase(DeeString_STR(lpFileName)[3], 'i') &&
		    eqnocase(DeeString_STR(lpFileName)[4], 'n') &&
		    DeeString_STR(lpFileName)[5] == '$') {
			hResult = GetStdHandle(STD_INPUT_HANDLE);
do_copy_and_return_hResult:
			if (!DuplicateHandle(GetCurrentProcess(), hResult,
			                     GetCurrentProcess(), &hResult,
			                     dwDesiredAccess, FALSE, 0))
				return INVALID_HANDLE_VALUE;
			if unlikely(hResult == NULL)
				hResult = INVALID_HANDLE_VALUE;
			return hResult;
		}
		if (DeeString_SIZE(lpFileName) == 7 &&
		    DeeString_STR(lpFileName)[6] == '$') {
			if (eqnocase(DeeString_STR(lpFileName)[3], 'o') &&
			    eqnocase(DeeString_STR(lpFileName)[4], 'u') &&
			    eqnocase(DeeString_STR(lpFileName)[5], 't')) {
				hResult = GetStdHandle(STD_OUTPUT_HANDLE);
				goto do_copy_and_return_hResult;
			}
			if (eqnocase(DeeString_STR(lpFileName)[3], 'e') &&
			    eqnocase(DeeString_STR(lpFileName)[4], 'r') &&
			    eqnocase(DeeString_STR(lpFileName)[5], 'r')) {
				hResult = GetStdHandle(STD_ERROR_HANDLE);
				goto do_copy_and_return_hResult;
			}
		}
	}
#undef eqnocase
#endif /* CONFIG_WANT_WINDOWS_STD_FILES */
	lpwName = (LPWSTR)DeeString_AsWide(lpFileName);
	if unlikely(!lpwName)
		goto err;
again_createfile:
	DBG_ALIGNMENT_DISABLE();
	hResult = CreateFileW(lpwName,
	                      (DWORD)dwDesiredAccess,
	                      (DWORD)dwShareMode,
	                      (LPSECURITY_ATTRIBUTES)lpSecurityAttributes,
	                      (DWORD)dwCreationDisposition,
	                      (DWORD)dwFlagsAndAttributes,
	                      (HANDLE)hTemplateFile);
	if (hResult == INVALID_HANDLE_VALUE) {
		DWORD dwError = GetLastError();
		if (DeeNTSystem_IsUncError(dwError)) {
			/* Fix the filename and try again. */
			DBG_ALIGNMENT_ENABLE();
			lpFileName = DeeNTSystem_FixUncPath(lpFileName);
			if unlikely(!lpFileName)
				goto err;
			lpwName = (LPWSTR)DeeString_AsWide(lpFileName);
			if unlikely(!lpwName) {
				Dee_Decref(lpFileName);
				goto err;
			}
			DBG_ALIGNMENT_DISABLE();
			hResult = CreateFileW(lpwName,
			                     (DWORD)dwDesiredAccess,
			                     (DWORD)dwShareMode,
			                     (LPSECURITY_ATTRIBUTES)lpSecurityAttributes,
			                     (DWORD)dwCreationDisposition,
			                     (DWORD)dwFlagsAndAttributes,
			                     (HANDLE)hTemplateFile);
			DBG_ALIGNMENT_ENABLE();
			Dee_Decref(lpFileName);
		} else if (DeeNTSystem_IsIntr(dwError)) {
			DBG_ALIGNMENT_ENABLE();
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again_createfile;
		}
	}
	DBG_ALIGNMENT_ENABLE();
	if unlikely(hResult == NULL)
		hResult = INVALID_HANDLE_VALUE; /* Shouldn't happen... */
	return hResult;
err:
	return NULL;
}

/* Determine the filename from a handle, as returned by `DeeNTSystem_CreateFile()' */
PUBLIC WUNUSED /*String*/ DREF DeeObject *DCALL
DeeNTSystem_GetFilenameOfHandle(/*HANDLE*/ void *hFile) {
	int error;
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	error = DeeNTSystem_PrintFilenameOfHandle(&printer, hFile);
	if unlikely(error != 0) {
		if (error > 0) {
			DeeNTSystem_ThrowLastErrorf(NULL,
			                            "Failed to print path of HANDLE %p",
			                            hFile);
		}
		goto err;
	}
	return unicode_printer_pack(&printer);
err:
	unicode_printer_fini(&printer);
	return NULL;
}

/* Same as `DeeNTSystem_GetFilenameOfHandle()', but return `ITER_DONE' rather than
 * throwing a SystemError when `DeeNTSystem_PrintFilenameOfHandle()' returns `1' */
PUBLIC WUNUSED DREF /*String*/ DeeObject *DCALL
DeeNTSystem_TryGetFilenameOfHandle(/*HANDLE*/ void *hFile) {
	int error;
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	error = DeeNTSystem_PrintFilenameOfHandle(&printer, hFile);
	if unlikely(error != 0) {
		if (error > 0) {
			unicode_printer_fini(&printer);
			return ITER_DONE;
		}
		goto err;
	}
	return unicode_printer_pack(&printer);
err:
	unicode_printer_fini(&printer);
	return NULL;
}



typedef DWORD (WINAPI *LPGETFINALPATHNAMEBYHANDLEW)(HANDLE hFile, LPWSTR lpszFilePath,
                                                    DWORD cchFilePath, DWORD dwFlags);
PRIVATE LPGETFINALPATHNAMEBYHANDLEW pdyn_GetFinalPathNameByHandleW = NULL;

#ifndef DEFINED_GET_KERNEL32_HANDLE
#define DEFINED_GET_KERNEL32_HANDLE 1
PRIVATE WCHAR const wKernel32[]    = { 'K', 'E', 'R', 'N', 'E', 'L', '3', '2', 0 };
PRIVATE WCHAR const wKernel32Dll[] = { 'K', 'e', 'r', 'n', 'e', 'l', '3', '2', '.', 'd', 'l', 'l', 0 };
PRIVATE HMODULE DCALL GetKernel32Handle(void) {
	HMODULE hKernel32;
	hKernel32 = GetModuleHandleW(wKernel32);
	if (!hKernel32)
		hKernel32 = LoadLibraryW(wKernel32Dll);
	return hKernel32;
}
#endif /* !DEFINED_GET_KERNEL32_HANDLE */

/* Wrapper for the `GetFinalPathNameByHandle()' system call.
 * @return: 2:  Unsupported.
 * @return: 1:  The system call failed (See GetLastError()).
 * @return: 0:  Success.
 * @return: -1: A deemon callback failed and an error was thrown. */
PUBLIC WUNUSED int DCALL
DeeNTSystem_PrintFinalPathNameByHandle(struct unicode_printer *__restrict printer,
                                       /*HANDLE*/ void *hFile,
                                       /*DWORD*/ DeeNT_DWORD dwFlags) {
	LPWSTR lpNewBuffer, lpBuffer;
	DWORD dwNewBufSize, dwBufSize;
	if (!pdyn_GetFinalPathNameByHandleW) {
		/* Try to load `GetFinalPathNameByHandleW()' */
		HMODULE hKernel32 = GetKernel32Handle();
		if (!hKernel32)
			ATOMIC_WRITE(*(void **)&pdyn_GetFinalPathNameByHandleW, (void *)(uintptr_t)-1);
		else {
			LPGETFINALPATHNAMEBYHANDLEW func;
			func = (LPGETFINALPATHNAMEBYHANDLEW)GetProcAddress(hKernel32, "GetFinalPathNameByHandleW");
			if (!func)
				*(void **)&func = (void *)(uintptr_t)-1;
			ATOMIC_WRITE(pdyn_GetFinalPathNameByHandleW, func);
		}
	}
	if (*(void **)&pdyn_GetFinalPathNameByHandleW == (void *)(uintptr_t)-1)
		return 2;
	/* Make use of `GetFinalPathNameByHandleW()' */
	dwBufSize = PATH_MAX;
	lpBuffer = unicode_printer_alloc_wchar(printer, dwBufSize);
	if unlikely(!lpBuffer)
		goto err;
	for (;;) {
		dwNewBufSize = (*pdyn_GetFinalPathNameByHandleW)((HANDLE)hFile,
		                                                 lpBuffer,
		                                                 dwBufSize,
		                                                 dwFlags);
		if unlikely(!dwNewBufSize) {
			DWORD dwError;
			dwError = GetLastError();
			if (DeeNTSystem_IsIntr(dwError)) {
				if (DeeThread_CheckInterrupt())
					goto err_lpBuffer;
				continue;
			}
			if (DeeNTSystem_IsBufferTooSmall(dwError)) {
				dwNewBufSize = dwBufSize * 2;
				goto do_resize_buffer;
			}
			unicode_printer_free_wchar(printer, lpBuffer);
			SetLastError(dwError);
			return 1;
		}
		if (dwNewBufSize <= dwBufSize)
			break;
		--dwNewBufSize; /* This would include the trailing NUL-character */
do_resize_buffer:
		lpNewBuffer = unicode_printer_resize_wchar(printer, lpBuffer, dwNewBufSize);
		if unlikely(!lpNewBuffer) {
err_lpBuffer:
			unicode_printer_free_wchar(printer, lpBuffer);
			goto err;
		}
		dwBufSize = dwNewBufSize;
	}
	if (unicode_printer_confirm_wchar(printer, lpBuffer, dwNewBufSize) < 0)
		goto err;
	return 0;
err:
	return -1;
}

/* @return: 1:  The system call failed (See GetLastError()).
 * @return: 0:  Success.
 * @return: -1: A deemon callback failed and an error was thrown. */
PUBLIC WUNUSED int DCALL
DeeNTSystem_PrintFilenameOfHandle(struct unicode_printer *__restrict printer,
                                  /*HANDLE*/ void *hFile) {
	int error;
	size_t length;
	length = UNICODE_PRINTER_LENGTH(printer);
	error = DeeNTSystem_PrintFinalPathNameByHandle(printer, hFile, 0);
	if (error == 0) {
		size_t newLength;
		/* Try to get rid of the \\?\ prefix */
		newLength = UNICODE_PRINTER_LENGTH(printer);
		if (newLength >= length + 6 &&
		    UNICODE_PRINTER_GETCHAR(printer, length + 0) == '\\' &&
		    UNICODE_PRINTER_GETCHAR(printer, length + 1) == '\\' &&
		    UNICODE_PRINTER_GETCHAR(printer, length + 2) == '?' &&
		    UNICODE_PRINTER_GETCHAR(printer, length + 3) == '\\') {
			if (DeeUni_IsAlpha(UNICODE_PRINTER_GETCHAR(printer, length + 4))) {
				size_t drive_end = length + 5;
				for (;;) {
					uint32_t ch;
					ch = UNICODE_PRINTER_GETCHAR(printer, drive_end);
					if (ch == ':')
						break;
					if (!DeeUni_IsAlpha(ch))
						goto not_a_drive_prefix;
					++drive_end;
				}
				/* This is a r"\\?\<DRIVE_LETTER(S)>:"-like prefix */
				unicode_printer_memmove(printer, length, length + 4,
				                        (newLength - length) - 4);
				unicode_printer_truncate(printer, newLength - 4);
				return 0;
			}
not_a_drive_prefix:
			/* Check for r"\\?\UNC\<SERVER>\<SHARE>"-like prefixes.
			 * These then have to be converted into r"\\<SERVER>\<SHARE>",
			 * so we can simply delete the r"?\UNC\" portion. */
			if (UNICODE_PRINTER_GETCHAR(printer, length + 4) == 'U' &&
			    UNICODE_PRINTER_GETCHAR(printer, length + 5) == 'N' &&
			    UNICODE_PRINTER_GETCHAR(printer, length + 6) == 'C' &&
			    UNICODE_PRINTER_GETCHAR(printer, length + 7) == '\\') {
				unicode_printer_memmove(printer, length + 2, length + 8,
				                        (newLength - length) - 6);
				unicode_printer_truncate(printer, newLength - 6);
				return 0;
			}
		}
		return 0;
	}
	if (error != 2)
		return error; /* Error (-1) or System error (1) */
	/* GetFinalPathNameByHandle() isn't supported (try to emulate it) */
	/* TODO: GetMappedFileName(MapViewOfFile(CreateFileMapping(fd))); */

	(void)printer;
	(void)hFile;
	DERROR_NOTIMPLEMENTED();
/*err:*/
	return -1;
}


typedef DWORD (WINAPI *LPGETMAPPEDFILENAMEW)(HANDLE hProcess, LPVOID lpv,
                                             LPWSTR lpFilename, DWORD nSize);
PRIVATE LPGETMAPPEDFILENAMEW pdyn_GetMappedFileNameW = NULL;
PRIVATE WCHAR const wPsapi[]    = { 'P', 'S', 'A', 'P', 'I', 0 };
PRIVATE WCHAR const wPsapiDll[] = { 'P', 's', 'a', 'p', 'i', '.', 'd', 'l', 'l', 0 };
PRIVATE char const name_GetMappedFileNameW[] = "GetMappedFileNameW";

/* Wrapper for the `GetMappedFileName()' system call.
 * @return: 2:  Unsupported.
 * @return: 1:  The system call failed (See GetLastError()).
 * @return: 0:  Success.
 * @return: -1: A deemon callback failed and an error was thrown. */
PUBLIC WUNUSED int DCALL
DeeNTSystem_PrintMappedFileName(struct Dee_unicode_printer *__restrict printer,
                                /*HANDLE*/ void *hProcess,
                                /*LPVOID*/ void *lpv) {
	LPWSTR lpNewBuffer, lpBuffer;
	DWORD dwNewBufSize, dwBufSize;
	if (pdyn_GetMappedFileNameW == NULL) {
		LPGETMAPPEDFILENAMEW lpGetMappedFileNameW = NULL;
		lpGetMappedFileNameW = (LPGETMAPPEDFILENAMEW)GetProcAddress(GetModuleHandleW(wPsapi),
		                                                            name_GetMappedFileNameW);
		if (!lpGetMappedFileNameW) {
			lpGetMappedFileNameW = (LPGETMAPPEDFILENAMEW)GetProcAddress(GetModuleHandleW(wKernel32),
			                                                            name_GetMappedFileNameW);
		}
		if (!lpGetMappedFileNameW) {
			HMODULE hModule = LoadLibraryW(wPsapiDll);
			if (hModule) {
				lpGetMappedFileNameW = (LPGETMAPPEDFILENAMEW)GetProcAddress(hModule, name_GetMappedFileNameW);
				if (!lpGetMappedFileNameW)
					FreeLibrary(hModule);
			}
		}
		if (!lpGetMappedFileNameW) {
			HMODULE hModule = LoadLibraryW(wKernel32);
			if (hModule) {
				lpGetMappedFileNameW = (LPGETMAPPEDFILENAMEW)GetProcAddress(hModule, name_GetMappedFileNameW);
				if (!lpGetMappedFileNameW)
					FreeLibrary(hModule);
			}
		}
		if (!lpGetMappedFileNameW)
			*(void **)&lpGetMappedFileNameW = (void *)(uintptr_t)-1;
		ATOMIC_WRITE(pdyn_GetMappedFileNameW, lpGetMappedFileNameW);
	}
	if (*(void **)&pdyn_GetMappedFileNameW == (void *)(uintptr_t)-1)
		return 2; /* Unsupported. */
	/* Make use of `GetMappedFileNameW()' */
	dwBufSize = PATH_MAX;
	lpBuffer = unicode_printer_alloc_wchar(printer, dwBufSize);
	if unlikely(!lpBuffer)
		goto err;
	for (;;) {
		dwNewBufSize = (*pdyn_GetMappedFileNameW)((HANDLE)hProcess,
		                                          (LPVOID)lpv,
		                                          lpBuffer,
		                                          dwBufSize);
		if unlikely(!dwNewBufSize) {
			DWORD dwError;
			dwError = GetLastError();
			if (DeeNTSystem_IsIntr(dwError)) {
				if (DeeThread_CheckInterrupt())
					goto err_lpBuffer;
				continue;
			}
			if (DeeNTSystem_IsBufferTooSmall(dwError)) {
				dwNewBufSize = dwBufSize * 2;
				goto do_resize_buffer;
			}
			unicode_printer_free_wchar(printer, lpBuffer);
			SetLastError(dwError);
			return 1;
		}
		if (dwNewBufSize <= dwBufSize)
			break;
do_resize_buffer:
		lpNewBuffer = unicode_printer_resize_wchar(printer, lpBuffer, dwNewBufSize);
		if unlikely(!lpNewBuffer) {
err_lpBuffer:
			unicode_printer_free_wchar(printer, lpBuffer);
			goto err;
		}
		dwBufSize = dwNewBufSize;
	}
	if (unicode_printer_confirm_wchar(printer, lpBuffer, dwNewBufSize) < 0)
		goto err;
	return 0;
err:
	return -1;
}





/* Wrapper for the `FormatMessageW()' system call.
 * @return: * :        The formatted message.
 * @return: NULL:      A deemon callback failed and an error was thrown.
 * @return: ITER_DONE: The system call failed (See GetLastError()). */
DFUNDEF WUNUSED DREF /*String*/ DeeObject *DCALL
DeeNTSystem_FormatMessage(DeeNT_DWORD dwFlags, void const *lpSource,
                          DeeNT_DWORD dwMessageId, DeeNT_DWORD dwLanguageId,
                          /* va_list * */ void *Arguments) {
	int error;
	DWORD dwLastError;
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	error = DeeNTSystem_PrintFormatMessage(&printer, dwFlags, lpSource,
	                                       dwMessageId, dwLanguageId, Arguments);
	if (error == 0)
		return unicode_printer_pack(&printer);
	/* Preserve LastError during `unicode_printer_fini()' */
	dwLastError = GetLastError();
	unicode_printer_fini(&printer);
	SetLastError(dwLastError);
	if (error < 0)
		goto err;
	return ITER_DONE;
err:
	return NULL;
}

/* @return: 1:  The system call failed (See GetLastError())
 * @return: 0:  Successfully printed the message.
 * @return: -1: A deemon callback failed and an error was thrown. */
DFUNDEF WUNUSED int DCALL
DeeNTSystem_PrintFormatMessage(struct unicode_printer *__restrict printer,
                               DeeNT_DWORD dwFlags, void const *lpSource,
                               DeeNT_DWORD dwMessageId, DeeNT_DWORD dwLanguageId,
                               /* va_list * */ void *Arguments) {
	LPWSTR buffer, newBuffer;
	DWORD dwNewBufsize, dwBufSize = 128;
	buffer = unicode_printer_alloc_wchar(printer, dwBufSize);
	if unlikely(!buffer)
		goto err;
again:
	DBG_ALIGNMENT_DISABLE();
	dwNewBufsize = FormatMessageW(dwFlags,
	                              lpSource,
	                              dwMessageId,
	                              dwLanguageId,
	                              buffer,
	                              dwBufSize,
	                              (va_list *)Arguments);
	DBG_ALIGNMENT_ENABLE();
	if unlikely(!dwNewBufsize) {
		DWORD dwError;
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsBadAllocError(dwError)) {
			if (Dee_CollectMemory(1))
				goto again;
		} else {
			/* MSDN says that this string cannot exceed 32*1024
			 * >> So if it does, treat it as a failure of translating the message... */
			if (dwBufSize > 32 * 1024) {
				unicode_printer_free_wchar(printer, buffer);
				return 1;
			}
			dwBufSize *= 2;
			newBuffer = unicode_printer_resize_wchar(printer, buffer, dwBufSize);
			if unlikely(!newBuffer)
				goto err_release;
			buffer = newBuffer;
			goto again;
		}
		goto err_release;
	}
	if (dwNewBufsize > dwBufSize) {
		LPWSTR new_buffer;
		/* Increase the buffer and try again. */
		new_buffer = unicode_printer_resize_wchar(printer, buffer, dwNewBufsize);
		if unlikely(!new_buffer)
			goto err_release;
		dwBufSize = dwNewBufsize;
		goto again;
	}
	/* Trim all trailing space characters */
	while (dwNewBufsize) {
		if (!DeeUni_IsSpace(buffer[dwNewBufsize - 1]))
			break;
		--dwNewBufsize;
	}
	if unlikely(unicode_printer_confirm_wchar(printer, buffer, dwNewBufsize) < 0)
		goto err;
	return 0;
err_release:
	unicode_printer_free_wchar(printer, buffer);
err:
	return -1;
}

/* Convenience wrapper around `DeeNTSystem_FormatMessage()' for getting error message.
 * When no error message exists, return an empty string.
 * @return: * :   The error message. (or an empty string)
 * @return: NULL: A deemon callback failed and an error was thrown. */
PUBLIC WUNUSED DREF /*String*/ DeeObject *DCALL
DeeNTSystem_FormatErrorMessage(DeeNT_DWORD dwError) {
	DREF /*String*/ DeeObject *result;
	result = DeeNTSystem_FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwError,
	                                   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
	                                   NULL);
	if (result == ITER_DONE) {
		result = Dee_EmptyString;
		Dee_Incref(Dee_EmptyString);
	}
	return result;
}


DECL_END

#endif /* CONFIG_HOST_WINDOWS */

#endif /* !GUARD_DEEMON_SYSTEM_SYSTEM_NT_C_INL */
