/* Copyright (c) 2018 Griefer@Work                                            *
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
 *    in a product, an acknowledgement in the product documentation would be  *
 *    appreciated but is not required.                                        *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEX_FS_WINDOWS_C_INL
#define GUARD_DEX_FS_WINDOWS_C_INL 1
#define _KOS_SOURCE 1

#include <deemon/api.h>
#include <Windows.h>

#include "libfs.h"
#include "../time/libtime.h"

#include <deemon/none.h>
#include <deemon/exec.h>
#include <deemon/bool.h>
#include <deemon/file.h>
#include <deemon/string.h>
#include <deemon/error.h>
#include <deemon/seq.h>
#include <deemon/arg.h>
#include <deemon/tuple.h>
#ifndef CONFIG_NO_THREADS
#include <deemon/util/rwlock.h>
#endif
#include <hybrid/unaligned.h>

#include <string.h>
#include <wchar.h>

#ifndef __USE_KOS
#define strend(str) ((str)+strlen(str))
#endif

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


DECL_BEGIN

INTDEF DREF DeeObject *DCALL nt_GetEnvironmentVariableA(char const *__restrict name);
INTDEF DREF DeeObject *DCALL nt_GetTempPath(void);

/* Work around a problem with long path names.
 * @return:  0: Successfully changed working directories.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (See GetLastError()) */
INTDEF int DCALL nt_SetCurrentDirectory(DeeObject *__restrict lpPathName);

/* Work around a problem with long path names.
 * @return:  0: Successfully retrieved attributes.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (See GetLastError()) */
INTDEF int DCALL
nt_GetFileAttributesEx(DeeObject *__restrict lpFileName,
                       GET_FILEEX_INFO_LEVELS fInfoLevelId,
                       LPVOID lpFileInformation);

/* Work around a problem with long path names.
 * @return:  0: Successfully retrieved attributes.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (See GetLastError()) */
INTDEF int DCALL
nt_GetFileAttributes(DeeObject *__restrict lpFileName,
                     DWORD *__restrict presult);

/* Work around a problem with long path names.
 * @return:  0: Successfully set attributes.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (See GetLastError()) */
INTDEF int DCALL
nt_SetFileAttributes(DeeObject *__restrict lpFileName,
                     DWORD dwFileAttributes);

/* Work around a problem with long path names.
 * @return:  0: Successfully created the new directory.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (See GetLastError()) */
INTDEF int DCALL
nt_CreateDirectory(DeeObject *__restrict lpPathName,
                   LPSECURITY_ATTRIBUTES lpSecurityAttributes);

/* Work around a problem with long path names.
 * @return:  0: Successfully removed the given directory.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (See GetLastError()) */
INTDEF int DCALL
nt_RemoveDirectory(DeeObject *__restrict lpPathName);

/* Work around a problem with long path names.
 * @return:  0: Successfully removed the given file.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (See GetLastError()) */
INTDEF int DCALL
nt_DeleteFile(DeeObject *__restrict lpFileName);

/* Work around a problem with long path names.
 * @return:  0: Successfully moved the given file.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (See GetLastError()) */
INTDEF int DCALL
nt_MoveFile(DeeObject *__restrict lpExistingFileName,
            DeeObject *__restrict lpNewFileName);

/* Work around a problem with long path names.
 * @return:  0: Successfully created the hardlink.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (See GetLastError()) */
INTDEF int DCALL
nt_CreateHardLink(DeeObject *__restrict lpFileName,
                  DeeObject *__restrict lpExistingFileName,
                  LPSECURITY_ATTRIBUTES lpSecurityAttributes);

/* Work around a problem with long path names.
 * @return:  0: Successfully created the symlink.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (See GetLastError()) */
INTDEF int APIENTRY
nt_CreateSymbolicLink(DeeObject *__restrict lpSymlinkFileName,
                      DeeObject *__restrict lpTargetFileName,
                      DWORD dwFlags);




LOCAL bool DCALL nt_IsBusy(DWORD dwError) {
 switch (dwError) {
#ifdef ERROR_CURRENT_DIRECTORY
 case ERROR_CURRENT_DIRECTORY:
#endif
#ifdef ERROR_BUSY
 case ERROR_BUSY:
#endif
#ifdef ERROR_NETWORK_BUSY
 case ERROR_NETWORK_BUSY:
#endif
#ifdef ERROR_DEVICE_SUPPORT_IN_PROGRESS
 case ERROR_DEVICE_SUPPORT_IN_PROGRESS:
#endif
#ifdef DXGI_ERROR_MODE_CHANGE_IN_PROGRESS
 case DXGI_ERROR_MODE_CHANGE_IN_PROGRESS:
#endif
#ifdef ERROR_CLUSTER_BACKUP_IN_PROGRESS
 case ERROR_CLUSTER_BACKUP_IN_PROGRESS:
#endif
#ifdef ERROR_CLUSTER_DATABASE_TRANSACTION_IN_PROGRESS
 case ERROR_CLUSTER_DATABASE_TRANSACTION_IN_PROGRESS:
#endif
#ifdef ERROR_CLUSTER_JOIN_IN_PROGRESS
 case ERROR_CLUSTER_JOIN_IN_PROGRESS:
#endif
#ifdef ERROR_CLUSTER_NODE_DRAIN_IN_PROGRESS
 case ERROR_CLUSTER_NODE_DRAIN_IN_PROGRESS:
#endif
#ifdef ERROR_DS_DOMAIN_RENAME_IN_PROGRESS
 case ERROR_DS_DOMAIN_RENAME_IN_PROGRESS:
#endif
#ifdef ERROR_DS_PDC_OPERATION_IN_PROGRESS
 case ERROR_DS_PDC_OPERATION_IN_PROGRESS:
#endif
#ifdef ERROR_FTP_TRANSFER_IN_PROGRESS
 case ERROR_FTP_TRANSFER_IN_PROGRESS:
#endif
#ifdef ERROR_GRAPHICS_OPM_SESSION_TYPE_CHANGE_IN_PROGRESS
 case ERROR_GRAPHICS_OPM_SESSION_TYPE_CHANGE_IN_PROGRESS:
#endif
#ifdef ERROR_GRAPHICS_SESSION_TYPE_CHANGE_IN_PROGRESS
 case ERROR_GRAPHICS_SESSION_TYPE_CHANGE_IN_PROGRESS:
#endif
#ifdef ERROR_JOURNAL_DELETE_IN_PROGRESS
 case ERROR_JOURNAL_DELETE_IN_PROGRESS:
#endif
#ifdef ERROR_LOG_ARCHIVE_IN_PROGRESS
 case ERROR_LOG_ARCHIVE_IN_PROGRESS:
#endif
#ifdef ERROR_LOG_FULL_HANDLER_IN_PROGRESS
 case ERROR_LOG_FULL_HANDLER_IN_PROGRESS:
#endif
#ifdef ERROR_NDIS_RESET_IN_PROGRESS
 case ERROR_NDIS_RESET_IN_PROGRESS:
#endif
#ifdef ERROR_OPERATION_IN_PROGRESS
 case ERROR_OPERATION_IN_PROGRESS:
#endif
#ifdef ERROR_OPLOCK_BREAK_IN_PROGRESS
 case ERROR_OPLOCK_BREAK_IN_PROGRESS:
#endif
#ifdef ERROR_RUNLEVEL_SWITCH_IN_PROGRESS
 case ERROR_RUNLEVEL_SWITCH_IN_PROGRESS:
#endif
#ifdef ERROR_SERVER_SHUTDOWN_IN_PROGRESS
 case ERROR_SERVER_SHUTDOWN_IN_PROGRESS:
#endif
#ifdef ERROR_SHUTDOWN_IN_PROGRESS
 case ERROR_SHUTDOWN_IN_PROGRESS:
#endif
#ifdef ERROR_TIERING_VOLUME_DISMOUNT_IN_PROGRESS
 case ERROR_TIERING_VOLUME_DISMOUNT_IN_PROGRESS:
#endif
#ifdef ERROR_TRANSACTION_FREEZE_IN_PROGRESS
 case ERROR_TRANSACTION_FREEZE_IN_PROGRESS:
#endif
#ifdef ERROR_VOLMGR_TRANSACTION_IN_PROGRESS
 case ERROR_VOLMGR_TRANSACTION_IN_PROGRESS:
#endif
  return true;
 default: break;
 }
 return false;
}
LOCAL bool DCALL nt_IsExists(DWORD dwError) {
 switch (dwError) {
#ifdef ERROR_FILE_EXISTS
 case ERROR_FILE_EXISTS:
#endif
#ifdef ERROR_ALREADY_EXISTS
 case ERROR_ALREADY_EXISTS:
#endif
#ifdef ERROR_ALIAS_EXISTS
 case ERROR_ALIAS_EXISTS:
#endif
#ifdef ERROR_OBJECT_NAME_EXISTS
 case ERROR_OBJECT_NAME_EXISTS:
#endif
  return true;
 default: break;
 }
 return false;
}
LOCAL bool DCALL nt_IsNotDir(DWORD dwError) {
 switch (dwError) {
#ifdef ERROR_DIRECTORY
 case ERROR_DIRECTORY:
#endif
#ifdef ERROR_PATH_NOT_FOUND
 case ERROR_PATH_NOT_FOUND:
#endif
#ifdef ERROR_INVALID_DRIVE
 case ERROR_INVALID_DRIVE:
#endif
  return true;
 default: break;
 }
 return false;
}
LOCAL bool DCALL nt_IsNotEmpty(DWORD dwError) {
 switch (dwError) {
#ifdef ERROR_DIR_NOT_EMPTY
 case ERROR_DIR_NOT_EMPTY:
#endif
#ifdef ERROR_NOT_EMPTY
 case ERROR_NOT_EMPTY:
#endif
#ifdef ERROR_TXF_DIR_NOT_EMPTY
 case ERROR_TXF_DIR_NOT_EMPTY:
#endif
#ifdef ERROR_VOLMGR_DISK_NOT_EMPTY
 case ERROR_VOLMGR_DISK_NOT_EMPTY:
#endif
  return true;
 default: break;
 }
 return false;
}
LOCAL bool DCALL nt_IsBadf(DWORD dwError) {
 switch (dwError) {
#ifdef ERROR_INVALID_HANDLE
 case ERROR_INVALID_HANDLE:
#endif
  return true;
 default: break;
 }
 return false;
}
LOCAL bool DCALL nt_IsXDev(DWORD dwError) {
 switch (dwError) {
#ifdef ERROR_NOT_SAME_DEVICE
 case ERROR_NOT_SAME_DEVICE:
#endif
  return true;
 default: break;
 }
 return false;
}
LOCAL bool DCALL nt_IsBadAlloc(DWORD dwError) {
 switch (dwError) {
#ifdef ERROR_OUTOFMEMORY
  case ERROR_OUTOFMEMORY:
#endif
#ifdef ERROR_NOT_ENOUGH_SERVER_MEMORY
  case ERROR_NOT_ENOUGH_SERVER_MEMORY:
#endif
#ifdef ERROR_IPSEC_IKE_OUT_OF_MEMORY
 case ERROR_IPSEC_IKE_OUT_OF_MEMORY:
#endif
#ifdef E_OUTOFMEMORY
 case E_OUTOFMEMORY:
#endif
#ifdef CO_E_INIT_MEMORY_ALLOCATOR
 case CO_E_INIT_MEMORY_ALLOCATOR:
#endif
#ifdef STG_E_INSUFFICIENTMEMORY
 case STG_E_INSUFFICIENTMEMORY:
#endif
#ifdef NTE_NO_MEMORY
 case NTE_NO_MEMORY:
#endif
#ifdef SEC_E_INSUFFICIENT_MEMORY
 case SEC_E_INSUFFICIENT_MEMORY:
#endif
#ifdef OSS_OUT_MEMORY
 case OSS_OUT_MEMORY:
#endif
#ifdef CRYPT_E_ASN1_MEMORY
 case CRYPT_E_ASN1_MEMORY:
#endif
#ifdef SCARD_E_NO_MEMORY
 case SCARD_E_NO_MEMORY:
#endif
#ifdef ERROR_GRAPHICS_NO_VIDEO_MEMORY
 case ERROR_GRAPHICS_NO_VIDEO_MEMORY:
#endif
#ifdef TPMAPI_E_OUT_OF_MEMORY
 case TPMAPI_E_OUT_OF_MEMORY:
#endif
#ifdef TBSIMP_E_OUT_OF_MEMORY
 case TBSIMP_E_OUT_OF_MEMORY:
#endif
#ifdef ERROR_HV_INSUFFICIENT_MEMORY
 case ERROR_HV_INSUFFICIENT_MEMORY:
#endif
#ifdef E_MBN_SMS_MEMORY_FULL
 case E_MBN_SMS_MEMORY_FULL:
#endif
#ifdef DXGI_ERROR_REMOTE_OUTOFMEMORY
 case DXGI_ERROR_REMOTE_OUTOFMEMORY:
#endif
#ifdef UCEERR_MEMORYFAILURE
 case UCEERR_MEMORYFAILURE:
#endif
#if defined(DNS_ERROR_NO_MEMORY) && DNS_ERROR_NO_MEMORY != ERROR_OUTOFMEMORY
 case DNS_ERROR_NO_MEMORY:
#endif
  return true;
 default: break;
 }
 return false;
}
LOCAL bool DCALL nt_IsUnsupported(DWORD dwError) {
 switch (dwError) {
#ifdef ERROR_NOT_SUPPORTED
  case ERROR_NOT_SUPPORTED:
#endif
#ifdef ERROR_INVALID_FUNCTION
  case ERROR_INVALID_FUNCTION:
#endif
#ifdef ERROR_ATOMIC_LOCKS_NOT_SUPPORTED
  case ERROR_ATOMIC_LOCKS_NOT_SUPPORTED:
#endif
#ifdef ERROR_EAS_NOT_SUPPORTED
  case ERROR_EAS_NOT_SUPPORTED:
#endif
#ifdef ERROR_DEVICE_FEATURE_NOT_SUPPORTED
  case ERROR_DEVICE_FEATURE_NOT_SUPPORTED:
#endif
#ifdef ERROR_FILE_LEVEL_TRIM_NOT_SUPPORTED
  case ERROR_FILE_LEVEL_TRIM_NOT_SUPPORTED:
#endif
#ifdef ERROR_RESIDENT_FILE_NOT_SUPPORTED
  case ERROR_RESIDENT_FILE_NOT_SUPPORTED:
#endif
#ifdef ERROR_DIRECTORY_NOT_SUPPORTED
  case ERROR_DIRECTORY_NOT_SUPPORTED:
#endif
#ifdef ERROR_UNSUPPORTED_COMPRESSION
  case ERROR_UNSUPPORTED_COMPRESSION:
#endif
#ifdef ERROR_CARDBUS_NOT_SUPPORTED
  case ERROR_CARDBUS_NOT_SUPPORTED:
#endif
#ifdef ERROR_NOT_SUPPORTED_ON_SBS
  case ERROR_NOT_SUPPORTED_ON_SBS:
#endif
#ifdef ERROR_SYMLINK_NOT_SUPPORTED
  case ERROR_SYMLINK_NOT_SUPPORTED:
#endif
#ifdef ERROR_INSTALL_LANGUAGE_UNSUPPORTED
  case ERROR_INSTALL_LANGUAGE_UNSUPPORTED:
#endif
#ifdef ERROR_UNSUPPORTED_TYPE
  case ERROR_UNSUPPORTED_TYPE:
#endif
#ifdef ERROR_INSTALL_PLATFORM_UNSUPPORTED
  case ERROR_INSTALL_PLATFORM_UNSUPPORTED:
#endif
#ifdef ERROR_PATCH_PACKAGE_UNSUPPORTED
  case ERROR_PATCH_PACKAGE_UNSUPPORTED:
#endif
#ifdef ERROR_PATCH_REMOVAL_UNSUPPORTED
  case ERROR_PATCH_REMOVAL_UNSUPPORTED:
#endif
#ifdef RPC_S_PROTSEQ_NOT_SUPPORTED
  case RPC_S_PROTSEQ_NOT_SUPPORTED:
#endif
#ifdef RPC_S_UNSUPPORTED_TRANS_SYN
  case RPC_S_UNSUPPORTED_TRANS_SYN:
#endif
#ifdef RPC_S_UNSUPPORTED_TYPE
  case RPC_S_UNSUPPORTED_TYPE:
#endif
#ifdef RPC_S_UNSUPPORTED_NAME_SYNTAX
  case RPC_S_UNSUPPORTED_NAME_SYNTAX:
#endif
#ifdef RPC_S_CANNOT_SUPPORT
  case RPC_S_CANNOT_SUPPORT:
#endif
#ifdef RPC_S_UNSUPPORTED_AUTHN_LEVEL
  case RPC_S_UNSUPPORTED_AUTHN_LEVEL:
#endif
#ifdef ERROR_METAFILE_NOT_SUPPORTED
  case ERROR_METAFILE_NOT_SUPPORTED:
#endif
#ifdef ERROR_TRANSFORM_NOT_SUPPORTED
  case ERROR_TRANSFORM_NOT_SUPPORTED:
#endif
#ifdef ERROR_CLIPPING_NOT_SUPPORTED
  case ERROR_CLIPPING_NOT_SUPPORTED:
#endif
#ifdef PEERDIST_ERROR_CONTENTINFO_VERSION_UNSUPPORTED
  case PEERDIST_ERROR_CONTENTINFO_VERSION_UNSUPPORTED:
#endif
#ifdef PEERDIST_ERROR_VERSION_UNSUPPORTED
  case PEERDIST_ERROR_VERSION_UNSUPPORTED:
#endif
#ifdef ERROR_NOT_SUPPORTED_IN_APPCONTAINER
  case ERROR_NOT_SUPPORTED_IN_APPCONTAINER:
#endif
#ifdef ERROR_NO_SUPPORTING_DRIVES
  case ERROR_NO_SUPPORTING_DRIVES:
#endif
#ifdef ERROR_OFFLOAD_READ_FLT_NOT_SUPPORTED
  case ERROR_OFFLOAD_READ_FLT_NOT_SUPPORTED:
#endif
#ifdef ERROR_OFFLOAD_WRITE_FLT_NOT_SUPPORTED
  case ERROR_OFFLOAD_WRITE_FLT_NOT_SUPPORTED:
#endif
#ifdef ERROR_OFFLOAD_READ_FILE_NOT_SUPPORTED
  case ERROR_OFFLOAD_READ_FILE_NOT_SUPPORTED:
#endif
#ifdef ERROR_OFFLOAD_WRITE_FILE_NOT_SUPPORTED
  case ERROR_OFFLOAD_WRITE_FILE_NOT_SUPPORTED:
#endif
#ifdef ERROR_CLUSTER_RESTYPE_NOT_SUPPORTED
  case ERROR_CLUSTER_RESTYPE_NOT_SUPPORTED:
#endif
#ifdef ERROR_CLUSTER_RESOURCE_CONTAINS_UNSUPPORTED_DIFF_AREA_FOR_SHARED_VOLUMES
  case ERROR_CLUSTER_RESOURCE_CONTAINS_UNSUPPORTED_DIFF_AREA_FOR_SHARED_VOLUMES:
#endif
#ifdef ERROR_VOLUME_NOT_SUPPORT_EFS
  case ERROR_VOLUME_NOT_SUPPORT_EFS:
#endif
#ifdef ERROR_EFS_VERSION_NOT_SUPPORT
  case ERROR_EFS_VERSION_NOT_SUPPORT:
#endif
#ifdef ERROR_CS_ENCRYPTION_UNSUPPORTED_SERVER
  case ERROR_CS_ENCRYPTION_UNSUPPORTED_SERVER:
#endif
#ifdef ERROR_IMPLICIT_TRANSACTION_NOT_SUPPORTED
  case ERROR_IMPLICIT_TRANSACTION_NOT_SUPPORTED:
#endif
#ifdef ERROR_TRANSACTIONS_UNSUPPORTED_REMOTE
  case ERROR_TRANSACTIONS_UNSUPPORTED_REMOTE:
#endif
#ifdef ERROR_TRANSACTED_MAPPING_UNSUPPORTED_REMOTE
  case ERROR_TRANSACTED_MAPPING_UNSUPPORTED_REMOTE:
#endif
#ifdef ERROR_OPERATION_NOT_SUPPORTED_IN_TRANSACTION
  case ERROR_OPERATION_NOT_SUPPORTED_IN_TRANSACTION:
#endif
#ifdef ERROR_DS_AUTH_METHOD_NOT_SUPPORTED
  case ERROR_DS_AUTH_METHOD_NOT_SUPPORTED:
#endif
#ifdef ERROR_DS_NOT_SUPPORTED
  case ERROR_DS_NOT_SUPPORTED:
#endif
#ifdef ERROR_DS_DRA_NOT_SUPPORTED
  case ERROR_DS_DRA_NOT_SUPPORTED:
#endif
#ifdef ERROR_DS_NOT_SUPPORTED_SORT_ORDER
  case ERROR_DS_NOT_SUPPORTED_SORT_ORDER:
#endif
#ifdef ERROR_NOT_SUPPORTED_ON_STANDARD_SERVER
  case ERROR_NOT_SUPPORTED_ON_STANDARD_SERVER:
#endif
#ifdef DNS_ERROR_UNSUPPORTED_ALGORITHM
  case DNS_ERROR_UNSUPPORTED_ALGORITHM:
#endif
#ifdef DNS_ERROR_KSP_DOES_NOT_SUPPORT_PROTECTION
  case DNS_ERROR_KSP_DOES_NOT_SUPPORT_PROTECTION:
#endif
#ifdef WSAENOPROTOOPT
  case WSAENOPROTOOPT:
#endif
#ifdef WSAEPROTONOSUPPORT
  case WSAEPROTONOSUPPORT:
#endif
#ifdef WSAESOCKTNOSUPPORT
  case WSAESOCKTNOSUPPORT:
#endif
#ifdef WSAEOPNOTSUPP
  case WSAEOPNOTSUPP:
#endif
#ifdef WSAEPFNOSUPPORT
  case WSAEPFNOSUPPORT:
#endif
#ifdef WSAEAFNOSUPPORT
  case WSAEAFNOSUPPORT:
#endif
#ifdef WSAVERNOTSUPPORTED
  case WSAVERNOTSUPPORTED:
#endif
#ifdef ERROR_IPSEC_IKE_UNSUPPORTED_ID
  case ERROR_IPSEC_IKE_UNSUPPORTED_ID:
#endif
#ifdef ERROR_IPSEC_IKE_PEER_DOESNT_SUPPORT_MOBIKE
  case ERROR_IPSEC_IKE_PEER_DOESNT_SUPPORT_MOBIKE:
#endif
#ifdef ERROR_EVT_FILTER_UNSUPPORTEDOP
  case ERROR_EVT_FILTER_UNSUPPORTEDOP:
#endif
#ifdef ERROR_MRM_UNSUPPORTED_DIRECTORY_TYPE
  case ERROR_MRM_UNSUPPORTED_DIRECTORY_TYPE:
#endif
#ifdef ERROR_MRM_UNSUPPORTED_PROFILE_TYPE
  case ERROR_MRM_UNSUPPORTED_PROFILE_TYPE:
#endif
#ifdef ERROR_MRM_UNSUPPORTED_FILE_TYPE_FOR_MERGE
  case ERROR_MRM_UNSUPPORTED_FILE_TYPE_FOR_MERGE:
#endif
#ifdef ERROR_MRM_UNSUPPORTED_FILE_TYPE_FOR_LOAD_UNLOAD_PRI_FILE
  case ERROR_MRM_UNSUPPORTED_FILE_TYPE_FOR_LOAD_UNLOAD_PRI_FILE:
#endif
#ifdef ERROR_MCA_UNSUPPORTED_MCCS_VERSION
  case ERROR_MCA_UNSUPPORTED_MCCS_VERSION:
#endif
#ifdef ERROR_MCA_UNSUPPORTED_COLOR_TEMPERATURE
  case ERROR_MCA_UNSUPPORTED_COLOR_TEMPERATURE:
#endif
#ifdef ERROR_HASH_NOT_SUPPORTED
  case ERROR_HASH_NOT_SUPPORTED:
#endif
#ifdef ERROR_GPIO_VERSION_NOT_SUPPORTED
  case ERROR_GPIO_VERSION_NOT_SUPPORTED:
#endif
#ifdef RO_E_UNSUPPORTED_FROM_MTA
  case RO_E_UNSUPPORTED_FROM_MTA:
#endif
#ifdef CO_E_NOT_SUPPORTED
  case CO_E_NOT_SUPPORTED:
#endif
#ifdef OLE_E_ADVISENOTSUPPORTED
  case OLE_E_ADVISENOTSUPPORTED:
#endif
#ifdef CACHE_S_FORMATETC_NOTSUPPORTED
  case CACHE_S_FORMATETC_NOTSUPPORTED:
#endif
#ifdef SCHED_E_UNSUPPORTED_ACCOUNT_OPTION
  case SCHED_E_UNSUPPORTED_ACCOUNT_OPTION:
#endif
#ifdef NTE_NOT_SUPPORTED
  case NTE_NOT_SUPPORTED:
#endif
#ifdef NTE_HMAC_NOT_SUPPORTED
  case NTE_HMAC_NOT_SUPPORTED:
#endif
#ifdef SEC_E_UNSUPPORTED_FUNCTION
  case SEC_E_UNSUPPORTED_FUNCTION:
#endif
#ifdef SEC_E_QOP_NOT_SUPPORTED
  case SEC_E_QOP_NOT_SUPPORTED:
#endif
#ifdef SEC_E_STRONG_CRYPTO_NOT_SUPPORTED
  case SEC_E_STRONG_CRYPTO_NOT_SUPPORTED:
#endif
#ifdef SEC_E_UNSUPPORTED_PREAUTH
  case SEC_E_UNSUPPORTED_PREAUTH:
#endif
#ifdef SEC_E_NO_S4U_PROT_SUPPORT
  case SEC_E_NO_S4U_PROT_SUPPORT:
#endif
#ifdef OSS_INDEFINITE_NOT_SUPPORTED
  case OSS_INDEFINITE_NOT_SUPPORTED:
#endif
#ifdef CERTSRV_E_KEY_ATTESTATION_NOT_SUPPORTED
  case CERTSRV_E_KEY_ATTESTATION_NOT_SUPPORTED:
#endif
#ifdef CERTSRV_E_UNSUPPORTED_CERT_TYPE
  case CERTSRV_E_UNSUPPORTED_CERT_TYPE:
#endif
#ifdef SPAPI_E_REMOTE_REQUEST_UNSUPPORTED
  case SPAPI_E_REMOTE_REQUEST_UNSUPPORTED:
#endif
#ifdef SCARD_E_READER_UNSUPPORTED
  case SCARD_E_READER_UNSUPPORTED:
#endif
#ifdef SCARD_E_CARD_UNSUPPORTED
  case SCARD_E_CARD_UNSUPPORTED:
#endif
#ifdef SCARD_E_UNSUPPORTED_FEATURE
  case SCARD_E_UNSUPPORTED_FEATURE:
#endif
#ifdef SCARD_W_UNSUPPORTED_CARD
  case SCARD_W_UNSUPPORTED_CARD:
#endif
  return true;
 default: break;
 }
 return false;
}




#if defined(__USE_KOS) && !defined(CONFIG_NO_CTYPE)
#define MEMCASEEQ(a,b,s) (memcasecmp(a,b,s) == 0)
#elif defined(_MSC_VER) && !defined(CONFIG_NO_CTYPE)
#define MEMCASEEQ(a,b,s) (_memicmp(a,b,s) == 0)
#else
#define MEMCASEEQ(a,b,s)  dee_memcaseeq((uint8_t *)(a),(uint8_t *)(b),s)
LOCAL bool dee_memcaseeq(uint8_t const *a, uint8_t const *b, size_t s) {
 while (s--) {
  if (DeeUni_ToLower(*a) != DeeUni_ToLower(*b))
      return false;
  ++a;
  ++b;
 }
 return true;
}
#endif

typedef struct {
    OBJECT_HEAD
    LPWCH e_strings; /* [1..1][const] Environment strings, as retrieved by `GetEnvironmentStringsW()' */
    LPWCH e_iter;    /* [1..1] Next environment string (Pointed to an empty string when done). */
} Env;

PRIVATE int DCALL
env_init(Env *__restrict self) {
again:
 DBG_ALIGNMENT_DISABLE();
 self->e_strings = GetEnvironmentStringsW();
 DBG_ALIGNMENT_ENABLE();
 if unlikely(!self->e_strings) {
  if (Dee_CollectMemory(42)) /* ??? */
      goto again;
  return -1;
 }
 self->e_iter = self->e_strings;
 return 0;
}

PRIVATE void DCALL
env_fini(Env *__restrict self) {
 DBG_ALIGNMENT_DISABLE();
 FreeEnvironmentStringsW(self->e_strings);
 DBG_ALIGNMENT_ENABLE();
}

PRIVATE int DCALL
env_bool(Env *__restrict self) {
 return self->e_iter[0] != 0;
}


PRIVATE DREF DeeObject *DCALL
env_next(Env *__restrict self) {
 LPWCH result_string,next_string;
 DREF DeeObject *name,*value,*result;
#ifdef CONFIG_NO_THREADS
 result_string = ATOMIC_READ(self->e_iter);
 if (!*result_string)
       return ITER_DONE;
 next_string = result_string;
 while (*next_string++);
 self->e_iter = next_string;
#else /* CONFIG_NO_THREADS */
 do {
  result_string = ATOMIC_READ(self->e_iter);
  if (!*result_string)
        return ITER_DONE;
  next_string = result_string;
  while (*next_string++);
 } while (!ATOMIC_CMPXCH(self->e_iter,result_string,next_string));
#endif /* !CONFIG_NO_THREADS */
 /* Split the line to extract the name and value. */
 next_string = result_string+1;
 /* XXX: This code assumes the double NUL-termination guarantied by windows. */
 while (*next_string++ && next_string[-1] != '=');
 name = DeeString_NewWide(result_string,
                         (size_t)(next_string-result_string)-1,
                          STRING_ERROR_FREPLAC);
 if unlikely(!name) goto err;
 value = DeeString_NewWide(next_string,
                           wcslen(next_string),
                           STRING_ERROR_FREPLAC);
 if unlikely(!value) goto err_name;
 result = DeeTuple_PackSymbolic(2,name,value); /* Inherit: name, value */
 if unlikely(!result) goto err_value;
 return result;
err_value:
 Dee_Decref(value);
err_name:
 Dee_Decref(name);
err:
 return NULL;
}

INTERN DREF DeeObject *DCALL
enviterator_next_key(DeeObject *__restrict self) {
 LPWCH result_string,next_string;
 Env *me = (Env *)self;
#ifdef CONFIG_NO_THREADS
 result_string = ATOMIC_READ(me->e_iter);
 if (!*result_string)
       return ITER_DONE;
 next_string = result_string;
 while (*next_string++);
 me->e_iter = next_string;
#else /* CONFIG_NO_THREADS */
 do {
  result_string = ATOMIC_READ(me->e_iter);
  if (!*result_string)
        return ITER_DONE;
  next_string = result_string;
  while (*next_string++);
 } while (!ATOMIC_CMPXCH(me->e_iter,result_string,next_string));
#endif /* !CONFIG_NO_THREADS */
 /* Split the line to extract the name and value. */
 next_string = result_string+1;
 /* XXX: This code assumes the double NUL-termination guarantied by windows. */
 while (*next_string++ && next_string[-1] != '=');
 return DeeString_NewWide(result_string,
                         (size_t)(next_string-result_string)-1,
                          STRING_ERROR_FREPLAC);
}

INTERN DREF DeeObject *DCALL
enviterator_next_value(DeeObject *__restrict self) {
 LPWCH result_string,next_string;
 Env *me = (Env *)self;
#ifdef CONFIG_NO_THREADS
 result_string = ATOMIC_READ(me->e_iter);
 if (!*result_string)
       return ITER_DONE;
 next_string = result_string;
 while (*next_string++);
 me->e_iter = next_string;
#else /* CONFIG_NO_THREADS */
 do {
  result_string = ATOMIC_READ(me->e_iter);
  if (!*result_string)
        return ITER_DONE;
  next_string = result_string;
  while (*next_string++);
 } while (!ATOMIC_CMPXCH(me->e_iter,result_string,next_string));
#endif /* !CONFIG_NO_THREADS */
 /* Split the line to extract the name and value. */
 next_string = result_string+1;
 /* XXX: This code assumes the double NUL-termination guarantied by windows. */
 while (*next_string++ && next_string[-1] != '=');
 return DeeString_NewWide(next_string,
                          wcslen(next_string),
                          STRING_ERROR_FREPLAC);
}


PRIVATE struct type_member env_members[] = {
    TYPE_MEMBER_CONST("seq",&DeeEnv_Singleton),
    TYPE_MEMBER_END
};

INTERN DeeTypeObject DeeEnvIterator_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"env.iterator",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FNORMAL|TP_FFINAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeIterator_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */(void *)&env_init,
                /* .tp_copy_ctor = */NULL,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */NULL,
                /* .tp_free      = */NULL,
                {
                    /* .tp_instance_size = */sizeof(Env)
                }
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&env_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */(int(DCALL *)(DeeObject *__restrict))&env_bool
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */NULL,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */NULL,
    /* .tp_seq           = */NULL,
    /* .tp_iter_next     = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&env_next,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */NULL,
    /* .tp_members       = */env_members,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */NULL
};



PRIVATE void DCALL
err_unknown_env_var(DeeObject *__restrict name) {
 DeeError_Throwf(&DeeError_KeyError,
                 "Unknown environment variable `%k'",
                 name);
}

INTERN bool DCALL
fs_hasenv(/*String*/DeeObject *__restrict name) {
 LPWSTR wname; bool result;
 wname = (LPWSTR)DeeString_AsWide(name);
 if unlikely(!wname) {
  DeeError_Handled(ERROR_HANDLED_RESTORE);
  return false;
 }
 DBG_ALIGNMENT_DISABLE();
 result = GetEnvironmentVariableW(wname,NULL,0) != 0;
 DBG_ALIGNMENT_ENABLE();
 return result;
}
INTERN DREF DeeObject *DCALL
fs_getenv(DeeObject *__restrict name, bool try_get) {
 LPWSTR buffer,new_buffer; DREF DeeObject *result;
 LPWSTR wname; DWORD bufsize = 256,error;
 wname = (LPWSTR)DeeString_AsWide(name);
 if unlikely(!wname) goto err_consume;
 buffer = DeeString_NewWideBuffer(bufsize);
 if unlikely(!buffer) goto err_consume;
 for (;;) {
  DBG_ALIGNMENT_DISABLE();
  error = GetEnvironmentVariableW(wname,buffer,bufsize+1);
  DBG_ALIGNMENT_ENABLE();
  if (!error) {
   /* Error. */
   if (!try_get)
        err_unknown_env_var(name);
   DeeString_FreeWideBuffer(buffer);
   goto err;
  }
  if (error <= bufsize) break;
  /* Resize to fit. */
  new_buffer = DeeString_ResizeWideBuffer(buffer,error);
  if unlikely(!new_buffer) goto err_result;
  buffer  = new_buffer;
  bufsize = error-1;
 }
 new_buffer = DeeString_TryResizeWideBuffer(buffer,error);
 if likely(new_buffer) buffer = new_buffer;
 result = DeeString_PackWideBuffer(buffer,STRING_ERROR_FREPLAC);
 if unlikely(!result) goto err_consume;
 return result;
err_result:
 DeeString_FreeWideBuffer(buffer);
err_consume:
 if (try_get)
     DeeError_Handled(ERROR_HANDLED_RESTORE);
err:
 return NULL;
}
INTERN int DCALL
fs_wprintenv(uint16_t const *__restrict name,
             struct unicode_printer *__restrict printer,
             bool try_get) {
 LPWSTR buffer; DWORD new_bufsize,bufsize = 256;
 buffer = unicode_printer_alloc_wchar(printer,bufsize);
 if unlikely(!buffer) goto err;
again:
 DBG_ALIGNMENT_DISABLE();
 new_bufsize = GetEnvironmentVariableW((LPCWSTR)name,buffer,bufsize+1);
 DBG_ALIGNMENT_ENABLE();
 if unlikely(!new_bufsize) {
  if (!try_get) {
   DeeError_Throwf(&DeeError_KeyError,
                   "Unknown environment variable `%I16s'",
                   name);
   goto err_release;
  }
  unicode_printer_free_wchar(printer,buffer);
  return 1; /* Not found. */
 }
 if (new_bufsize > bufsize) {
  LPWSTR new_buffer;
  /* Increase the buffer and try again. */
  new_buffer = unicode_printer_resize_wchar(printer,buffer,new_bufsize);
  if unlikely(!new_buffer) goto err_release;
  buffer  = new_buffer;
  bufsize = new_bufsize;
  goto again;
 }
 if (unicode_printer_confirm_wchar(printer,buffer,new_bufsize) < 0)
     goto err;
 return 0;
err_release:
 unicode_printer_free_wchar(printer,buffer);
err:
 return -1;
}
INTERN int DCALL
fs_printenv(/*utf-8*/char const *__restrict name,
            struct unicode_printer *__restrict printer,
            bool try_get) {
 LPWSTR buffer,wname; DWORD new_bufsize,bufsize = 256;
 DREF DeeObject *wide_name;
 wide_name = DeeString_NewUtf8(name,strlen(name),STRING_ERROR_FSTRICT);
 if unlikely(!wide_name) goto err;
 wname = DeeString_AsWide(wide_name);
 if unlikely(!wname) goto err_wide_name;
 buffer = unicode_printer_alloc_wchar(printer,bufsize);
 if unlikely(!buffer) goto err_wide_name;
again:
 DBG_ALIGNMENT_DISABLE();
 new_bufsize = GetEnvironmentVariableW(wname,buffer,bufsize+1);
 DBG_ALIGNMENT_ENABLE();
 if unlikely(!new_bufsize) {
  if (!try_get) {
   DeeError_Throwf(&DeeError_KeyError,
                   "Unknown environment variable `%s'",
                   name);
   goto err_release;
  }
  unicode_printer_free_wchar(printer,buffer);
  Dee_Decref(wide_name);
  return 1; /* Not found. */
 }
 if (new_bufsize > bufsize) {
  LPWSTR new_buffer;
  /* Increase the buffer and try again. */
  new_buffer = unicode_printer_resize_wchar(printer,buffer,new_bufsize);
  if unlikely(!new_buffer) goto err_release;
  buffer  = new_buffer;
  bufsize = new_bufsize;
  goto again;
 }
 if (unicode_printer_confirm_wchar(printer,buffer,new_bufsize) < 0)
     goto err;
 Dee_Decref(wide_name);
 return 0;
err_release:
 unicode_printer_free_wchar(printer,buffer);
err_wide_name:
 Dee_Decref(wide_name);
err:
 return -1;
}

INTERN int DCALL
fs_delenv(DeeObject *__restrict name) {
 LPWSTR wname;
 wname = (LPWSTR)DeeString_AsWide(name);
 if unlikely(!wname) goto err;
 DBG_ALIGNMENT_DISABLE();
 if (!SetEnvironmentVariableW(wname,NULL)) {
  DBG_ALIGNMENT_ENABLE();
  err_unknown_env_var(name);
  goto err;
 }
 DBG_ALIGNMENT_ENABLE();
 return DeeNotify_Broadcast(NOTIFICATION_CLASS_ENVIRON,name);
err:
 return -1;
}

INTERN int DCALL
fs_setenv(DeeObject *__restrict name,
          DeeObject *__restrict value) {
 LPWSTR wname,wvalue;
 wname = (LPWSTR)DeeString_AsWide(name);
 if unlikely(!wname) goto err;
 wvalue = (LPWSTR)DeeString_AsWide(value);
 if unlikely(!wvalue) goto err;
again_setenv:
 DBG_ALIGNMENT_DISABLE();
 if (!SetEnvironmentVariableW(wname,wvalue)) {
  DWORD dwError;
  dwError = GetLastError();
  DBG_ALIGNMENT_ENABLE();
  if (nt_IsBadAlloc(dwError)) {
   if (Dee_CollectMemory(1))
       goto again_setenv;
  } else {
   DeeError_SysThrowf(&DeeError_SystemError,dwError,
                      "Failed to set environment variable `%k' to `%k'",
                      name,value);
  }
  goto err;
 }
 DBG_ALIGNMENT_ENABLE();
 /* Broadcast an environ-changed notification. */
 return DeeNotify_Broadcast(NOTIFICATION_CLASS_ENVIRON,name);
err:
 return -1;
}

INTERN DREF /*String*/DeeObject *DCALL fs_gethostname(void) {
 DWORD bufsize = MAX_COMPUTERNAME_LENGTH + 1;
 LPWSTR buffer,new_buffer;
 if (DeeThread_CheckInterrupt()) goto err;
 buffer = DeeString_NewWideBuffer(bufsize - 1);
 if unlikely(!buffer) goto err;
again:
 DBG_ALIGNMENT_DISABLE();
 if (!GetComputerNameW(buffer,&bufsize)) {
  DWORD error = GetLastError();
  DBG_ALIGNMENT_ENABLE();
  if (error == ERROR_BUFFER_OVERFLOW && bufsize &&
      bufsize - 1 > WSTR_LENGTH(buffer)) {
   new_buffer = DeeString_ResizeWideBuffer(buffer,bufsize - 1);
   if unlikely(!new_buffer) goto err_result;
   buffer = new_buffer;
   goto again;
  }
  if (nt_IsBadAlloc(error)) {
   if (Dee_CollectMemory(1))
       goto again;
   goto err_result;
  }
  DeeError_Throwf(&DeeError_SystemError,
                  "Failed to retrieve the name of the hosting machine");
  goto err_result;
 }
 DBG_ALIGNMENT_ENABLE();
 /* Truncate the buffer and return it. */
 new_buffer = DeeString_TryResizeWideBuffer(buffer,bufsize);
 if likely(new_buffer) buffer = new_buffer;
 return DeeString_PackWideBuffer(buffer,STRING_ERROR_FREPLAC);
err_result:
 DeeString_FreeWideBuffer(buffer);
err:
 return NULL;
}

PRIVATE DEFINE_STRING(tmpdir_0,"TMPDIR");
PRIVATE DEFINE_STRING(tmpdir_1,"TMP");
PRIVATE DEFINE_STRING(tmpdir_2,"TEMP");
PRIVATE DEFINE_STRING(tmpdir_3,"TEMPDIR");
PRIVATE DeeObject *tmpdir_vars[] = {
    (DeeObject *)&tmpdir_0,
    (DeeObject *)&tmpdir_1,
    (DeeObject *)&tmpdir_2,
    (DeeObject *)&tmpdir_3
};

INTERN DREF DeeObject *DCALL fs_gettmp(void) {
 DREF DeeObject *result; size_t i;
 if (DeeThread_CheckInterrupt()) goto err;
 for (i = 0; i < COMPILER_STRLEN(tmpdir_vars); ++i)
     if ((result = fs_getenv(tmpdir_vars[i],true)) != NULL)
         goto done;
 /* Fallback: Lookup using windows. */
 result = nt_GetTempPath();
done:
 return result;
err:
 return NULL;
}


PRIVATE ATTR_COLD int DCALL
err_path_exists(int error, DeeObject *__restrict path) {
 return DeeError_SysThrowf(&DeeError_FileExists,error,
                           "Path %r already exists",
                           path);
}
PRIVATE ATTR_COLD int DCALL
err_path_no_dir(DWORD error, DeeObject *__restrict path) {
 return DeeError_SysThrowf(&DeeError_NoDirectory,error,
                           "Some part of the path %r is not a directory",
                           path);
}
PRIVATE ATTR_COLD int DCALL
err_path_is_dir(int error, DeeObject *__restrict path) {
 return DeeError_SysThrowf(&DeeError_IsDirectory,error,
                           "Path %r is a directory",
                           path);
}
PRIVATE ATTR_COLD int DCALL
err_path_not_found(DWORD error, DeeObject *__restrict path) {
 return DeeError_SysThrowf(&DeeError_FileNotFound,error,
                           "Path %r could not be found",
                           path);
}
PRIVATE ATTR_COLD int DCALL
err_file_not_found(DWORD error, DeeObject *__restrict path) {
 return DeeError_SysThrowf(&DeeError_FileNotFound,error,
                           "File %r could not be found",
                           path);
}
PRIVATE ATTR_COLD int DCALL
err_file_no_write_access(DWORD error, DeeObject *__restrict path) {
 return DeeError_SysThrowf(&DeeError_AccessError,error,
                           "Write permissions have not been granted for file %r",
                           path);
}
PRIVATE ATTR_COLD int DCALL
err_path_no_write_access(int error, DeeObject *__restrict path) {
 return DeeError_SysThrowf(&DeeError_AccessError,error,
                           "Write permissions have not been granted for path %r",
                           path);
}
PRIVATE ATTR_COLD int DCALL
err_path_busy(DWORD error, DeeObject *__restrict path) {
 return DeeError_SysThrowf(&DeeError_BusyFile,error,
                           "Path %r cannot be deleted because it is still in use",
                           path);
}
PRIVATE ATTR_COLD int DCALL
err_path_no_access(DWORD error, DeeObject *__restrict path) {
 return DeeError_SysThrowf(&DeeError_AccessError,error,
                           "Search permissions are not granted for path %r",
                           path);
}
PRIVATE ATTR_COLD int DCALL
err_path_not_empty(DWORD error, DeeObject *__restrict path) {
 return DeeError_SysThrowf(&DeeError_NotEmpty,error,
                           "The directory %r cannot be deleted because it is not empty",
                           path);
}
PRIVATE ATTR_COLD int DCALL
err_chtime_no_access(DWORD error, DeeObject *__restrict path) {
 return DeeError_SysThrowf(&DeeError_AccessError,error,
                           "Changes to the selected timestamps of %r are not allowed",
                           path);
}
PRIVATE ATTR_COLD int DCALL
err_chattr_no_access(DWORD error, DeeObject *__restrict path) {
 return DeeError_SysThrowf(&DeeError_AccessError,error,
                           "Changes to the attributes of %r are not allowed",
                           path);
}
PRIVATE ATTR_COLD int DCALL
err_handle_closed(DWORD error, DeeObject *__restrict path) {
 return DeeError_SysThrowf(&DeeError_HandleClosed,error,
                           "The given handle %r has been closed",
                           path);
}
PRIVATE ATTR_COLD int DCALL
err_path_no_access2(int error,
                    DeeObject *__restrict existing_path,
                    DeeObject *__restrict new_path) {
 return DeeError_SysThrowf(&DeeError_AccessError,error,
                           "Access to %r or %r has not been granted",
                           existing_path,new_path);
}
PRIVATE ATTR_COLD int DCALL
err_path_not_found2(int error,
                    DeeObject *__restrict existing_path,
                    DeeObject *__restrict new_path) {
 return DeeError_SysThrowf(&DeeError_FileNotFound,error,
                           "Path %r or %r could not be found",
                           existing_path,new_path);
}
PRIVATE ATTR_COLD int DCALL
err_path_crossdev2(int error,
                   DeeObject *__restrict existing_path,
                   DeeObject *__restrict new_path) {
 return DeeError_SysThrowf(&DeeError_CrossDevice,error,
                           "Paths %r and %r are not apart of the same filesystem",
                           existing_path,new_path);
}




PRIVATE ATTR_COLD int DCALL err_getcwd(DWORD dwError) {
 if (nt_IsAccessDenied(dwError)) {
  return DeeError_SysThrowf(&DeeError_AccessError,dwError,
                            "Permission to read a part of the current "
                            "working directory's path was denied");
 } else if (nt_IsFileNotFound(dwError)) {
  return DeeError_SysThrowf(&DeeError_FileNotFound,dwError,
                            "The current working directory has been unlinked");
 }
 return DeeError_SysThrowf(&DeeError_FSError,dwError,
                           "Failed to retrieve the current working directory");
}

INTERN int DCALL
fs_printcwd(struct unicode_printer *__restrict printer) {
 LPWSTR buffer; DWORD new_bufsize,bufsize = PATH_MAX;
 if (DeeThread_CheckInterrupt()) goto err;
 buffer = unicode_printer_alloc_wchar(printer,bufsize);
 if unlikely(!buffer) goto err;
again:
 DBG_ALIGNMENT_DISABLE();
 new_bufsize = GetCurrentDirectoryW(bufsize+1,buffer);
 if unlikely(!new_bufsize) {
  DWORD dwError;
  dwError = GetLastError();
  DBG_ALIGNMENT_ENABLE();
  if (nt_IsBadAlloc(dwError)) {
   if (Dee_CollectMemory(1))
       goto again;
  } else {
   err_getcwd(dwError);
  }
  goto err_release;
 }
 DBG_ALIGNMENT_ENABLE();
 if (new_bufsize > bufsize) {
  LPWSTR new_buffer;
  /* Increase the buffer and try again. */
  new_buffer = unicode_printer_resize_wchar(printer,buffer,new_bufsize);
  if unlikely(!new_buffer) goto err_release;
  buffer  = new_buffer;
  bufsize = new_bufsize;
  goto again;
 }
 if (unicode_printer_confirm_wchar(printer,buffer,new_bufsize) < 0)
     goto err;
 return 0;
err_release:
 unicode_printer_free_wchar(printer,buffer);
err:
 return -1;
}


INTERN DREF DeeObject *DCALL fs_getcwd(void) {
 LPWSTR buffer,new_buffer; DWORD bufsize = PATH_MAX,new_bufsize;
 if (DeeThread_CheckInterrupt()) goto err;
 buffer = DeeString_NewWideBuffer(bufsize);
 if unlikely(!buffer) goto err;
 for (;;) {
again:
  DBG_ALIGNMENT_DISABLE();
  new_bufsize = GetCurrentDirectoryW(bufsize+1,buffer);
  if (!new_bufsize) {
   DWORD dwError;
   dwError = GetLastError();
   DBG_ALIGNMENT_ENABLE();
   if (nt_IsBadAlloc(dwError)) {
    if (Dee_CollectMemory(1))
        goto again;
   } else {
    err_getcwd(dwError);
   }
   goto err_result;
  }
  DBG_ALIGNMENT_ENABLE();
  if (new_bufsize <= bufsize) break;
  /* Resize to fit. */
  new_buffer = DeeString_ResizeWideBuffer(buffer,new_bufsize);
  if unlikely(!new_buffer) goto err_result;
  buffer  = new_buffer;
  bufsize = new_bufsize;
 }
 new_buffer = DeeString_TryResizeWideBuffer(buffer,new_bufsize);
 if likely(new_buffer) buffer = new_buffer;
 return DeeString_PackWideBuffer(buffer,STRING_ERROR_FREPLAC);
err_result:
 DeeString_FreeWideBuffer(buffer);
err:
 return NULL;
}



INTERN int DCALL fs_chdir(DeeObject *__restrict path) {
 int result;
 if (DeeThread_CheckInterrupt()) goto err;
 if (!DeeString_Check(path)) {
  if (DeeInt_Check(path)) {
   HANDLE fd; /* Support for descriptor-based chdir() */
   if (DeeObject_AsUIntptr(path,(uintptr_t *)&fd))
       goto err;
   path = nt_GetFilenameOfHandle(fd);
   if unlikely(!path) goto err;
   result = fs_chdir(path);
   Dee_Decref(path);
   return result;
  }
  path = DeeFile_Filename(path);
  if unlikely(!path) goto err;
  result = fs_chdir(path);
  Dee_Decref(path);
  return result;
 }
again:
 /* Allow an empty path as an alias for `chdir(".")' (aka. no-op)
  * Windows would normally set an `ERROR_INVALID_NAME' error for this case. */
 if (DeeString_WLEN(path) == 0) goto done;
 result = nt_SetCurrentDirectory(path);
 if unlikely(result != 0) {
  DWORD dwError;
  if unlikely(result < 0) goto err;
  DBG_ALIGNMENT_DISABLE();
  dwError = GetLastError();
  DBG_ALIGNMENT_ENABLE();
  if (nt_IsBadAlloc(dwError)) {
   if (Dee_CollectMemory(1))
       goto again;
   goto err;
  }
  if (nt_IsNotDir(dwError))
      goto do_throw_not_dir;
  if (dwError == ERROR_ACCESS_DENIED) {
   DWORD dwAttributes;
   /* Check if the path is actually a directory. */
   result = nt_GetFileAttributes(path,&dwAttributes);
   if (result < 0) goto err;
   if (result == 0 && !(dwAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
do_throw_not_dir:
    err_path_no_dir(dwError,path);
    goto err;
   }
  }
  if (nt_IsFileNotFound(dwError)) {
   err_path_not_found(dwError,path);
  } else if (nt_IsAccessDenied(dwError)) {
   err_path_no_access(dwError,path);
  } else {
   DeeError_SysThrowf(&DeeError_FSError,dwError,
                      "Failed to change the current working directory to %r",
                      path);
  }
  goto err;
 }
done:
 return 0;
err:
 return -1;
}


typedef BOOL (WINAPI *LPGETUSERPROFILEDIRECTORYW)(HANDLE hToken, LPWSTR lpProfileDir, LPDWORD lpcchSize);
typedef BOOL (WINAPI *LPGETPROFILESDIRECTORYW)(LPWSTR lpProfileDir, LPDWORD lpcchSize);
typedef BOOL (WINAPI *LPGETDEFAULTUSERPROFILEDIRECTORYW)(LPWSTR lpProfileDir, LPDWORD lpcchSize);
typedef BOOL (WINAPI *LPGETALLUSERSPROFILEDIRECTORYW)(LPWSTR lpProfileDir, LPDWORD lpcchSize);
PRIVATE LPGETUSERPROFILEDIRECTORYW p_GetUserProfileDirectoryW = NULL;
PRIVATE LPGETPROFILESDIRECTORYW p_GetProfilesDirectoryW = NULL;
PRIVATE LPGETDEFAULTUSERPROFILEDIRECTORYW p_GetDefaultUserProfileDirectoryW = NULL;
PRIVATE LPGETALLUSERSPROFILEDIRECTORYW p_GetAllUsersProfileDirectoryW = NULL;
PRIVATE char const s_GetUserProfileDirectoryW[] = "GetUserProfileDirectoryW";
PRIVATE char const s_GetProfilesDirectoryW[] = "GetProfilesDirectoryW";
PRIVATE char const s_GetDefaultUserProfileDirectoryW[] = "GetDefaultUserProfileDirectoryW";
PRIVATE char const s_GetAllUsersProfileDirectoryW[] = "GetAllUsersProfileDirectoryW";
PRIVATE WCHAR s_USERENV[] = { 'U','S','E','R','E','N','V',0 };
PRIVATE WCHAR s_userenv_dll[] = { 'u','s','e','r','e','n','v','.','d','l','l',0 };

PRIVATE HMODULE DCALL get_userenv_module(void) {
 HMODULE result = GetModuleHandleW(s_USERENV);
 if (!result) result = LoadLibraryW(s_userenv_dll);
 return result;
}

INTERN int DCALL
nt_printhome_token(struct unicode_printer *__restrict printer, void *hToken, bool bTryGet) {
 LPGETUSERPROFILEDIRECTORYW my_GetUserProfileDirectoryW;
 LPWSTR wBuffer; DWORD dwBufsize;
 my_GetUserProfileDirectoryW = p_GetUserProfileDirectoryW;
 if (*(void **)&my_GetUserProfileDirectoryW == (void *)NULL) {
  /* Load the module */
  HMODULE hUserEnv;
  DBG_ALIGNMENT_DISABLE();
  hUserEnv = get_userenv_module();
  if (hUserEnv)
      my_GetUserProfileDirectoryW = (LPGETUSERPROFILEDIRECTORYW)GetProcAddress(hUserEnv,"GetUserProfileDirectoryW");
  DBG_ALIGNMENT_ENABLE();
  if (!my_GetUserProfileDirectoryW)
    *(void **)&my_GetUserProfileDirectoryW = (void *)ITER_DONE;
  p_GetUserProfileDirectoryW = my_GetUserProfileDirectoryW;
 }
 if (*(void **)&my_GetUserProfileDirectoryW == (void *)ITER_DONE) {
  if (bTryGet) return 1;
  return DeeError_SysThrowf(&DeeError_UnsupportedAPI,SYSTEM_ERROR_UNKNOWN,
                            "Cannot determine home of token %p (`GetUserProfileDirectoryW' not found)",
                            hToken);
 }
 dwBufsize = PATH_MAX;
 wBuffer = unicode_printer_alloc_wchar(printer,dwBufsize);
 if unlikely(!wBuffer) goto err;
 DBG_ALIGNMENT_DISABLE();
 while (!(*my_GetUserProfileDirectoryW)((HANDLE)hToken,wBuffer,&dwBufsize)) {
  DWORD dwError = GetLastError();
  DBG_ALIGNMENT_ENABLE();
  if (dwError == ERROR_INSUFFICIENT_BUFFER ||
      dwError == ERROR_MORE_DATA) {
   LPWSTR wNewBuffer;
   wNewBuffer = unicode_printer_resize_wchar(printer,wBuffer,dwBufsize - 1);
   if unlikely(!wNewBuffer) goto err_release;
   wBuffer = wNewBuffer;
  } else {
   if (bTryGet) { unicode_printer_free_wchar(printer,wBuffer); return 1; }
   DeeError_SysThrowf(&DeeError_SystemError,dwError,
                      "Failed to determine home of token %p",
                      hToken);
   goto err_release;
  }
  DBG_ALIGNMENT_DISABLE();
 }
 DBG_ALIGNMENT_ENABLE();
 if (unicode_printer_confirm_wchar(printer,wBuffer,dwBufsize - 1) < 0)
     goto err;
 return 0;
err_release:
 unicode_printer_free_wchar(printer,wBuffer);
err:
 return -1;
}

INTERN int DCALL
nt_print_GetProfilesDirectory(struct unicode_printer *__restrict printer, bool bTryGet) {
 LPGETPROFILESDIRECTORYW my_GetProfilesDirectoryW;
 LPWSTR wBuffer; DWORD dwBufsize;
 my_GetProfilesDirectoryW = p_GetProfilesDirectoryW;
 if (*(void **)&my_GetProfilesDirectoryW == (void *)NULL) {
  /* Load the module */
  HMODULE hUserEnv;
  DBG_ALIGNMENT_DISABLE();
  hUserEnv = get_userenv_module();
  if (hUserEnv)
      my_GetProfilesDirectoryW = (LPGETPROFILESDIRECTORYW)GetProcAddress(hUserEnv,"GetProfilesDirectoryW");
  DBG_ALIGNMENT_ENABLE();
  if (!my_GetProfilesDirectoryW)
    *(void **)&my_GetProfilesDirectoryW = (void *)ITER_DONE;
  p_GetProfilesDirectoryW = my_GetProfilesDirectoryW;
 }
 if (*(void **)&my_GetProfilesDirectoryW == (void *)ITER_DONE) {
  if (bTryGet) return 1;
  return DeeError_SysThrowf(&DeeError_UnsupportedAPI,SYSTEM_ERROR_UNKNOWN,
                            "Cannot determine profiles directory (`GetProfilesDirectoryW' not found)");
 }
 dwBufsize = PATH_MAX;
 wBuffer = unicode_printer_alloc_wchar(printer,dwBufsize);
 if unlikely(!wBuffer) goto err;
 DBG_ALIGNMENT_DISABLE();
 while (!(*my_GetProfilesDirectoryW)(wBuffer,&dwBufsize)) {
  DWORD dwError = GetLastError();
  DBG_ALIGNMENT_ENABLE();
  if (dwError == ERROR_INSUFFICIENT_BUFFER ||
      dwError == ERROR_MORE_DATA) {
   LPWSTR wNewBuffer;
   wNewBuffer = unicode_printer_resize_wchar(printer,wBuffer,dwBufsize - 1);
   if unlikely(!wNewBuffer) goto err_release;
   wBuffer = wNewBuffer;
  } else {
   if (bTryGet) { unicode_printer_free_wchar(printer,wBuffer); return 1; }
   DeeError_SysThrowf(&DeeError_SystemError,dwError,
                      "Failed to determine profiles directory");
   goto err_release;
  }
  DBG_ALIGNMENT_DISABLE();
 }
 DBG_ALIGNMENT_ENABLE();
 if (unicode_printer_confirm_wchar(printer,wBuffer,dwBufsize - 1) < 0)
     goto err;
 return 0;
err_release:
 unicode_printer_free_wchar(printer,wBuffer);
err:
 return -1;
}


INTERN int DCALL
nt_print_GetDefaultUserProfileDirectory(struct unicode_printer *__restrict printer, bool bTryGet) {
 LPGETDEFAULTUSERPROFILEDIRECTORYW my_GetDefaultUserProfileDirectoryW;
 LPWSTR wBuffer; DWORD dwBufsize;
 my_GetDefaultUserProfileDirectoryW = p_GetDefaultUserProfileDirectoryW;
 if (*(void **)&my_GetDefaultUserProfileDirectoryW == (void *)NULL) {
  /* Load the module */
  HMODULE hUserEnv;
  DBG_ALIGNMENT_DISABLE();
  hUserEnv = get_userenv_module();
  if (hUserEnv)
      my_GetDefaultUserProfileDirectoryW = (LPGETDEFAULTUSERPROFILEDIRECTORYW)GetProcAddress(hUserEnv,"GetDefaultUserProfileDirectoryW");
  DBG_ALIGNMENT_ENABLE();
  if (!my_GetDefaultUserProfileDirectoryW)
    *(void **)&my_GetDefaultUserProfileDirectoryW = (void *)ITER_DONE;
  p_GetDefaultUserProfileDirectoryW = my_GetDefaultUserProfileDirectoryW;
 }
 if (*(void **)&my_GetDefaultUserProfileDirectoryW == (void *)ITER_DONE) {
  if (bTryGet) return 1;
  return DeeError_SysThrowf(&DeeError_UnsupportedAPI,SYSTEM_ERROR_UNKNOWN,
                            "Cannot determine profiles directory (`GetDefaultUserProfileDirectoryW' not found)");
 }
 dwBufsize = PATH_MAX;
 wBuffer = unicode_printer_alloc_wchar(printer,dwBufsize);
 if unlikely(!wBuffer) goto err;
 DBG_ALIGNMENT_DISABLE();
 while (!(*my_GetDefaultUserProfileDirectoryW)(wBuffer,&dwBufsize)) {
  DWORD dwError = GetLastError();
  DBG_ALIGNMENT_ENABLE();
  if (dwError == ERROR_INSUFFICIENT_BUFFER ||
      dwError == ERROR_MORE_DATA) {
   LPWSTR wNewBuffer;
   wNewBuffer = unicode_printer_resize_wchar(printer,wBuffer,dwBufsize - 1);
   if unlikely(!wNewBuffer) goto err_release;
   wBuffer = wNewBuffer;
  } else {
   if (bTryGet) { unicode_printer_free_wchar(printer,wBuffer); return 1; }
   DeeError_SysThrowf(&DeeError_SystemError,dwError,
                      "Failed to determine profiles directory");
   goto err_release;
  }
  DBG_ALIGNMENT_DISABLE();
 }
 DBG_ALIGNMENT_ENABLE();
 if (unicode_printer_confirm_wchar(printer,wBuffer,dwBufsize - 1) < 0)
     goto err;
 return 0;
err_release:
 unicode_printer_free_wchar(printer,wBuffer);
err:
 return -1;
}

INTERN int DCALL
nt_print_GetAllUsersProfileDirectory(struct unicode_printer *__restrict printer, bool bTryGet) {
 LPGETALLUSERSPROFILEDIRECTORYW my_GetAllUsersProfileDirectoryW;
 LPWSTR wBuffer; DWORD dwBufsize;
 my_GetAllUsersProfileDirectoryW = p_GetAllUsersProfileDirectoryW;
 if (*(void **)&my_GetAllUsersProfileDirectoryW == (void *)NULL) {
  /* Load the module */
  HMODULE hUserEnv;
  DBG_ALIGNMENT_DISABLE();
  hUserEnv = get_userenv_module();
  if (hUserEnv)
      my_GetAllUsersProfileDirectoryW = (LPGETALLUSERSPROFILEDIRECTORYW)GetProcAddress(hUserEnv,"GetAllUsersProfileDirectoryW");
  DBG_ALIGNMENT_ENABLE();
  if (!my_GetAllUsersProfileDirectoryW)
    *(void **)&my_GetAllUsersProfileDirectoryW = (void *)ITER_DONE;
  p_GetAllUsersProfileDirectoryW = my_GetAllUsersProfileDirectoryW;
 }
 if (*(void **)&my_GetAllUsersProfileDirectoryW == (void *)ITER_DONE) {
  if (bTryGet) return 1;
  return DeeError_SysThrowf(&DeeError_UnsupportedAPI,SYSTEM_ERROR_UNKNOWN,
                            "Cannot determine profiles directory (`GetAllUsersProfileDirectoryW' not found)");
 }
 dwBufsize = PATH_MAX;
 wBuffer = unicode_printer_alloc_wchar(printer,dwBufsize);
 if unlikely(!wBuffer) goto err;
 DBG_ALIGNMENT_DISABLE();
 while (!(*my_GetAllUsersProfileDirectoryW)(wBuffer,&dwBufsize)) {
  DWORD dwError = GetLastError();
  DBG_ALIGNMENT_ENABLE();
  if (dwError == ERROR_INSUFFICIENT_BUFFER ||
      dwError == ERROR_MORE_DATA) {
   LPWSTR wNewBuffer;
   wNewBuffer = unicode_printer_resize_wchar(printer,wBuffer,dwBufsize - 1);
   if unlikely(!wNewBuffer) goto err_release;
   wBuffer = wNewBuffer;
  } else {
   if (bTryGet) { unicode_printer_free_wchar(printer,wBuffer); return 1; }
   DeeError_SysThrowf(&DeeError_SystemError,dwError,
                      "Failed to determine profiles directory");
   goto err_release;
  }
  DBG_ALIGNMENT_DISABLE();
 }
 DBG_ALIGNMENT_ENABLE();
 if (unicode_printer_confirm_wchar(printer,wBuffer,dwBufsize - 1) < 0)
     goto err;
 return 0;
err_release:
 unicode_printer_free_wchar(printer,wBuffer);
err:
 return -1;
}


INTERN int DCALL
nt_printhome_process(struct unicode_printer *__restrict printer, void *hProcess, bool bTryGet) {
 int result; HANDLE hProcessToken;
 DBG_ALIGNMENT_DISABLE();
 if unlikely(!OpenProcessToken((HANDLE)hProcess,TOKEN_QUERY,&hProcessToken)) {
  DWORD dwError;
  DBG_ALIGNMENT_ENABLE();
  if (bTryGet) return 1;
  DBG_ALIGNMENT_DISABLE();
  dwError = GetLastError();
  DBG_ALIGNMENT_ENABLE();
  return DeeError_SysThrowf(&DeeError_SystemError,dwError,
                            "Failed to determine home of process %p",
                            hProcess);
 }
 result = nt_printhome_token(printer,(void *)hProcessToken,bTryGet);
 CloseHandle(hProcessToken);
 return result;
}

INTERN int DCALL
fs_printhome(struct unicode_printer *__restrict printer, bool try_get) {
 PRIVATE uint16_t const var_HOME[] = { 'H','O','M','E',0 };
 PRIVATE uint16_t const var_USERPROFILE[] = { 'U','S','E','R','P','R','O','F','I','L','E',0 };
 PRIVATE uint16_t const var_HOMEDRIVE[] = { 'H','O','M','E','D','R','I','V','E',0 };
 PRIVATE uint16_t const var_HOMEPATH[] = { 'H','O','M','E','P','A','T','H',0 };
 int error; size_t old_length;
 if ((error = fs_wprintenv(var_HOME,printer,true)) <= 0) goto done_error;
 if ((error = fs_wprintenv(var_USERPROFILE,printer,true)) <= 0) goto done_error;
 old_length = UNICODE_PRINTER_LENGTH(printer);
 if ((error = fs_wprintenv(var_HOMEDRIVE,printer,true)) <= 0) {
  size_t before_homepath_index;
  if unlikely(error < 0) goto done_error;
  before_homepath_index = UNICODE_PRINTER_LENGTH(printer);
  while (before_homepath_index) {
   uint32_t ch;
   ch = UNICODE_PRINTER_GETCHAR(printer,before_homepath_index - 1);
   if (ch != '/' && ch != '\\' && !DeeUni_IsSpace(ch)) break;
   --before_homepath_index;
  }
  unicode_printer_truncate(printer,before_homepath_index);
  if (unicode_printer_putascii(printer,'\\')) goto err;
  ++before_homepath_index;
  error = fs_wprintenv(var_HOMEPATH,printer,true);
  if (error <= 0) {
   size_t remove_count;
   if unlikely(error < 0)
      goto done_error;
   /* Remove any additional leading slashes/space characters from $HOMEPATH */
   remove_count = 0;
   while (before_homepath_index + remove_count < UNICODE_PRINTER_LENGTH(printer)) {
    uint32_t ch;
    ch = UNICODE_PRINTER_GETCHAR(printer,
                                 before_homepath_index +
                                 remove_count);
    if (ch != '/' && ch != '\\' && !DeeUni_IsSpace(ch)) break;
    ++remove_count;
   }
   if (remove_count) {
    unicode_printer_memmove(printer,
                            before_homepath_index,
                            before_homepath_index + remove_count,
                            UNICODE_PRINTER_LENGTH(printer) -
                           (before_homepath_index + remove_count));
    unicode_printer_truncate(printer,UNICODE_PRINTER_LENGTH(printer) - remove_count);
   }
   goto done_error;
  }
  unicode_printer_truncate(printer,old_length);
 }
 /* Environment variables aren't holding the key...
  * -> Let's try something else. */
 return nt_printhome_process(printer,GetCurrentProcess(),try_get);
done_error:
 return error;
err:
 return -1;
}

INTERN int DCALL
fs_printuser(struct unicode_printer *__restrict printer, bool try_get) {
 DWORD dwBufsize = 64 + 1;
 LPWSTR wBuffer = unicode_printer_alloc_wchar(printer,dwBufsize - 1);
 if unlikely(!wBuffer) goto err;
 while (!GetUserNameW(wBuffer,&dwBufsize)) {
  DWORD dwError = GetLastError();
  if (dwError == ERROR_BUFFER_OVERFLOW ||
      dwError == ERROR_INSUFFICIENT_BUFFER ||
      dwError == ERROR_MORE_DATA) {
   LPWSTR wNewBuffer;
   wNewBuffer = unicode_printer_resize_wchar(printer,wBuffer,dwBufsize - 1);
   if unlikely(!wNewBuffer) goto err_release;
  } else {
   if (try_get) { unicode_printer_free_wchar(printer,wBuffer); return 1; }
   DeeError_SysThrowf(&DeeError_SystemError,dwError,
                      "Failed to determine name of the current user");
   goto err_release;
  }
 }
 if (unicode_printer_confirm_wchar(printer,wBuffer,dwBufsize - 1) < 0)
     goto err;
 return 0;
err_release:
 unicode_printer_free_wchar(printer,wBuffer);
err:
 return -1;
}



INTERN DREF DeeObject *DCALL
fs_gethome(bool try_get) {
 struct unicode_printer printer = UNICODE_PRINTER_INIT;
 if (fs_printhome(&printer,try_get))
     goto err;
 return unicode_printer_pack(&printer);
err:
 unicode_printer_fini(&printer);
 return NULL;
}

INTERN DREF DeeObject *DCALL
fs_getuser(bool try_get) {
 struct unicode_printer printer = UNICODE_PRINTER_INIT;
 if (fs_printuser(&printer,try_get))
     goto err;
 return unicode_printer_pack(&printer);
err:
 unicode_printer_fini(&printer);
 return NULL;
}

#include <aclapi.h>

/* User (SSID) implementation. */
struct user_object {
    OBJECT_HEAD
    /* TODO: SSID */
    PSECURITY_DESCRIPTOR u_sd;   /* [0..1][const][owned(LocalFree)] The security descriptor buffer, or `NULL' if the current user is being referred to. */
    PSID                 u_sid;  /* [0..1][valid_if(u_sd != NULL)][const] The SID descriptor of this user-object */
};

PRIVATE int DCALL
user_init(struct user_object *__restrict self,
          size_t argc, DeeObject **__restrict argv) {
 DREF DeeObject *name_or_id = NULL;
 if (DeeArg_Unpack(argc,argv,"|o:user",&name_or_id))
     goto err;
 if (!name_or_id) {
  self->u_sd  = NULL;
  self->u_sid = NULL;
 } else {
  /* TODO */
  DERROR_NOTIMPLEMENTED();
  goto err;
 }
 return 0;
err:
 return -1;
}


PRIVATE DREF DeeObject *DCALL
default_user_get_home(DeeObject *__restrict UNUSED(self)) {
 return fs_gethome(false);
}

PRIVATE DREF DeeObject *DCALL
default_user_get_domain(DeeObject *__restrict UNUSED(self)) {
 return fs_gethostname();
}

PRIVATE DREF DeeObject *DCALL
default_user_get_name(DeeObject *__restrict UNUSED(self)) {
 return fs_getuser(false);
}

PRIVATE DREF DeeObject *DCALL
user_get_profiles_dir(DeeObject *__restrict UNUSED(self)) {
 struct unicode_printer printer = UNICODE_PRINTER_INIT;
 if (nt_print_GetProfilesDirectory(&printer,false))
     goto err;
 return unicode_printer_pack(&printer);
err:
 unicode_printer_fini(&printer);
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
user_get_defaulthome(DeeObject *__restrict UNUSED(self)) {
 struct unicode_printer printer = UNICODE_PRINTER_INIT;
 if (nt_print_GetDefaultUserProfileDirectory(&printer,false))
     goto err;
 return unicode_printer_pack(&printer);
err:
 unicode_printer_fini(&printer);
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
user_get_allusershome(DeeObject *__restrict UNUSED(self)) {
 struct unicode_printer printer = UNICODE_PRINTER_INIT;
 if (nt_print_GetAllUsersProfileDirectory(&printer,false))
     goto err;
 return unicode_printer_pack(&printer);
err:
 unicode_printer_fini(&printer);
 return NULL;
}

PRIVATE struct type_getset user_class_getsets[] = {
    { "name", &default_user_get_name, NULL, NULL, DeeUser_static_name_doc },
    { "home", &default_user_get_home, NULL, NULL, DeeUser_static_home_doc },
    { "domain_np", &default_user_get_domain, NULL, NULL,
      DOC("->string\n"
          "Alias for :gethostname") },
    { "profilesdir_np", &user_get_profiles_dir, NULL, NULL,
      DOC("->string\n"
          "Returns the windows profiles directory containing all user profiles (Usually ${r\"C:\\Users\"})") },
    { "defaultuserhome_np", &user_get_defaulthome, NULL, NULL,
      DOC("->string\n"
          "Returns the home folder of the default profile (Usually ${r\"C:\\Users\\Default\"})") },
    { "allusershome_np", &user_get_allusershome, NULL, NULL,
      DOC("->string\n"
          "Returns the home folder of all profiles (Usually ${r\"C:\\ProgramData\"})") },
    { NULL }
};


PRIVATE void DCALL
user_fini(struct user_object *__restrict self) {
 LocalFree(self->u_sd);
}

PRIVATE int DCALL
user_get_name_and_domain(struct user_object *__restrict self,
                         LPWSTR *__restrict pname,
                         LPWSTR *__restrict pdomain,
                         PSID_NAME_USE peUse) {
 LPWSTR wNameBuffer; DWORD wNameBufSize = 64 + 1;
 LPWSTR wDomainBuffer; DWORD wDomainBufSize = 64 + 1;
 LPWSTR wNewBuffer;
 wNameBuffer   = DeeString_NewWideBuffer(wNameBufSize - 1);
 if unlikely(!wNameBuffer) goto err;
 wDomainBuffer = DeeString_NewWideBuffer(wDomainBufSize - 1);
 if unlikely(!wDomainBuffer) goto err_name_buffer;
 DBG_ALIGNMENT_DISABLE();
 while (!LookupAccountSidW(NULL,
                           self->u_sid,
                           wNameBuffer,&wNameBufSize,
                           wDomainBuffer,&wDomainBufSize,
                           peUse)) {
  DWORD dwError;
  dwError = GetLastError();
  DBG_ALIGNMENT_ENABLE();
  if (dwError == ERROR_BUFFER_OVERFLOW ||
      dwError == ERROR_INSUFFICIENT_BUFFER ||
      dwError == ERROR_MORE_DATA) {
   wNewBuffer = DeeString_ResizeWideBuffer(wNameBuffer,wNameBufSize - 1);
   if unlikely(!wNewBuffer) goto err_domain_buffer;
   wNameBuffer = wNewBuffer;
   wNewBuffer = DeeString_ResizeWideBuffer(wDomainBuffer,wDomainBufSize - 1);
   if unlikely(!wNewBuffer) goto err_domain_buffer;
   wDomainBuffer = wNewBuffer;
  } else {

   goto err_domain_buffer;
  }
  DBG_ALIGNMENT_DISABLE();
 }
 DBG_ALIGNMENT_ENABLE();
 if (wNameBuffer[wNameBufSize])
     wNameBufSize = wcsnlen(wNameBuffer,wNameBufSize);
 if (wDomainBuffer[wDomainBufSize])
     wDomainBufSize = wcsnlen(wDomainBuffer,wDomainBufSize);
 wNewBuffer = DeeString_TryResizeWideBuffer(wNameBuffer,wNameBufSize);
 if likely(wNewBuffer) wNameBuffer = wNewBuffer;
 wNewBuffer = DeeString_TryResizeWideBuffer(wDomainBuffer,wDomainBufSize);
 if likely(wNewBuffer) wDomainBuffer = wNewBuffer;
 *pname   = wNameBuffer;
 *pdomain = wDomainBuffer;
 return 0;
err_domain_buffer:
 DeeString_FreeWideBuffer(wDomainBuffer);
err_name_buffer:
 DeeString_FreeWideBuffer(wNameBuffer);
err:
 return -1;
}


PRIVATE DREF DeeObject *DCALL
user_get_name(struct user_object *__restrict self) {
 LPWSTR name,domain; SID_NAME_USE use;
 if (!self->u_sd) return fs_getuser(false);
 if (user_get_name_and_domain(self,&name,&domain,&use))
     return NULL;
 DeeString_FreeWideBuffer(domain);
 return DeeString_PackWideBuffer(name,STRING_ERROR_FIGNORE);
}
PRIVATE DREF DeeObject *DCALL
user_get_domain(struct user_object *__restrict self) {
 LPWSTR name,domain; SID_NAME_USE use;
 if (!self->u_sd) return fs_gethostname();
 if (user_get_name_and_domain(self,&name,&domain,&use))
     return NULL;
 DeeString_FreeWideBuffer(name);
 return DeeString_PackWideBuffer(domain,STRING_ERROR_FIGNORE);
}
PRIVATE DREF DeeObject *DCALL
user_get_home(struct user_object *__restrict self) {
 LPWSTR name,domain; SID_NAME_USE use;
 if (!self->u_sd) return fs_gethome(false);
 {
  dssize_t error;
  struct unicode_printer printer = UNICODE_PRINTER_INIT;
  if (nt_print_GetProfilesDirectory(&printer,false))
      goto err;
  if (UNICODE_PRINTER_LENGTH(&printer) &&
      UNICODE_PRINTER_GETCHAR(&printer,UNICODE_PRINTER_LENGTH(&printer) - 1) != '\\' &&
      unicode_printer_putascii(&printer,'\\'))
      goto err;
  if (user_get_name_and_domain(self,&name,&domain,&use))
      goto err;
  DeeString_FreeWideBuffer(domain);
  error = unicode_printer_printwide(&printer,name,WSTR_LENGTH(name));
  DeeString_FreeWideBuffer(name);
  if unlikely(error < 0) goto err;
  return unicode_printer_pack(&printer);
err:
  unicode_printer_fini(&printer);
  return NULL;
 }
}

PRIVATE struct type_getset user_getsets[] = {
    { "name", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&user_get_name, NULL, NULL, DeeUser_name_doc },
    { "home", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&user_get_home, NULL, NULL, DeeUser_home_doc },
    { "domain_np", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&user_get_domain, NULL, NULL,
      DOC("->string\n"
          "@throw SystemError Failed to retrieve the domain\n"
          "Returns the windows-specific name of the domain associated with @this user") },
    { NULL }
};

INTERN DeeTypeObject DeeUser_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"user",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FNORMAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeObject_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */NULL,
                /* .tp_copy_ctor = */NULL,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */(void *)&user_init,
                /* .tp_free      = */NULL,
                {
                    /* .tp_instance_size = */sizeof(DeeUserObject)
                }
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&user_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */NULL,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */NULL,
    /* .tp_seq           = */NULL,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */user_getsets,
    /* .tp_members       = */NULL,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */user_class_getsets,
    /* .tp_class_members = */NULL
};

PRIVATE DREF DeeObject *DCALL
nt_NewUserDescriptor(/*inherit(on_success)*/PSID pSid,
                     /*inherit(on_success)*/PSECURITY_DESCRIPTOR pSD) {
 DREF struct user_object *result;
 result = DeeObject_MALLOC(struct user_object);
 if unlikely(!result) goto done;
 result->u_sd = pSD;
 result->u_sid = pSid;
 DeeObject_Init(result,&DeeUser_Type);
done:
 return (DREF DeeObject *)result;
}

PRIVATE DREF DeeObject *DCALL
nt_NewUserDescriptorFromHandleOwner(HANDLE hHandle, SE_OBJECT_TYPE ObjectType) {
 PSID pSidOwner; DWORD dwError;
 PSECURITY_DESCRIPTOR pSD;
 DREF DeeObject *result;
 DBG_ALIGNMENT_DISABLE();
 dwError = GetSecurityInfo(hHandle,ObjectType,
                           OWNER_SECURITY_INFORMATION,
                          &pSidOwner,NULL,NULL,NULL,&pSD);
 if (dwError != ERROR_SUCCESS) {
  dwError = GetLastError();
  DBG_ALIGNMENT_ENABLE();
  DeeError_SysThrowf(&DeeError_SystemError,dwError,
                     "Failed to query owner SID of %lu-typed handle %p",
                    (unsigned long)ObjectType,(void *)hHandle);
  goto err;
 }
 DBG_ALIGNMENT_ENABLE();
 result = nt_NewUserDescriptor(pSidOwner,pSD);
 if likely(result) return result;
 LocalFree(pSD);
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
nt_NewUserDescriptorFromHandleGroup(HANDLE hHandle, SE_OBJECT_TYPE ObjectType) {
 PSID pSidGroup; DWORD dwError;
 PSECURITY_DESCRIPTOR pSD;
 DREF DeeObject *result;
 DBG_ALIGNMENT_DISABLE();
 dwError = GetSecurityInfo(hHandle,ObjectType,
                           GROUP_SECURITY_INFORMATION,
                           NULL,&pSidGroup,NULL,NULL,&pSD);
 if (dwError != ERROR_SUCCESS) {
  dwError = GetLastError();
  DBG_ALIGNMENT_ENABLE();
  DeeError_SysThrowf(&DeeError_SystemError,dwError,
                     "Failed to query group SID of %lu-typed handle %p",
                    (unsigned long)ObjectType,(void *)hHandle);
  goto err;
 }
 DBG_ALIGNMENT_ENABLE();
 result = nt_NewUserDescriptor(pSidGroup,pSD);
 if likely(result) return result;
 LocalFree(pSD);
err:
 return NULL;
}







typedef struct {
    BY_HANDLE_FILE_INFORMATION s_info;   /* Windows-specific stat information. */
    DWORD                      s_ftype;  /* One of `FILE_TYPE_*' or `FILE_TYPE_UNKNOWN' when not determined. */
#define STAT_FNORMAL           0x0000    /* Normal information. */
#define STAT_FNOTIME           0x0001    /* Time stamps are unknown. */
#define STAT_FNOVOLSERIAL      0x0002    /* `dwVolumeSerialNumber' is unknown. */
#define STAT_FNOSIZE           0x0004    /* `nFileSize' is unknown. */
#define STAT_FNONLINK          0x0008    /* `nNumberOfLinks' is unknown. */
#define STAT_FNOFILEID         0x0010    /* `nFileIndex' is unknown. */
#define STAT_FNONTTYPE         0x0020    /* `s_ftype' is unknown. */
    uint16_t                   s_valid;  /* Set of `STAT_F*' */
    uint16_t                   s_pad[1]; /* ... */
    HANDLE                     s_hand;   /* [0..1|NULL(INVALID_HANDLE_VALUE)]
                                          * Optional handle that may be used to load
                                          * additional information upon request. */
} Stat;
#define Stat_Fini(x) ((x)->s_hand == INVALID_HANDLE_VALUE || (CloseHandle((x)->s_hand),0))


/* STAT implementation. */
struct stat_object {
    OBJECT_HEAD
    Stat   st_stat;
};


/* Missing stat information errors. */
PRIVATE ATTR_NOINLINE ATTR_COLD int
DCALL err_no_info(char const *__restrict level) {
 return DeeError_Throwf(&DeeError_ValueError,
                        "The stat object does not contain any %s information",
                        level);
}
PRIVATE ATTR_NOINLINE ATTR_COLD int DCALL err_no_dev_info(void) { return err_no_info("device"); }
PRIVATE ATTR_NOINLINE ATTR_COLD int DCALL err_no_ino_info(void) { return err_no_info("inode"); }
PRIVATE ATTR_NOINLINE ATTR_COLD int DCALL err_no_link_info(void) { return err_no_info("nlink"); }
PRIVATE ATTR_NOINLINE ATTR_COLD int DCALL err_no_uid_info(void) { return err_no_info("uid"); }
PRIVATE ATTR_NOINLINE ATTR_COLD int DCALL err_no_gid_info(void) { return err_no_info("gid"); }
PRIVATE ATTR_NOINLINE ATTR_COLD int DCALL err_no_size_info(void) { return err_no_info("size"); }
PRIVATE ATTR_NOINLINE ATTR_COLD int DCALL err_no_time_info(void) { return err_no_info("time"); }
PRIVATE ATTR_NOINLINE ATTR_COLD int DCALL err_no_nttype_info(void) { return err_no_info("NT Type"); }

#define DOSTAT_FNORMAL   0x00
#define DOSTAT_FTRY      0x01
#define DOSTAT_FLSTAT    0x02
#define DOSTAT_FNOEXINFO 0x04


/* @return:  1: `try_stat' was true and the given `path' could not be found.
 * @return:  0: Successfully did a stat() in the given `path'.
 * @return: -1: The state failed and an error was thrown.
 * @param: flags: Set of `DOSTAT_F*' */
PRIVATE int DCALL
Stat_Init(Stat *__restrict self,
          DeeObject *__restrict path,
          uint16_t flags) {
 HANDLE fd; int error;
again:
 if (DeeThread_CheckInterrupt()) goto err;
 self->s_hand  = INVALID_HANDLE_VALUE; /* If inherited, set later. */
 self->s_ftype = FILE_TYPE_UNKNOWN; /* Lazily initialized. */
 if (DeeString_Check(path)) {
  int error; WIN32_FILE_ATTRIBUTE_DATA attrib;
  fd = nt_CreateFile(path,FILE_READ_ATTRIBUTES|READ_CONTROL,
                     FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,NULL,OPEN_EXISTING,
                    (flags&DOSTAT_FLSTAT ? /* In lstat()-mode, open a reparse point.
                                            * NOTE: If the file isn't a reparse
                                            *       point, the flag is ignored ;) */
                    (FILE_ATTRIBUTE_NORMAL|FILE_FLAG_BACKUP_SEMANTICS|FILE_FLAG_OPEN_REPARSE_POINT) :
                    (FILE_ATTRIBUTE_NORMAL|FILE_FLAG_BACKUP_SEMANTICS)),NULL);
  if (fd == INVALID_HANDLE_VALUE) {
   /* Try again, but leave out READ_CONTROL access permissions.
    * NOTE: `READ_CONTROL' is normally required by the `st_uid' / `st_gid' fields. */
   fd = nt_CreateFile(path,FILE_READ_ATTRIBUTES,
                      FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,NULL,OPEN_EXISTING,
                     (flags&DOSTAT_FLSTAT ? /* In lstat()-mode, open a reparse point.
                                             * NOTE: If the file isn't a reparse
                                             *       point, the flag is ignored ;) */
                     (FILE_ATTRIBUTE_NORMAL|FILE_FLAG_BACKUP_SEMANTICS|FILE_FLAG_OPEN_REPARSE_POINT) :
                     (FILE_ATTRIBUTE_NORMAL|FILE_FLAG_BACKUP_SEMANTICS)),NULL);
  }

  if (fd == NULL) goto err;
  if (fd != INVALID_HANDLE_VALUE) {
   BOOL error;
   DBG_ALIGNMENT_DISABLE();
   error = GetFileInformationByHandle(fd,&self->s_info);
   if (!error) {
    CloseHandle(fd);
    DBG_ALIGNMENT_ENABLE();
    goto err_nt;
   }
   DBG_ALIGNMENT_ENABLE();
   self->s_valid = STAT_FNORMAL;
   self->s_hand  = fd; /* Inherit */
done:
   return 0;
  }
  /* Failed to open the file as a reparse point.
   * All the fallback code here only works for
   * regular stat, so we can't use it... */
#if 0
  if (flags&DOSTAT_FLSTAT) goto err_nt;
#endif

  /* CreateFile() failed. - Try a more direct approach. */
  memset(&self->s_info,0,sizeof(BY_HANDLE_FILE_INFORMATION));
  DBG_ALIGNMENT_DISABLE();
  error = nt_GetFileAttributesEx(path,GetFileExInfoStandard,&attrib);
  DBG_ALIGNMENT_ENABLE();
  if unlikely(error < 0) goto err;
  if (!error) {
   /* It worked! */
   self->s_info.dwFileAttributes = attrib.dwFileAttributes;
   self->s_info.ftCreationTime   = attrib.ftCreationTime;
   self->s_info.ftLastAccessTime = attrib.ftLastAccessTime;
   self->s_info.ftLastWriteTime  = attrib.ftLastWriteTime;
   self->s_info.nFileSizeHigh    = attrib.nFileSizeHigh;
   self->s_info.nFileSizeLow     = attrib.nFileSizeLow;
   self->s_valid = (STAT_FNOVOLSERIAL|STAT_FNONLINK|STAT_FNOFILEID);
   goto done;
  }
  /* Nope. Still nothing...
   * Try this one last thing. */
  error = nt_GetFileAttributes(path,&self->s_info.dwFileAttributes);
  if unlikely(error < 0) goto err;
  if unlikely(error) goto err_nt;
  self->s_valid = (STAT_FNOTIME|STAT_FNOVOLSERIAL|STAT_FNOSIZE|
                   STAT_FNONLINK|STAT_FNOFILEID);
  goto done;
 }
 if (!DeeInt_Check(path)) {
  fd = DeeFile_Fileno(path);
  if (fd == INVALID_HANDLE_VALUE) {
   if (DeeError_Catch(&DeeError_AttributeError) ||
       DeeError_Catch(&DeeError_NotImplemented))
       goto try_filename;
   goto err;
  }
  /* Retrieve information by handle. */
  DBG_ALIGNMENT_DISABLE();
  if (GetFileInformationByHandle(fd,&self->s_info))
      goto ok_user_fd;
  DBG_ALIGNMENT_ENABLE();
  /* Didn't work... (Try the filename) */
try_filename:
  path = DeeFile_Filename(path);
  if unlikely(!path) goto err;
  error = Stat_Init(self,path,flags);
  Dee_Decref(path);
  return error;
 }
 if (DeeObject_AsUIntptr(path,(uintptr_t *)&fd))
     goto err;
 /* Retrieve information by handle. */
 DBG_ALIGNMENT_DISABLE();
 if (GetFileInformationByHandle(fd,&self->s_info)) {
ok_user_fd:
  DBG_ALIGNMENT_ENABLE();
  /* Immediately load the file type if the descriptor was given by the user. */
  if (flags&DOSTAT_FNOEXINFO) {
   self->s_valid |= STAT_FNONTTYPE;
  } else {
   HANDLE hCurrentProcess;
   DBG_ALIGNMENT_DISABLE();
   hCurrentProcess = GetCurrentProcess();
   DBG_ALIGNMENT_ENABLE();
   /* Duplicate the user-given handle, so we can use it to lookup additional information later. */
   if (!DuplicateHandle(hCurrentProcess,fd,
                        hCurrentProcess,&self->s_hand,
                        0,TRUE,DUPLICATE_SAME_ACCESS)) {
    self->s_hand = INVALID_HANDLE_VALUE;
    DBG_ALIGNMENT_DISABLE();
    self->s_ftype = GetFileType(fd);
    DBG_ALIGNMENT_ENABLE();
    self->s_valid = STAT_FNORMAL;
    if unlikely(self->s_ftype == FILE_TYPE_UNKNOWN)
       self->s_valid |= STAT_FNONTTYPE;
   }
  }
  goto done;
 }
err_nt:
 DBG_ALIGNMENT_DISABLE();
 error = (int)GetLastError();
 DBG_ALIGNMENT_ENABLE();
 if ((flags&DOSTAT_FTRY) && nt_IsFileNotFound((DWORD)error))
      return 1; /* File not found. */
 if (nt_IsBadAlloc((DWORD)error)) {
  if (Dee_CollectMemory(1))
      goto again;
 } else if (nt_IsNotDir((DWORD)error)) {
  if (flags&DOSTAT_FTRY) return 1;
  err_path_no_dir((DWORD)error,path);
 } else if (nt_IsAccessDenied((DWORD)error)) {
  err_path_no_access((DWORD)error,path);
 } else if (nt_IsFileNotFound((DWORD)error)) {
  if (flags&DOSTAT_FTRY) return 1;
  err_path_not_found((DWORD)error,path);
 } else if (nt_IsBadf((DWORD)error)) {
  err_handle_closed((DWORD)error,path);
 } else {
  DeeError_SysThrowf(&DeeError_FSError,(DWORD)error,
                     "Failed to open file %r",
                     path);
 }
err:
 return -1;
}

/* Returns one of `FILE_TIME_*' describing the type of the given file.
 * When the type cannot be determined, return FILE_TIME_UNKNOWN when
 * `try_get' is true, or throw an error when it is false. */
PRIVATE DWORD DCALL
stat_get_nttype(Stat *__restrict self, bool try_get) {
 DWORD new_type,result = self->s_ftype;
 if (result == FILE_TYPE_UNKNOWN) {
  if (self->s_valid&STAT_FNONTTYPE)
      goto err_noinfo;
  if (self->s_hand == INVALID_HANDLE_VALUE) {
   ATOMIC_FETCHOR(self->s_valid,STAT_FNONTTYPE);
   goto err_noinfo;
  }
  if (DeeThread_CheckInterrupt()) goto err;
  DBG_ALIGNMENT_DISABLE();
  result = GetFileType(self->s_hand);
  DBG_ALIGNMENT_ENABLE();
  if unlikely(result == FILE_TYPE_UNKNOWN)
     goto err_noinfo;
  new_type = ATOMIC_CMPXCH_VAL(self->s_ftype,FILE_TYPE_UNKNOWN,result);
  if (new_type != FILE_TYPE_UNKNOWN) result = new_type;
 }
 return result;
err_noinfo:
 if (!try_get) err_no_nttype_info();
err:
 return FILE_TYPE_UNKNOWN;
}




PRIVATE int DCALL
stat_ctor(DeeStatObject *__restrict self,
          size_t argc, DeeObject **__restrict argv) {
 DeeObject *path;
 if (DeeArg_Unpack(argc,argv,"o:stat",&path))
     return -1;
 return Stat_Init(&self->st_stat,path,DOSTAT_FNORMAL);
}
PRIVATE int DCALL
lstat_ctor(DeeStatObject *__restrict self,
           size_t argc, DeeObject **__restrict argv) {
 DeeObject *path;
 if (DeeArg_Unpack(argc,argv,"o:lstat",&path))
     return -1;
 return Stat_Init(&self->st_stat,path,DOSTAT_FLSTAT);
}
PRIVATE void DCALL
stat_fini(DeeStatObject *__restrict self) {
 Stat_Fini(&self->st_stat);
}

#ifdef CONFIG_BIG_ENDIAN
#define FILETIME_GET64(x) (((x) << 32)|((x) >> 32))
#else
#define FILETIME_GET64(x)   (x)
#endif
#define FILETIME_PER_SECONDS 10000000 /* 100 nanoseconds / 0.1 microseconds. */

PRIVATE DREF DeeObject *DCALL
DeeTime_NewFiletime(FILETIME const *__restrict val) {
 uint64_t result;
 result  = (FILETIME_GET64(*(uint64_t *)val)/(FILETIME_PER_SECONDS/MICROSECONDS_PER_SECOND));
 result += time_yer2day(1601)*MICROSECONDS_PER_DAY;
 return DeeTime_New(result);
}


PRIVATE DREF DeeObject *DCALL
stat_get_dev(DeeStatObject *__restrict self) {
 if unlikely(self->st_stat.s_valid&STAT_FNOVOLSERIAL) { err_no_dev_info(); return NULL; }
 return DeeInt_NewU32((uint32_t)self->st_stat.s_info.dwVolumeSerialNumber);
}
PRIVATE DREF DeeObject *DCALL
stat_get_ino(DeeStatObject *__restrict self) {
 if unlikely(self->st_stat.s_valid&STAT_FNOFILEID) { err_no_ino_info(); return NULL; }
 return DeeInt_NewU64(((uint64_t)self->st_stat.s_info.nFileIndexHigh << 32) |
                      ((uint64_t)self->st_stat.s_info.nFileIndexLow));
}

PRIVATE DREF DeeObject *DCALL
stat_get_mode(DeeStatObject *__restrict self) {
 uint32_t result = 0222|0111; /* XXX: executable should depend on extension. */
 if (self->st_stat.s_info.dwFileAttributes&FILE_ATTRIBUTE_READONLY)
     result |= 0444;
 switch (stat_get_nttype(&self->st_stat,true)) {
 case FILE_TYPE_CHAR:   result |= STAT_IFCHR; break;
 case FILE_TYPE_PIPE:   result |= STAT_IFIFO; break;
 case FILE_TYPE_REMOTE: result |= STAT_IFSOCK; break;
 case FILE_TYPE_DISK:
  /* Actually means a file on-disk when the device flag isn't set. */
  if (self->st_stat.s_info.dwFileAttributes&FILE_ATTRIBUTE_DEVICE) {
   result |= STAT_IFBLK;
   break;
  }
  ATTR_FALLTHROUGH
 default:
  if (self->st_stat.s_info.dwFileAttributes &
      FILE_ATTRIBUTE_DIRECTORY)
   result |= STAT_IFDIR;
  else if (self->st_stat.s_info.dwFileAttributes &
           FILE_ATTRIBUTE_REPARSE_POINT)
   result |= STAT_IFLNK;
  else
   result |= STAT_IFREG;
  break;
 }
 return DeeInt_NewU32(result);
}
PRIVATE DREF DeeObject *DCALL
stat_get_nlink(DeeStatObject *__restrict self) {
 if unlikely(self->st_stat.s_valid&STAT_FNONLINK) { err_no_link_info(); return NULL; }
 return DeeInt_NewU32(self->st_stat.s_info.nNumberOfLinks);
}
PRIVATE DREF DeeObject *DCALL
stat_get_uid(DeeStatObject *__restrict self) {
 if unlikely(self->st_stat.s_hand == INVALID_HANDLE_VALUE) { err_no_uid_info(); return NULL; }
 return nt_NewUserDescriptorFromHandleOwner(self->st_stat.s_hand,SE_FILE_OBJECT);
}
PRIVATE DREF DeeObject *DCALL
stat_get_gid(DeeStatObject *__restrict self) {
 if unlikely(self->st_stat.s_hand == INVALID_HANDLE_VALUE) { err_no_gid_info(); return NULL; }
 return nt_NewUserDescriptorFromHandleGroup(self->st_stat.s_hand,SE_FILE_OBJECT);
}
PRIVATE DREF DeeObject *DCALL
stat_get_rdev(DeeStatObject *__restrict UNUSED(self)) {
 err_no_info("rdev");
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
stat_get_size(DeeStatObject *__restrict self) {
 if unlikely(self->st_stat.s_valid&STAT_FNOSIZE) { err_no_size_info(); return NULL; }
 return DeeInt_NewU64(((uint64_t)self->st_stat.s_info.nFileSizeHigh << 32) |
                      ((uint64_t)self->st_stat.s_info.nFileSizeLow));
}
PRIVATE DREF DeeObject *DCALL
stat_get_atime(DeeStatObject *__restrict self) {
 if unlikely(self->st_stat.s_valid&STAT_FNOTIME) { err_no_time_info(); return NULL; }
 return DeeTime_NewFiletime(&self->st_stat.s_info.ftLastAccessTime);
}
PRIVATE DREF DeeObject *DCALL
stat_get_mtime(DeeStatObject *__restrict self) {
 if unlikely(self->st_stat.s_valid&STAT_FNOTIME) { err_no_time_info(); return NULL; }
 return DeeTime_NewFiletime(&self->st_stat.s_info.ftLastWriteTime);
}
PRIVATE DREF DeeObject *DCALL
stat_get_ctime(DeeStatObject *__restrict self) {
 if unlikely(self->st_stat.s_valid&STAT_FNOTIME) { err_no_time_info(); return NULL; }
 return DeeTime_NewFiletime(&self->st_stat.s_info.ftCreationTime);
}

PRIVATE DREF DeeObject *DCALL
stat_getntattr_np(DeeStatObject *__restrict self) {
 return DeeInt_NewU32(self->st_stat.s_info.dwFileAttributes);
}
PRIVATE DREF DeeObject *DCALL
stat_getnttype_np(DeeStatObject *__restrict self) {
 DWORD result = stat_get_nttype(&self->st_stat,false);
 if unlikely(result == FILE_TYPE_UNKNOWN)
    return NULL;
 return DeeInt_NewU32(result);
}


PRIVATE struct type_getset stat_getsets[] = {
    { "st_dev", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&stat_get_dev, NULL, NULL, DeeStat_st_dev_doc },
    { "st_ino", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&stat_get_ino, NULL, NULL, DeeStat_st_ino_doc },
    { "st_mode", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&stat_get_mode, NULL, NULL, DeeStat_st_mode_doc },
    { "st_nlink", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&stat_get_nlink, NULL, NULL, DeeStat_st_nlink_doc },
    { "st_uid", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&stat_get_uid, NULL, NULL, DeeStat_st_uid_doc },
    { "st_gid", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&stat_get_gid, NULL, NULL, DeeStat_st_gid_doc },
    { "st_rdev", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&stat_get_rdev, NULL, NULL, DeeStat_st_rdev_doc },
    { "st_size", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&stat_get_size, NULL, NULL, DeeStat_st_size_doc },
    { "st_atime", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&stat_get_atime, NULL, NULL, DeeStat_st_atime_doc },
    { "st_mtime", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&stat_get_mtime, NULL, NULL, DeeStat_st_mtime_doc },
    { "st_ctime", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&stat_get_ctime, NULL, NULL, DeeStat_st_ctime_doc },

    /* Non-portable NT extensions. */
    { "ntattr_np", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&stat_getntattr_np, NULL, NULL,
      DOC("->int\n"
          "Non-portable windows extension for retrieveing the NT attributes of the stat-file, those "
          "attributes being a set of the `FILE_ATTRIBUTE_*' constants found in windows system headers") },
    { "nttype_np", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&stat_getnttype_np, NULL, NULL,
      DOC("->int\n"
          "@throw ValueError @this stat-file does not contain valid NT-type information\n"
          "Non-portable windows extension for retrieveing the NT type of this stat-file, that "
          "type being one of the `FILE_TYPE_*' constants found in windows system headers") },
    { NULL }
};

PRIVATE DREF DeeObject *DCALL
stat_isdir(DeeStatObject *__restrict self,
           size_t argc, DeeObject **__restrict argv) {
 if (DeeArg_Unpack(argc,argv,":isdir"))
     return NULL;
 if (self->st_stat.s_info.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
     return_true;
 return_false;
}

PRIVATE DREF DeeObject *DCALL
stat_ischr(DeeStatObject *__restrict self,
           size_t argc, DeeObject **__restrict argv) {
 if (DeeArg_Unpack(argc,argv,":ischr"))
     return NULL;
 if (!(self->st_stat.s_info.dwFileAttributes&FILE_ATTRIBUTE_DEVICE))
       return_false;
 return_bool(stat_get_nttype(&self->st_stat,true) == FILE_TYPE_CHAR);
}

PRIVATE DREF DeeObject *DCALL
stat_isblk(DeeStatObject *__restrict self,
           size_t argc, DeeObject **__restrict argv) {
 if (DeeArg_Unpack(argc,argv,":isblk"))
     return NULL;
 if (!(self->st_stat.s_info.dwFileAttributes&FILE_ATTRIBUTE_DEVICE))
       return_false;
 return_bool(stat_get_nttype(&self->st_stat,true) == FILE_TYPE_DISK);
}

PRIVATE DREF DeeObject *DCALL
stat_isreg(DeeStatObject *__restrict self,
           size_t argc, DeeObject **__restrict argv) {
 if (DeeArg_Unpack(argc,argv,":isreg"))
     return NULL;
 return_bool(!(self->st_stat.s_info.dwFileAttributes&
              (FILE_ATTRIBUTE_DEVICE|FILE_ATTRIBUTE_DIRECTORY|
               FILE_ATTRIBUTE_REPARSE_POINT)));
}

PRIVATE DREF DeeObject *DCALL
stat_isfifo(DeeStatObject *__restrict self,
            size_t argc, DeeObject **__restrict argv) {
 if (DeeArg_Unpack(argc,argv,":isfifo"))
     return NULL;
 return_bool(stat_get_nttype(&self->st_stat,true) == FILE_TYPE_PIPE);
}

PRIVATE DREF DeeObject *DCALL
stat_islnk(DeeStatObject *__restrict self,
           size_t argc, DeeObject **__restrict argv) {
 if (DeeArg_Unpack(argc,argv,":islnk"))
     return NULL;
 return_bool(stat_get_nttype(&self->st_stat,true) == FILE_TYPE_PIPE);
}

PRIVATE DREF DeeObject *DCALL
stat_issock(DeeStatObject *__restrict self,
            size_t argc, DeeObject **__restrict argv) {
 if (DeeArg_Unpack(argc,argv,":issock"))
     return NULL;
 return_bool(stat_get_nttype(&self->st_stat,true) == FILE_TYPE_REMOTE);
}

PRIVATE struct type_method stat_methods[] = {
    { "isdir", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&stat_isdir, DeeStat_isdir_doc },
    { "ischr", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&stat_ischr, DeeStat_ischr_doc },
    { "isblk", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&stat_isblk, DeeStat_isblk_doc },
    { "isreg", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&stat_isreg, DeeStat_isreg_doc },
    { "isfifo", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&stat_isfifo, DeeStat_isfifo_doc },
    { "islnk", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&stat_islnk, DeeStat_islnk_doc },
    { "issock", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&stat_issock, DeeStat_issock_doc },
    { NULL }
};

PRIVATE DREF DeeObject *DCALL
stat_class_exists(DeeObject *__restrict self,
                  size_t argc, DeeObject **__restrict argv) {
 DeeObject *path; int error; Stat buf;
 if (DeeThread_CheckInterrupt()) goto err;
 if (DeeArg_Unpack(argc,argv,"o:exists",&path))
     goto err;
 if (DeeString_Check(path)) {
  DWORD attr; /* Do a quick attribute query. */
  error = nt_GetFileAttributes(path,&attr);
  if unlikely(error < 0) goto err;
  return_bool(!error);
 }
 error = Stat_Init(&buf,path,self == (DeeObject *)&DeeLStat_Type
                   ? DOSTAT_FTRY|DOSTAT_FNOEXINFO|DOSTAT_FLSTAT
                   : DOSTAT_FTRY|DOSTAT_FNOEXINFO);
 if unlikely(error < 0) goto err;
 if (error > 0) return_false;
 Stat_Fini(&buf);
 return_true;
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
stat_class_isdir(DeeObject *__restrict self,
                 size_t argc, DeeObject **__restrict argv) {
 DeeObject *path; int error; Stat buf;
 if (DeeThread_CheckInterrupt()) goto err;
 if (DeeArg_Unpack(argc,argv,"o:isdir",&path))
     goto err;
 if (DeeString_Check(path)) {
  DWORD attr; /* Do a quick attribute query. */
  error = nt_GetFileAttributes(path,&attr);
  if unlikely(error < 0) goto err;
  if (!error) return_bool(attr & FILE_ATTRIBUTE_DIRECTORY);
 }
 error = Stat_Init(&buf,path,self == (DeeObject *)&DeeLStat_Type
                   ? DOSTAT_FTRY|DOSTAT_FNOEXINFO|DOSTAT_FLSTAT
                   : DOSTAT_FTRY|DOSTAT_FNOEXINFO);
 if unlikely(error < 0) goto err;
 if (error > 0) return_false;
 error = (int)(buf.s_info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
 Stat_Fini(&buf);
 return_bool(error);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
stat_class_ischr(DeeObject *__restrict self,
                 size_t argc, DeeObject **__restrict argv) {
 DeeObject *path; int error; Stat buf;
 if (DeeThread_CheckInterrupt()) goto err;
 if (DeeArg_Unpack(argc,argv,"o:ischr",&path))
     goto err;
 error = Stat_Init(&buf,path,self == (DeeObject *)&DeeLStat_Type
                   ? DOSTAT_FTRY|DOSTAT_FLSTAT
                   : DOSTAT_FTRY);
 if unlikely(error < 0) goto err;
 if (error > 0) return_false;
 if (!(buf.s_info.dwFileAttributes&FILE_ATTRIBUTE_DEVICE))
  error = 0;
 else {
  error = stat_get_nttype(&buf,true) == FILE_TYPE_CHAR;
 }
 Stat_Fini(&buf);
 return_bool(error);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
stat_class_isblk(DeeObject *__restrict self,
                 size_t argc, DeeObject **__restrict argv) {
 DeeObject *path; int error; Stat buf;
 if (DeeThread_CheckInterrupt()) goto err;
 if (DeeArg_Unpack(argc,argv,"o:isblk",&path))
     goto err;
 error = Stat_Init(&buf,path,self == (DeeObject *)&DeeLStat_Type
                   ? DOSTAT_FTRY|DOSTAT_FLSTAT
                   : DOSTAT_FTRY);
 if unlikely(error < 0) goto err;
 if (error > 0) return_false;
 if (!(buf.s_info.dwFileAttributes&FILE_ATTRIBUTE_DEVICE))
  error = 0;
 else {
  error = stat_get_nttype(&buf,true) == FILE_TYPE_DISK;
 }
 Stat_Fini(&buf);
 return_bool(error);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
stat_class_isreg(DeeObject *__restrict self,
                 size_t argc, DeeObject **__restrict argv) {
 DeeObject *path; int error; Stat buf;
 if (DeeThread_CheckInterrupt()) goto err;
 if (DeeArg_Unpack(argc,argv,"o:isreg",&path))
     goto err;
 if (DeeString_Check(path)) {
  DWORD attr; /* Do a quick attribute query. */
  error = nt_GetFileAttributes(path,&attr);
  if unlikely(error < 0) goto err;
  if ((attr&FILE_ATTRIBUTE_REPARSE_POINT) &&
       self != (DeeObject *)&DeeLStat_Type)
       goto do_normal_stat;
  if (!error)
       return_bool(!(attr & (FILE_ATTRIBUTE_DEVICE|
                             FILE_ATTRIBUTE_DIRECTORY|
                             FILE_ATTRIBUTE_REPARSE_POINT)));
 }
do_normal_stat:
 error = Stat_Init(&buf,path,self == (DeeObject *)&DeeLStat_Type
                   ? DOSTAT_FTRY|DOSTAT_FNOEXINFO|DOSTAT_FLSTAT
                   : DOSTAT_FTRY|DOSTAT_FNOEXINFO);
 if unlikely(error < 0) goto err;
 if (error > 0) return_false;
 error = (int)(buf.s_info.dwFileAttributes &
              (FILE_ATTRIBUTE_DEVICE|FILE_ATTRIBUTE_DIRECTORY|
               FILE_ATTRIBUTE_REPARSE_POINT));
 Stat_Fini(&buf);
 return_bool(!error);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
stat_class_isfifo(DeeObject *__restrict self,
                 size_t argc, DeeObject **__restrict argv) {
 DeeObject *path; int error; Stat buf;
 if (DeeThread_CheckInterrupt()) goto err;
 if (DeeArg_Unpack(argc,argv,"o:isfifo",&path))
     goto err;
 error = Stat_Init(&buf,path,self == (DeeObject *)&DeeLStat_Type
                   ? DOSTAT_FTRY|DOSTAT_FLSTAT
                   : DOSTAT_FTRY);
 if unlikely(error < 0) goto err;
 if (error > 0) return_false;
 error = stat_get_nttype(&buf,true) == FILE_TYPE_PIPE;
 Stat_Fini(&buf);
 return_bool(error);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
stat_class_islnk(DeeObject *__restrict UNUSED(self),
                 size_t argc, DeeObject **__restrict argv) {
 DeeObject *path; int error; Stat buf;
 if (DeeThread_CheckInterrupt()) goto err;
 if (DeeArg_Unpack(argc,argv,"o:islnk",&path))
     goto err;
 if (DeeString_Check(path)) {
  DWORD attr; /* Do a quick attribute query. */
  error = nt_GetFileAttributes(path,&attr);
  if unlikely(error < 0) goto err;
  if (!error)
       return_bool(attr & FILE_ATTRIBUTE_REPARSE_POINT);
 }
 error = Stat_Init(&buf,path,DOSTAT_FTRY|DOSTAT_FNOEXINFO|DOSTAT_FLSTAT);
 if unlikely(error < 0) goto err;
 if (error > 0) return_false;
 error = (int)(buf.s_info.dwFileAttributes &
               FILE_ATTRIBUTE_REPARSE_POINT);
 Stat_Fini(&buf);
 return_bool(error);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
stat_class_issock(DeeObject *__restrict self,
                 size_t argc, DeeObject **__restrict argv) {
 DeeObject *path; int error; Stat buf;
 if (DeeThread_CheckInterrupt()) goto err;
 if (DeeArg_Unpack(argc,argv,"o:issock",&path))
     goto err;
 error = Stat_Init(&buf,path,self == (DeeObject *)&DeeLStat_Type
                   ? DOSTAT_FTRY|DOSTAT_FLSTAT
                   : DOSTAT_FTRY);
 if unlikely(error < 0) goto err;
 if (error > 0) return_false;
 error = stat_get_nttype(&buf,true) == FILE_TYPE_REMOTE;
 Stat_Fini(&buf);
 return_bool(error);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
stat_class_ishidden(DeeObject *__restrict self,
                    size_t argc, DeeObject **__restrict argv) {
 DeeObject *path; int error; Stat buf;
 if (DeeThread_CheckInterrupt()) goto err;
 if (DeeArg_Unpack(argc,argv,"o:ishidden",&path))
     goto err;
 if (DeeString_Check(path)) {
  DWORD attr; /* Do a quick attribute query. */
  error = nt_GetFileAttributes(path,&attr);
  if unlikely(error < 0) goto err;
  if ((attr&FILE_ATTRIBUTE_REPARSE_POINT) &&
       self != (DeeObject *)&DeeLStat_Type)
       goto do_normal_stat;
  if (!error)
       return_bool(attr & FILE_ATTRIBUTE_HIDDEN);
 }
do_normal_stat:
 error = Stat_Init(&buf,path,self == (DeeObject *)&DeeLStat_Type
                   ? DOSTAT_FTRY|DOSTAT_FNOEXINFO|DOSTAT_FLSTAT
                   : DOSTAT_FTRY|DOSTAT_FNOEXINFO);
 if unlikely(error < 0) goto err;
 if (error > 0) return_false;
 error = (int)(buf.s_info.dwFileAttributes &
               FILE_ATTRIBUTE_HIDDEN);
 Stat_Fini(&buf);
 return_bool(error);
err:
 return NULL;
}


PRIVATE bool DCALL
is_exe_filename(DeeObject *__restrict path) {
 DREF DeeObject *pathext_ob; bool result;
 char *ext_begin,*ext_end,*pathext; size_t ext_size;
 ext_begin = ext_end = DeeString_END(path);
 for (;;) {
  if (ext_begin == DeeString_STR(path))
      return false;
  if (ext_begin[-1] == '.')
      break;
  --ext_begin;
  if (*ext_begin == '/' ||
      *ext_begin == '\\')
       return false;
 }
 ext_size = (size_t)(ext_end-ext_begin);
 /* Got the file path. */
 pathext_ob = nt_GetEnvironmentVariableA("PATHEXT");
 if likely(pathext_ob)
  pathext = DeeString_STR(pathext_ob);
 else {
  pathext = (char *)".COM;.EXE;.BAT;.CMD";
 }
 result = false;
 while (*pathext) {
  char *next = strchr(pathext,';');
  if (!next) next = strend(pathext);
  /* Check if this is the extension we've been looking for. */
  if (ext_size == (size_t)(next-pathext) &&
      MEMCASEEQ(pathext,ext_begin,ext_size*sizeof(char)))
  { result = true; break; }
  pathext = next;
  if (*pathext) ++pathext; /* Skip `;' */
 }
 Dee_XDecref(pathext_ob);
 return result;
}

PRIVATE DREF DeeObject *DCALL
stat_class_isexe(DeeObject *__restrict UNUSED(self),
                 size_t argc, DeeObject **__restrict argv) {
 DeeObject *path; bool result;
 if (DeeArg_Unpack(argc,argv,"o:isexe",&path))
     goto err;
 if (DeeThread_CheckInterrupt()) goto err;
 if (DeeInt_Check(path))
     return_false;
 if (DeeString_Check(path)) {
  result = is_exe_filename(path);
 } else {
  if (DeeInt_Check(path)) {
   HANDLE fd; /* Support for descriptor-based isexe() */
   if (DeeObject_AsUIntptr(path,(uintptr_t *)&fd))
    goto err;
   path = nt_GetFilenameOfHandle(fd);
  } else {
   path = DeeFile_Filename(path);
  }
  if unlikely(!path) goto err;
  result = is_exe_filename(path);
  Dee_Decref(path);
 }
 return_bool(result);
err:
 return NULL;
}

PRIVATE struct type_method stat_class_methods[] = {
    { "exists", &stat_class_exists, DeeStat_class_exists_doc },
    { "isdir", &stat_class_isdir, DeeStat_class_isdir_doc },
    { "ischr", &stat_class_ischr, DeeStat_class_ischr_doc },
    { "isblk", &stat_class_isblk, DeeStat_class_isblk_doc },
    { "isreg", &stat_class_isreg, DeeStat_class_isreg_doc },
    { "isfifo", &stat_class_isfifo, DeeStat_class_isfifo_doc },
    { "islnk", &stat_class_islnk, DeeStat_class_islnk_doc },
    { "issock", &stat_class_issock, DeeStat_class_issock_doc },
    { "ishidden", &stat_class_ishidden, DeeStat_class_ishidden_doc },
    { "isexe", &stat_class_isexe, DeeStat_class_isexe_doc },
    { NULL }
};

INTERN DeeTypeObject DeeStat_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"stat",
    /* .tp_doc      = */DeeStat_TP_DOC,
    /* .tp_flags    = */TP_FNORMAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeObject_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */NULL,
                /* .tp_copy_ctor = */NULL,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */(void *)&stat_ctor,
                /* .tp_free      = */NULL,
                {
                    /* .tp_instance_size = */sizeof(DeeStatObject)
                }
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&stat_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */NULL,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */NULL,
    /* .tp_seq           = */NULL,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */stat_methods,
    /* .tp_getsets       = */stat_getsets,
    /* .tp_members       = */NULL,
    /* .tp_class_methods = */stat_class_methods,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */NULL
};

INTERN DeeTypeObject DeeLStat_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"lstat",
    /* .tp_doc      = */DeeLStat_TP_DOC,
    /* .tp_flags    = */TP_FNORMAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeStat_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */NULL,
                /* .tp_copy_ctor = */NULL,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */(void *)&lstat_ctor,
                /* .tp_free      = */NULL,
                {
                    /* .tp_instance_size = */sizeof(DeeStatObject)
                }
            }
        },
        /* .tp_dtor        = */NULL,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */NULL,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */NULL,
    /* .tp_seq           = */NULL,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */NULL,
    /* .tp_members       = */NULL,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */NULL
};



/* @return:  0: Successfully written the handle to `phandle'
 * @return:  1: Successfully written the handle to `phandle',
 *              but the caller must CloseHandle() it when they are done
 * @return: -1: An error occurred. */
PRIVATE int DCALL
get_pathhandle_wrattr(DeeObject *__restrict path,
                      HANDLE *__restrict phandle) {
 int result;
 if (DeeString_Check(path)) {
again:
  *phandle = nt_CreateFile(path,FILE_WRITE_ATTRIBUTES,
                           FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
                           NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL|
                           FILE_FLAG_BACKUP_SEMANTICS,NULL);
  if (*phandle == INVALID_HANDLE_VALUE) {
   DWORD dwError;
   DBG_ALIGNMENT_DISABLE();
   dwError = GetLastError();
   DBG_ALIGNMENT_ENABLE();
   if (nt_IsBadAlloc(dwError)) {
    if (Dee_CollectMemory(1))
        goto again;
   } else if (nt_IsNotDir(dwError)) {
    err_path_no_dir(dwError,path);
   } else if (nt_IsFileNotFound(dwError)) {
    err_file_not_found(dwError,path);
   } else if (nt_IsAccessDenied(dwError)) {
    err_file_no_write_access(dwError,path);
   } else {
    DeeError_SysThrowf(&DeeError_FSError,dwError,
                       "Failed to obtain a writable handle for %r",
                       path);
   }
   goto err;
  }
  if (*phandle == NULL) goto err;
  return 1;
 }
 /* Convert an integer to a handle. */
 if (DeeInt_Check(path))
     return DeeObject_AsUIntptr(path,(uintptr_t *)phandle);
 /* Load the file number of a file stream. */
 *phandle = DeeFile_Fileno(path);
 if (*phandle != INVALID_HANDLE_VALUE) return 0;
 if (!DeeError_Catch(&DeeError_AttributeError) &&
     !DeeError_Catch(&DeeError_NotImplemented))
      goto err;
 /* Use the filename of a file stream. */
 path = DeeFile_Filename(path);
 if unlikely(!path) goto err;
 result = get_pathhandle_wrattr(path,phandle);
 Dee_Decref(path);
 return result;
err:
 return -1;
}

PRIVATE int DCALL
ob_GetFileTime(DeeObject *__restrict lpTime,
               FILETIME *__restrict lpFileTime) {
 uint64_t value;
 if (DeeObject_AsUInt64(lpTime,&value))
     goto err;
 if (value < time_yer2day(1601)*MICROSECONDS_PER_DAY) {
  DeeError_Throwf(&DeeError_ValueError,
                  "Invalid file timestamp %k (must be located after 01.01.1601)",
                  lpTime);
  goto err;
 }
 value -= time_yer2day(1601)*MICROSECONDS_PER_DAY;
 UNALIGNED_SET64((uint64_t *)lpFileTime,
                  FILETIME_GET64(value *
                                (FILETIME_PER_SECONDS /
                                 MICROSECONDS_PER_SECOND)));
 return 0;
err:
 return -1;
}

/* Filesystem write operations. */
INTERN int DCALL
fs_chtime(DeeObject *__restrict path, DeeObject *__restrict atime,
          DeeObject *__restrict mtime, DeeObject *__restrict ctime) {
 int result; HANDLE hnd; BOOL error;
 FILETIME ftAtime,ftMtime,ftCtime;
 if (DeeThread_CheckInterrupt()) goto err;
 result = get_pathhandle_wrattr(path,&hnd);
 if unlikely(result < 0) goto err;
 if (!DeeNone_Check(atime) && unlikely(ob_GetFileTime(atime,&ftAtime))) goto err;
 if (!DeeNone_Check(mtime) && unlikely(ob_GetFileTime(mtime,&ftMtime))) goto err;
 if (!DeeNone_Check(ctime) && unlikely(ob_GetFileTime(ctime,&ftCtime))) goto err;
again:
 DBG_ALIGNMENT_DISABLE();
 error = SetFileTime(hnd,
                     DeeNone_Check(ctime) ? NULL : &ftCtime,
                     DeeNone_Check(atime) ? NULL : &ftAtime,
                     DeeNone_Check(mtime) ? NULL : &ftMtime);
 if (result) CloseHandle(hnd);
 if unlikely(!error) {
  DWORD dwError;
  dwError = GetLastError();
  DBG_ALIGNMENT_ENABLE();
  if (nt_IsBadAlloc(dwError)) {
   if (Dee_CollectMemory(1))
       goto again;
  } else if (nt_IsBadf(dwError)) {
   err_handle_closed(dwError,path);
  } else if (nt_IsAccessDenied(dwError)) {
   err_chtime_no_access(dwError,path);
  } else if (nt_IsUnsupported(dwError)) {
   DeeError_SysThrowf(&DeeError_UnsupportedAPI,(DWORD)error,
                      "The filesystem hosting the path %r does "
                      "not support the changing of time stamps",
                      path);
  } else {
   DeeError_SysThrowf(&DeeError_FSError,dwError,
                      "Failed to change time properties of %r",
                      path);
  }
  goto err;
 }
 DBG_ALIGNMENT_ENABLE();
 return 0;
err:
 return -1;
}

INTERN int DCALL
fs_chmod(DeeObject *__restrict path,
         DeeObject *__restrict mode) {
 uint16_t mask,flags;
 DWORD old_flags,new_flags; int error;
 if (DeeThread_CheckInterrupt()) goto err;
 if (!DeeString_Check(path)) {
  if (DeeInt_Check(path)) {
   HANDLE fd; /* Support for descriptor-based chmod() */
   if (DeeObject_AsUIntptr(path,(uintptr_t *)&fd))
    goto err;
   path = nt_GetFilenameOfHandle(fd);
  } else {
   path = DeeFile_Filename(path);
  }
  if unlikely(!path) goto err;
  error = fs_chmod(path,mode);
  Dee_Decref(path);
  return error;
 }
 if (fs_getchmod_mask(mode,&mask,&flags))
     goto err;
again:
 error = nt_GetFileAttributes(path,&old_flags);
 if unlikely(error < 0) goto err;
 if unlikely(error) goto err_nt;
 new_flags = old_flags & ~FILE_ATTRIBUTE_READONLY;
 if (mask&0222) /* Inherit old writability mode. */
     new_flags |= old_flags&FILE_ATTRIBUTE_READONLY;
 if (flags&0222) {
  new_flags &= ~FILE_ATTRIBUTE_READONLY; /* Make writable. */
 } else {
  new_flags |= FILE_ATTRIBUTE_READONLY; /* Make readonly. */
 }
 if (new_flags != old_flags) {
  /* Set new flags. */
  error = nt_SetFileAttributes(path,new_flags);
  if unlikely(error < 0) goto err;
  if unlikely(error) goto err_nt;
 }
 return 0;
err_nt:
 DBG_ALIGNMENT_DISABLE();
 error = (int)GetLastError();
 DBG_ALIGNMENT_ENABLE();
 if (nt_IsBadAlloc((DWORD)error)) {
  if (Dee_CollectMemory(1))
      goto again;
 } else if (nt_IsAccessDenied((DWORD)error)) {
  err_chattr_no_access((DWORD)error,path);
 } else if (nt_IsUnsupported((DWORD)error)) {
  DeeError_SysThrowf(&DeeError_UnsupportedAPI,(DWORD)error,
                     "The filesystem hosting the path %r does "
                     "not support the changing of NT attributes",
                     path);
 } else {
  DeeError_SysThrowf(&DeeError_FSError,(DWORD)error,
                     "Failed to change attributes of %r",
                     path);
 }
err:
 return -1;
}
INTERN int DCALL
fs_chattr_np(DeeObject *__restrict path,
             DeeObject *__restrict new_attr) {
 DWORD attr; int error;
 if (!DeeString_Check(path)) {
  if (DeeInt_Check(path)) {
   HANDLE fd; /* Support for descriptor-based chmod() */
   if (DeeObject_AsUIntptr(path,(uintptr_t *)&fd))
    goto err;
   path = nt_GetFilenameOfHandle(fd);
  } else {
   path = DeeFile_Filename(path);
  }
  if unlikely(!path) goto err;
  error = fs_chattr_np(path,new_attr);
  Dee_Decref(path);
  return error;
 }
 if (DeeThread_CheckInterrupt()) goto err;
 if (DeeObject_AsUInt32(new_attr,(uint32_t *)&attr))
     goto err;
again:
 error = nt_SetFileAttributes(path,attr);
 if (error <= 0) return error;
 DBG_ALIGNMENT_DISABLE();
 error = (int)GetLastError();
 DBG_ALIGNMENT_ENABLE();
 if (nt_IsBadAlloc((DWORD)error)) {
  if (Dee_CollectMemory(1))
      goto again;
 } else if (nt_IsAccessDenied((DWORD)error)) {
  err_chattr_no_access((DWORD)error,path);
 } else if (nt_IsUnsupported((DWORD)error)) {
  DeeError_SysThrowf(&DeeError_UnsupportedAPI,(DWORD)error,
                     "The filesystem hosting the path %r does "
                     "not support the changing of NT attributes",
                     path);
 } else {
  DeeError_SysThrowf(&DeeError_FSError,(DWORD)error,
                     "Failed to change attributes of %r",
                     path);
 }
err:
 return -1;
}

INTERN int DCALL
fs_chown(DeeObject *__restrict path,
         DeeObject *__restrict user,
         DeeObject *__restrict group) {
 if (DeeThread_CheckInterrupt()) goto err;
 (void)path,(void)user,(void)group; /* TODO */
 DERROR_NOTIMPLEMENTED();
err:
 return -1;
}

INTERN int DCALL
fs_mkdir(DeeObject *__restrict path,
         DeeObject *__restrict perm) {
 int error;
 if (DeeThread_CheckInterrupt()) goto err;
 if (DeeObject_AssertTypeExact(path,&DeeString_Type))
     goto err;
again:
 if (!DeeNone_Check(perm)) {
  /* TODO: Initial security attributes. */
  error = nt_CreateDirectory(path,NULL);
 } else {
  error = nt_CreateDirectory(path,NULL);
 }
 if unlikely(error > 0) goto err_nt;
 return error;
err_nt:
 DBG_ALIGNMENT_DISABLE();
 error = (int)GetLastError();
 DBG_ALIGNMENT_ENABLE();
 if (nt_IsBadAlloc((DWORD)error)) {
  if (Dee_CollectMemory(1))
      goto again;
 } else if (nt_IsAccessDenied((DWORD)error)) {
  err_path_no_write_access((DWORD)error,path);
 } else if (nt_IsExists((DWORD)error)) {
  err_path_exists((DWORD)error,path);
 } else if (nt_IsNotDir((DWORD)error)) {
  err_path_no_dir((DWORD)error,path);
 } else if (nt_IsUnsupported((DWORD)error)) {
  DeeError_SysThrowf(&DeeError_UnsupportedAPI,(DWORD)error,
                     "The filesystem hosting the path %r does "
                     "not support the creation of directories",
                     path);
 } else {
  DeeError_SysThrowf(&DeeError_FSError,(DWORD)error,
                     "Failed to create directory %r",
                     path);
 }
err:
 return -1;
}

PRIVATE ATTR_COLD int DCALL
handle_rmdir_error(DWORD error, DeeObject *__restrict path) {
 if (nt_IsNotEmpty(error))
     return err_path_not_empty(error,path);
 if (nt_IsAccessDenied(error))
     return err_path_no_write_access(error,path);
 if (nt_IsBusy(error))
     return err_path_busy(error,path);
 if (nt_IsFileNotFound(error))
     return err_path_not_found(error,path);
 if (nt_IsNotDir(error))
     return err_path_no_dir(error,path);
 if (nt_IsUnsupported(error)) {
  return DeeError_SysThrowf(&DeeError_UnsupportedAPI,error,
                            "The filesystem hosting the path %r does "
                            "not support the removal of directories",
                            path);
 }
 return DeeError_SysThrowf(&DeeError_FSError,error,
                           "Failed to remove directory %r",
                           path);
}

INTERN int DCALL
fs_rmdir(DeeObject *__restrict path) {
 int error;
 if (DeeThread_CheckInterrupt()) goto err;
 if (DeeObject_AssertTypeExact(path,&DeeString_Type))
     goto err;
again:
 error = nt_RemoveDirectory(path);
 if unlikely(error > 0) goto err_nt;
 return error;
err_nt:
 DBG_ALIGNMENT_DISABLE();
 error = (int)GetLastError();
 DBG_ALIGNMENT_ENABLE();
 if (nt_IsBadAlloc((DWORD)error)) {
  if (Dee_CollectMemory(1))
      goto again;
 }
 handle_rmdir_error((DWORD)error,path);
err:
 return -1;
}

PRIVATE ATTR_COLD int DCALL
handle_unlink_error(DWORD error, DeeObject *__restrict path) {
 if (error == ERROR_ACCESS_DENIED) {
  DWORD dwAttributes; int temp;
  /* Check if the path is actually a directory. */
  temp = nt_GetFileAttributes(path,&dwAttributes);
  if (temp < 0) return temp;
  if (temp == 0 && (dwAttributes & FILE_ATTRIBUTE_DIRECTORY))
      return err_path_is_dir(error,path);
 }
 if (nt_IsAccessDenied(error))
     return err_path_no_write_access(error,path);
 if (nt_IsBusy(error))
     return err_path_busy(error,path);
 if (nt_IsNotDir(error))
     return err_path_no_dir(error,path);
 if (nt_IsFileNotFound(error))
     return err_path_not_found(error,path);
 if (nt_IsUnsupported(error)) {
  return DeeError_SysThrowf(&DeeError_UnsupportedAPI,error,
                            "The filesystem hosting the path %r "
                            "does not support unlinking files",
                            path);
 }
 return DeeError_SysThrowf(&DeeError_FSError,error,
                           "Failed to unlink file %r",
                           path);
}

INTERN int DCALL
fs_unlink(DeeObject *__restrict path) {
 int error;
 if (DeeThread_CheckInterrupt()) goto err;
 if (DeeObject_AssertTypeExact(path,&DeeString_Type))
     goto err;
again_deletefile:
 error = nt_DeleteFile(path);
 if unlikely(error <= 0)
    return error;
 DBG_ALIGNMENT_DISABLE();
 error = (int)GetLastError();
 DBG_ALIGNMENT_ENABLE();
 if (error == ERROR_ACCESS_DENIED) {
  /* Check if we've failed to delete a symbolic
   * directory-link (for which RemoveDirectory() must be used) */
  DWORD attr;
again_getattr:
  error = nt_GetFileAttributes(path,&attr);
  if (error < 0) goto err;
  if (!error &&
      (attr & (FILE_ATTRIBUTE_DIRECTORY|FILE_ATTRIBUTE_REPARSE_POINT)) ==
              (FILE_ATTRIBUTE_DIRECTORY|FILE_ATTRIBUTE_REPARSE_POINT)) {
again_rmdir:
   error = nt_RemoveDirectory(path);
   if unlikely(error <= 0)
      return error;
   DBG_ALIGNMENT_DISABLE();
   error = (int)GetLastError();
   DBG_ALIGNMENT_ENABLE();
   if (nt_IsBadAlloc((DWORD)error)) {
    if (Dee_CollectMemory(1))
        goto again_rmdir;
    goto err;
   }
   handle_rmdir_error((DWORD)error,path);
   goto err;
  }
  DBG_ALIGNMENT_DISABLE();
  error = (int)GetLastError();
  DBG_ALIGNMENT_ENABLE();
  if (nt_IsBadAlloc((DWORD)error)) {
   if (Dee_CollectMemory(1))
       goto again_getattr;
   goto err;
  }
  error = ERROR_ACCESS_DENIED;
 }
 if (nt_IsBadAlloc((DWORD)error)) {
  if (Dee_CollectMemory(1))
      goto again_deletefile;
  goto err;
 }
 handle_unlink_error((DWORD)error,path);
err:
 return -1;
}
INTERN int DCALL
fs_remove(DeeObject *__restrict path) {
 int error;
 if (DeeThread_CheckInterrupt()) goto err;
 if (DeeObject_AssertTypeExact(path,&DeeString_Type))
     goto err;
again:
 error = nt_DeleteFile(path);
 if likely(error <= 0)
    return error;
 DBG_ALIGNMENT_DISABLE();
 error = (int)GetLastError();
 DBG_ALIGNMENT_ENABLE();
 /* NOTE: DeleteFile() sets `ERROR_ACCESS_DENIED'
  *       if the folder is actually a directory. */
 if (error == ERROR_ACCESS_DENIED) {
  error = nt_RemoveDirectory(path);
  if (error <= 0) return error;
  if (nt_IsBadAlloc((DWORD)error)) {
   if (Dee_CollectMemory(1))
       goto again;
   goto err;
  }
  handle_rmdir_error((DWORD)error,path);
 }
 if (nt_IsBadAlloc((DWORD)error)) {
  if (Dee_CollectMemory(1))
      goto again;
  goto err;
 }
 handle_unlink_error((DWORD)error,path);
err:
 return -1;
}



INTERN int DCALL
fs_rename(DeeObject *__restrict existing_path,
          DeeObject *__restrict new_path) {
 int error;
 if (!DeeString_Check(existing_path)) {
  if (DeeInt_Check(existing_path)) {
   HANDLE fd; /* Support for descriptor-based rename() */
   if (DeeObject_AsUIntptr(existing_path,(uintptr_t *)&fd))
    goto err;
   existing_path = nt_GetFilenameOfHandle(fd);
  } else {
   existing_path = DeeFile_Filename(existing_path);
  }
  if unlikely(!existing_path) goto err;
  error = fs_rename(existing_path,new_path);
  Dee_Decref(existing_path);
  return error;
 }
 if (DeeThread_CheckInterrupt()) goto err;
 if (DeeObject_AssertTypeExact(new_path,&DeeString_Type))
     goto err;
again_movefile:
 error = nt_MoveFile(existing_path,new_path);
 if likely(error <= 0) goto err_nt;
 return error;
err_nt:
 DBG_ALIGNMENT_DISABLE();
 error = (int)GetLastError();
 DBG_ALIGNMENT_ENABLE();
 if (nt_IsBadAlloc((DWORD)error)) {
  if (Dee_CollectMemory(1))
      goto again_movefile;
 } else if (nt_IsAccessDenied((DWORD)error)) {
  err_path_no_access2((DWORD)error,existing_path,new_path);
 } else if (nt_IsBusy((DWORD)error)) {
  DeeError_SysThrowf(&DeeError_BusyFile,(DWORD)error,
                     "Path %r or %r cannot be accessed because it is already in use",
                     existing_path,new_path);
 } else if (nt_IsNotDir((DWORD)error)) {
  DeeError_Throwf(&DeeError_ValueError,
                  "Cannot rename path %r to %r which is a sub-directory of the old",
                  existing_path,new_path);
 } else if (nt_IsFileNotFound((DWORD)error)) {
  err_path_not_found2((DWORD)error,existing_path,new_path);
 } else if (nt_IsNotEmpty((DWORD)error) || nt_IsExists((DWORD)error)) {
  err_path_exists((DWORD)error,new_path);
 } else if (nt_IsNotDir((DWORD)error)) {
  DeeError_SysThrowf(&DeeError_NoDirectory,(DWORD)error,
                     "Some part of the path %r or %r is not a directory",
                     existing_path,new_path);
 } else if (nt_IsUnsupported((DWORD)error)) {
  DeeError_SysThrowf(&DeeError_UnsupportedAPI,(DWORD)error,
                     "The filesystem hosting the paths %r and %r "
                     "does not support renaming of files",
                     existing_path,new_path);
 } else if (nt_IsXDev((DWORD)error)) {
  err_path_crossdev2((DWORD)error,existing_path,new_path);
 } else {
  DeeError_SysThrowf(&DeeError_FSError,(DWORD)error,
                     "Failed to rename %r to %r",
                     existing_path,new_path);
 }
err:
 return -1;
}



INTERN int DCALL
fs_link(DeeObject *__restrict existing_path,
        DeeObject *__restrict new_path) {
 int error;
 if (DeeThread_CheckInterrupt()) goto err;
 if (DeeObject_AssertTypeExact(existing_path,&DeeString_Type) ||
     DeeObject_AssertTypeExact(new_path,&DeeString_Type))
     goto err;
again:
 error = nt_CreateHardLink(new_path,existing_path,NULL);
 if likely(error <= 0)
    return error;
 DBG_ALIGNMENT_DISABLE();
 error = (int)GetLastError();
 DBG_ALIGNMENT_ENABLE();
 if (nt_IsBadAlloc((DWORD)error)) {
  if (Dee_CollectMemory(1))
      goto again;
 } else if (nt_IsAccessDenied((DWORD)error)) {
  err_path_no_access2((DWORD)error,existing_path,new_path);
 } else if (nt_IsExists((DWORD)error)) {
  err_path_exists((DWORD)error,new_path);
 } else if (nt_IsFileNotFound((DWORD)error)) {
  err_path_not_found2((DWORD)error,existing_path,new_path);
 } else if (nt_IsXDev((DWORD)error)) {
  err_path_crossdev2((DWORD)error,existing_path,new_path);
 } else if (nt_IsUnsupported((DWORD)error)) {
  DeeError_SysThrowf(&DeeError_UnsupportedAPI,(DWORD)error,
                     "The filesystem hosting the paths %r and %r "
                     "does not support creation of hardlinks",
                     existing_path,new_path);
 } else {
  DeeError_SysThrowf(&DeeError_FSError,(DWORD)error,
                     "Failed to rename %r to %r",
                     existing_path,new_path);
 }
err:
 return -1;
}


PRIVATE WCHAR const str_SeCreateSymbolicLinkPrivilege[] = {
    'S','e','C','r','e','a','t','e','S','y','m','b','o','l','i',
    'c','L','i','n','k','P','r','i','v','i','l','e','g','e',0};
PRIVATE BOOL DCALL nt_AcquirePrivilege(LPCWSTR lpName) {
 HANDLE tok,hProcess; LUID luid;
 TOKEN_PRIVILEGES tok_priv; DWORD error;
 hProcess = GetCurrentProcess();
 if unlikely(!OpenProcessToken(hProcess,TOKEN_ADJUST_PRIVILEGES,&tok))
    goto fail;
 if unlikely(!LookupPrivilegeValueW(NULL,lpName,&luid))
    goto fail;
 tok_priv.PrivilegeCount           = 1;
 tok_priv.Privileges[0].Luid       = luid;
 tok_priv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
 if unlikely(!AdjustTokenPrivileges(tok,FALSE,&tok_priv,0,NULL,NULL))
    return FALSE;
 error = GetLastError();
 SetLastError(0);
 return unlikely(error == ERROR_NOT_ALL_ASSIGNED) ? 0 : 1;
fail:
 return FALSE;
}


#define SYMBOLIC_LINK_FLAG_DIRECTORY                 0x1
#define SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE 0x2

INTERN int DCALL
fs_symlink(DeeObject *__restrict target_text,
           DeeObject *__restrict link_path,
           bool format_target) {
 PRIVATE DWORD symlink_additional_flags = SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE;
 PRIVATE BOOL holding_symlink_priv = FALSE;
 int error; DWORD flags;
 if (DeeThread_CheckInterrupt()) goto err;
 if (DeeObject_AssertTypeExact(target_text,&DeeString_Type) ||
     DeeObject_AssertTypeExact(link_path,&DeeString_Type))
     goto err;
 (void)format_target; /* TODO: Fix slashes. */
 flags = symlink_additional_flags;
 /* TODO: `SYMBOLIC_LINK_FLAG_DIRECTORY'
  * >> local abs_target = target_text;
  * >> if (!fs.isabs(target_text))
  * >>      abs_target = fs.abspath(target_text,fs.headof(link_path));
  * >> if (fs.stat.isdir(abs_target))
  * >>     flags |= SYMBOLIC_LINK_FLAG_DIRECTORY;
  */
again:
 error = nt_CreateSymbolicLink(link_path,target_text,flags);
 if unlikely(error > 0) goto err_nt;
 return error;
err_nt:
 DBG_ALIGNMENT_DISABLE();
 error = (int)GetLastError();
 DBG_ALIGNMENT_ENABLE();
 if (error == ERROR_INVALID_PARAMETER &&
    (flags&SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE)) {
  /* Older versions of windows didn't accept this flag. */
  ATOMIC_FETCHAND(symlink_additional_flags,~SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE);
  flags &= ~SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE;
  goto again;
 }
 /* Try to acquire the ~privilege~ to create symbolic links. */
 if (error == ERROR_PRIVILEGE_NOT_HELD) {
  if (!holding_symlink_priv) {
   DBG_ALIGNMENT_DISABLE();
   if (nt_AcquirePrivilege(str_SeCreateSymbolicLinkPrivilege)) {
    DBG_ALIGNMENT_ENABLE();
    holding_symlink_priv = TRUE;
    goto again;
   }
   DBG_ALIGNMENT_ENABLE();
  }
  /* May as well not exist at all... */
  DeeError_SysThrowf(&DeeError_UnsupportedAPI,error,
                     "The operating system has restricted "
                     "access to symlink functionality");
 } else if (nt_IsAccessDenied(error)) {
  err_path_no_access(error,link_path);
 } else if (nt_IsExists(error)) {
  err_path_exists(error,link_path);
 } else if (nt_IsFileNotFound(error)) {
  err_path_not_found(error,link_path);
 } else if (nt_IsNotDir(error)) {
  err_path_no_dir(error,link_path);
 } else if (nt_IsUnsupported(error)) {
  DeeError_SysThrowf(&DeeError_UnsupportedAPI,error,
                     "The filesystem hosting the path %r "
                     "does not support creation of symbolic links",
                     link_path);
 } else {
  DeeError_SysThrowf(&DeeError_FSError,error,
                     "Failed to create a symbolic link %r with text %r",
                     link_path,target_text);
 }
err:
 return -1;
}

typedef struct _REPARSE_DATA_BUFFER {
    ULONG  ReparseTag;
    USHORT ReparseDataLength;
    USHORT Reserved;
    union {
        struct {
            USHORT SubstituteNameOffset;
            USHORT SubstituteNameLength;
            USHORT PrintNameOffset;
            USHORT PrintNameLength;
            ULONG  Flags;
            WCHAR  PathBuffer[1];
        } SymbolicLinkReparseBuffer;
        struct {
            USHORT SubstituteNameOffset;
            USHORT SubstituteNameLength;
            USHORT PrintNameOffset;
            USHORT PrintNameLength;
            WCHAR  PathBuffer[1];
        } MountPointReparseBuffer;
        struct {
            UCHAR DataBuffer[1];
        } GenericReparseBuffer;
    };
} REPARSE_DATA_BUFFER, *PREPARSE_DATA_BUFFER;


#define READLINK_INITIAL_BUFFER 300
INTERN DREF DeeObject *DCALL
fs_readlink(DeeObject *__restrict path) {
 PREPARSE_DATA_BUFFER buffer; HANDLE link_fd;
 DREF DeeObject *result; DWORD bufsiz,buflen,error;
 LPWSTR linkstr_begin,linkstr_end; bool owns_linkfd;
 if (DeeString_Check(path)) {
again_createfile:
  link_fd = nt_CreateFile(path,FILE_READ_ATTRIBUTES,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,NULL,
                          OPEN_EXISTING,FILE_FLAG_BACKUP_SEMANTICS|FILE_FLAG_OPEN_REPARSE_POINT,NULL);
  if unlikely(!link_fd) goto err;
  if unlikely(link_fd == INVALID_HANDLE_VALUE)
     goto err_nt_createfile;
  owns_linkfd = true;
 } else if (DeeInt_Check(path)) {
  if (DeeObject_AsUIntptr(path,(uintptr_t *)&link_fd)) goto err;
  owns_linkfd = false;
 } else {
  link_fd = DeeFile_Fileno(path);
  if (link_fd == DSYSFD_INVALID) {
   if (!DeeError_Catch(&DeeError_AttributeError) &&
       !DeeError_Catch(&DeeError_NotImplemented))
        goto err;
   /* Use the filename of a file. */
   path = DeeFile_Filename(path);
   if unlikely(!path) goto err;
   result = fs_readlink(path);
   Dee_Decref(path);
   return result;
  }
  owns_linkfd = false;
 }
 bufsiz = READLINK_INITIAL_BUFFER;
 buffer = (PREPARSE_DATA_BUFFER)Dee_Malloc(bufsiz);
 if unlikely(!buffer) goto err_fd;
 /* Read symbolic link data. */
 DBG_ALIGNMENT_DISABLE();
 while (!DeviceIoControl(link_fd,FSCTL_GET_REPARSE_POINT,
                         NULL,0,buffer,bufsiz,&buflen,NULL)) {
  error = GetLastError();
  DBG_ALIGNMENT_ENABLE();
  if (error == ERROR_INSUFFICIENT_BUFFER ||
      error == ERROR_MORE_DATA) {
   PREPARSE_DATA_BUFFER new_buffer;
   bufsiz *= 2;
   new_buffer = (PREPARSE_DATA_BUFFER)Dee_Realloc(buffer,bufsiz);
   if unlikely(!new_buffer) goto err_buffer;
   DBG_ALIGNMENT_DISABLE();
   continue;
  }
  if (nt_IsAccessDenied(error)) {
   err_path_no_access(error,path);
  } else if (error == ERROR_NOT_A_REPARSE_POINT) {
   DeeError_SysThrowf(&DeeError_NoLink,error,
                      "Path %r is not a symbolic link",
                      path);
  } else if (nt_IsUnsupported(error)) {
   DeeError_SysThrowf(&DeeError_UnsupportedAPI,error,
                      "The filesystem hosting the path %r does "
                      "not support reading symbolic links",
                      path);
  } else {
   DeeError_SysThrowf(&DeeError_FSError,error,
                      "Failed to read symbolic link %r",
                      path);
  }
  goto err_buffer;
 }
 if (owns_linkfd) CloseHandle(link_fd);
 DBG_ALIGNMENT_ENABLE();
 /* Interpret the read data. */
 switch (buffer->ReparseTag) {
 case IO_REPARSE_TAG_SYMLINK:
  linkstr_end = (linkstr_begin = buffer->SymbolicLinkReparseBuffer.PathBuffer+
                (buffer->SymbolicLinkReparseBuffer.SubstituteNameOffset/sizeof(WCHAR)))+
                (buffer->SymbolicLinkReparseBuffer.SubstituteNameLength/sizeof(WCHAR));
  break;
 case IO_REPARSE_TAG_MOUNT_POINT:
  linkstr_end = (linkstr_begin = buffer->MountPointReparseBuffer.PathBuffer+
                (buffer->MountPointReparseBuffer.SubstituteNameOffset/sizeof(WCHAR)))+
                (buffer->MountPointReparseBuffer.SubstituteNameLength/sizeof(WCHAR));
  break;
 default:
  DeeError_Throwf(&DeeError_UnsupportedAPI,
                  "Unsupported link type %lu in file %r",
                 (unsigned long)buffer->ReparseTag,path);
  Dee_Free(buffer);
  goto err;
 }
 /* Get rid of that annoying '\??\' prefix */
 if (linkstr_begin+4 <= linkstr_end &&
     linkstr_begin[0] == '\\' && linkstr_begin[1] == '?' &&
     linkstr_begin[2] == '?'  && linkstr_begin[3] == '\\')
     linkstr_begin += 4;
 /* Create the resulting string. */
 result = DeeString_NewWide(linkstr_begin,
                           (size_t)(linkstr_end-linkstr_begin),
                            STRING_ERROR_FREPLAC);
 /* Free our buffer. */
 Dee_Free(buffer);
 return result;
err_nt_createfile:
 DBG_ALIGNMENT_DISABLE();
 error = (int)GetLastError();
 DBG_ALIGNMENT_ENABLE();
 if (nt_IsBadAlloc((DWORD)error)) {
  if (Dee_CollectMemory(1))
      goto again_createfile;
 } else if (nt_IsNotDir((DWORD)error)) {
  err_path_no_dir((DWORD)error,path);
 } else if (nt_IsAccessDenied((DWORD)error)) {
  err_path_no_access((DWORD)error,path);
 } else if (nt_IsFileNotFound((DWORD)error)) {
  err_path_not_found((DWORD)error,path);
 } else if (nt_IsBadf((DWORD)error)) {
  err_handle_closed((DWORD)error,path);
 } else {
  DeeError_SysThrowf(&DeeError_FSError,(DWORD)error,
                     "Failed to open file %r",
                     path);
 }
err:
 return NULL;
err_buffer:
 Dee_Free(buffer);
err_fd:
 if (owns_linkfd) {
  DBG_ALIGNMENT_DISABLE();
  CloseHandle(link_fd);
  DBG_ALIGNMENT_ENABLE();
 }
 goto err;
}



typedef struct {
    OBJECT_HEAD
    DREF DeeStringObject *d_path; /* [1..1] The path describing this directory. */
} Dir;

typedef struct {
    OBJECT_HEAD
    DREF Dir        *d_dir;  /* [1..1][const] The associated directory. */
    HANDLE           d_hnd;  /* [0..1|NULL(INVALID_HANDLE_VALUE)][lock(d_lock)]
                              * The iteration handle or INVALID_HANDLE_VALUE when exhausted. */
    WIN32_FIND_DATAW d_data; /* [lock(d_lock)] The file data for the next matching entry. */
#ifndef CONFIG_NO_THREADS
    rwlock_t         d_lock;
#endif
} DirIterator;

PRIVATE void DCALL
diriter_fini(DirIterator *__restrict self) {
 DBG_ALIGNMENT_DISABLE();
 FindClose(self->d_hnd);
 DBG_ALIGNMENT_ENABLE();
 Dee_Decref(self->d_dir);
}

PRIVATE void DCALL
diriter_visit(DirIterator *__restrict self, dvisit_t proc, void *arg) {
 Dee_Visit(self->d_dir);
}

#if 0 /* Find-handles aren't real handles! */
PRIVATE int DCALL
diriter_copy(DirIterator *__restrict self,
             DirIterator *__restrict other) {
 HANDLE prochnd = GetCurrentProcess();
#ifndef CONFIG_NO_THREADS
 rwlock_read(&other->d_lock);
#endif
 if (other->d_hnd == INVALID_HANDLE_VALUE) {
  /* The other directory has been exhausted. */
  self->d_hnd = INVALID_HANDLE_VALUE;
 } else {
  DBG_ALIGNMENT_DISABLE();
  if (!DuplicateHandle(prochnd,other->d_hnd,
                       prochnd,&self->d_hnd,
                       0,TRUE,DUPLICATE_SAME_ACCESS)) {
   DBG_ALIGNMENT_ENABLE();
#ifndef CONFIG_NO_THREADS
   rwlock_endread(&other->d_lock);
#endif
   nt_ThrowLastError();
   return -1;
  }
  DBG_ALIGNMENT_ENABLE();
  memcpy(&self->d_data,&other->d_data,sizeof(WIN32_FIND_DATAW));
 }
 self->d_dir = other->d_dir;
 Dee_Incref(self->d_dir);
#ifndef CONFIG_NO_THREADS
 rwlock_endread(&other->d_lock);
#endif
 return 0;
}
#endif

PRIVATE ATTR_COLD int DCALL
err_handle_findnextfile(DWORD dwError, DeeObject *__restrict path) {
 return DeeError_SysThrowf(&DeeError_FSError,dwError,
                           "Failed to read entires from directory %r",
                           path);
}


PRIVATE DREF DeeStringObject *DCALL
diriter_next(DirIterator *__restrict self) {
 WCHAR *result_string,*begin;
 size_t length; DWORD dwError;
again:
 if (DeeThread_CheckInterrupt()) goto err;
#ifndef CONFIG_NO_THREADS
 rwlock_write(&self->d_lock);
#endif
 /* Quick check: Has the iterator been exhausted. */
 if (self->d_hnd == INVALID_HANDLE_VALUE) {
#ifndef CONFIG_NO_THREADS
  rwlock_endwrite(&self->d_lock);
#endif
iter_done:
  return (DREF DeeStringObject *)ITER_DONE;
 }
read_filename:
 begin = self->d_data.cFileName,length = 0;
 for (; length < COMPILER_LENOF(self->d_data.cFileName) && begin[length]; ++length);
 if (length <= 2 && begin[0] == '.' &&
    (length == 1 || begin[1] == '.')) {
  /* Skip self/parent directories. */
  DBG_ALIGNMENT_DISABLE();
  if (!FindNextFileW(self->d_hnd,&self->d_data)) {
   dwError = GetLastError();
   DBG_ALIGNMENT_ENABLE();
   if (dwError == ERROR_NO_MORE_FILES) {
    HANDLE hnd = self->d_hnd;
    self->d_hnd = INVALID_HANDLE_VALUE;
#ifndef CONFIG_NO_THREADS
    rwlock_endwrite(&self->d_lock);
#endif
    DBG_ALIGNMENT_DISABLE();
    FindClose(hnd);
    DBG_ALIGNMENT_ENABLE();
    goto iter_done;
   }
#ifndef CONFIG_NO_THREADS
   rwlock_endwrite(&self->d_lock);
#endif
   if (nt_IsBadAlloc(dwError)) {
    if (Dee_CollectMemory(1))
        goto again;
    goto err;
   }
   goto err_read_failed;
  }
  DBG_ALIGNMENT_ENABLE();
  goto read_filename;
 }
 result_string = (WCHAR *)Dee_TryMalloc(sizeof(size_t)+4+length*2);
 if unlikely(!result_string) {
#ifndef CONFIG_NO_THREADS
  rwlock_endwrite(&self->d_lock);
#endif
  if (Dee_CollectMemory(sizeof(size_t)+4+length*2))
      goto again;
  goto err;
 }
 /* Create an encoding string. */
 *(*(size_t **)&result_string)++ = length;
 memcpy(result_string,self->d_data.cFileName,length*2);
 result_string[length] = 0;
 /* Advance the directory by one. */
 DBG_ALIGNMENT_DISABLE();
 if (!FindNextFileW(self->d_hnd,&self->d_data)) {
  HANDLE hnd;
  dwError = GetLastError();
  DBG_ALIGNMENT_ENABLE();
  if unlikely(dwError != ERROR_NO_MORE_FILES) {
#ifndef CONFIG_NO_THREADS
   rwlock_endwrite(&self->d_lock);
#endif
   if (nt_IsBadAlloc(dwError)) {
    if (Dee_CollectMemory(1))
        goto again;
    goto err;
   }
   goto err_read_failed;
  }
  hnd = self->d_hnd;
  self->d_hnd = INVALID_HANDLE_VALUE;
#ifndef CONFIG_NO_THREADS
  rwlock_endwrite(&self->d_lock);
#endif
  DBG_ALIGNMENT_DISABLE();
  FindClose(hnd);
  DBG_ALIGNMENT_ENABLE();
 } else {
  DBG_ALIGNMENT_ENABLE();
#ifndef CONFIG_NO_THREADS
  rwlock_endwrite(&self->d_lock);
#endif
 }
 /* Manually construct a string object and fill
  * it with data read from the directory entry. */
 return (DREF DeeStringObject *)DeeString_PackWideBuffer(result_string,STRING_ERROR_FREPLAC);
err_read_failed:
 err_handle_findnextfile(dwError,(DeeObject *)self->d_dir->d_path);
err:
 return NULL;
}

PRIVATE ATTR_COLD int DCALL
err_handle_opendir(DWORD error, DeeObject *__restrict path) {
 if (nt_IsFileNotFound(error))
     return err_path_not_found(error,path);
 if (nt_IsNotDir(error))
     return err_path_no_dir(error,path);
 return DeeError_SysThrowf(&DeeError_FSError,error,
                           "Failed to open directory %r",
                           path);
}


PRIVATE DREF DirIterator *DCALL
dir_iter(Dir *__restrict self) {
 DREF DirIterator *result;
 LPWSTR wname,wpattern; size_t i,wname_length;
 if (DeeThread_CheckInterrupt()) goto err;
 result = DeeObject_MALLOC(DirIterator);
 if unlikely(!result) goto err;
 wname = (LPWSTR)DeeString_AsWide((DeeObject *)self->d_path);
 if unlikely(!wname) goto err_r;
 /* Append the `\\*' to the given path and fix forward-slashes. */
 wname_length = WSTR_LENGTH(wname);
 wpattern     = (LPWSTR)Dee_AMalloc(8+wname_length*2);
 if unlikely(!wpattern) goto err_r;
 for (i = 0; i < wname_length; ++i) {
  WCHAR ch = wname[i];
  /* FindFirstFile() actually fails when handed forward-slashes.
   * That's something I didn't notice in the old deemon, which
   * caused fs.dir() to (seemingly) fail at random. */
  if (ch == '/')
      ch = '\\';
  wpattern[i] = ch;
 }
 /* Use the current directory if the given name is empty. */
 if (!wname_length)
     wpattern[wname_length++] = '.';
 /* Append a trailing backslash if there isn't one already. */
 if (wpattern[wname_length-1] != '\\')
     wpattern[wname_length++]  = '\\';
 /* Append a match-all wildcard. */
 wpattern[wname_length++] = '*';
 wpattern[wname_length]   = 0;
 DBG_ALIGNMENT_DISABLE();
 result->d_hnd = FindFirstFileExW(wpattern,FindExInfoBasic,&result->d_data,
                                  FindExSearchNameMatch,NULL,0);
 DBG_ALIGNMENT_ENABLE();
 Dee_AFree(wpattern);
 if unlikely(result->d_hnd == INVALID_HANDLE_VALUE) {
  DWORD dwError;
  DBG_ALIGNMENT_DISABLE();
  dwError = GetLastError();
  DBG_ALIGNMENT_ENABLE();
  if (dwError != ERROR_NO_MORE_FILES) {
   err_handle_opendir(dwError,(DeeObject *)self->d_path);
   goto err_r;
  }
  /* Empty directory? ok... */
 }
 /* Finish initializing misc. members. */
 Dee_Incref(self);
 result->d_dir = self;
#ifndef CONFIG_NO_THREADS
 rwlock_init(&result->d_lock);
#endif
 DeeObject_Init(result,&DeeDirIterator_Type);
 return result;
err_r:
 DeeObject_Free(result);
err:
 return NULL;
}

PRIVATE struct type_member diriter_members[] = {
    TYPE_MEMBER_FIELD("seq",STRUCT_OBJECT,offsetof(DirIterator,d_dir)),
    TYPE_MEMBER_END
};

INTERN DeeTypeObject DeeDirIterator_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"dir.iterator",
    /* .tp_doc      = */DeeDirIterator_TP_DOC,
    /* .tp_flags    = */TP_FNORMAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeIterator_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */NULL,
                /* .tp_copy_ctor = */NULL, /* (void *)&diriter_copy, */
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */NULL,
                /* .tp_free      = */NULL,
                {
                    /* .tp_instance_size = */sizeof(DirIterator)
                }
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&diriter_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */(void(DCALL *)(DeeObject *__restrict,dvisit_t,void*))&diriter_visit,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */NULL,
    /* .tp_seq           = */NULL,
    /* .tp_iter_next     = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&diriter_next,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */NULL,
    /* .tp_members       = */diriter_members,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */NULL
};

PRIVATE void DCALL
dir_fini(Dir *__restrict self) {
 Dee_Decref(self->d_path);
}

PRIVATE int DCALL
dir_copy(Dir *__restrict self,
         Dir *__restrict other) {
 self->d_path = other->d_path;
 Dee_Incref(self->d_path);
 return 0;
}

PRIVATE int DCALL
dir_ctor(Dir *__restrict self,
         size_t argc, DeeObject **__restrict argv) {
 if (DeeArg_Unpack(argc,argv,"o:dir",&self->d_path))
     goto err;
 if (DeeString_Check(self->d_path)) {
  Dee_Incref(self->d_path);
 } else if (DeeInt_Check(self->d_path)) {
  HANDLE fd; /* Support for descriptor-based dir() */
  if (DeeObject_AsUIntptr((DeeObject *)self->d_path,(uintptr_t *)&fd))
      goto err;
  self->d_path = (DREF DeeStringObject *)nt_GetFilenameOfHandle(fd);
  if unlikely(!self->d_path) goto err;
 } else {
  self->d_path = (DREF DeeStringObject *)DeeFile_Filename((DeeObject *)self->d_path);
  if unlikely(!self->d_path) goto err;
 }
 return 0;
err:
 return -1;
}

PRIVATE struct type_seq dir_seq = {
    /* .tp_iter_self = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&dir_iter,
    /* .tp_size      = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))NULL,
    /* .tp_contains  = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))NULL,
    /* .tp_get       = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))NULL,
    /* .tp_del       = */(int(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))NULL,
    /* .tp_set       = */(int(DCALL *)(DeeObject *__restrict,DeeObject *__restrict,DeeObject *__restrict))NULL,
    /* .tp_range_get = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict,DeeObject *__restrict))NULL,
    /* .tp_range_del = */(int(DCALL *)(DeeObject *__restrict,DeeObject *__restrict,DeeObject *__restrict))NULL,
    /* .tp_range_set = */(int(DCALL *)(DeeObject *__restrict,DeeObject *__restrict,DeeObject *__restrict,DeeObject *__restrict))NULL
};

PRIVATE struct type_member dir_class_members[] = {
    TYPE_MEMBER_CONST("iterator",&DeeDirIterator_Type),
    TYPE_MEMBER_END
};

INTERN DeeTypeObject DeeDir_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"dir",
    /* .tp_doc      = */DeeDir_TP_DOC,
    /* .tp_flags    = */TP_FNORMAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeSeq_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */NULL,
                /* .tp_copy_ctor = */(void *)&dir_copy,
                /* .tp_deep_ctor = */(void *)&dir_copy,
                /* .tp_any_ctor  = */(void *)&dir_ctor,
                /* .tp_free      = */NULL,
                {
                    /* .tp_instance_size = */sizeof(Dir)
                }
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&dir_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */NULL,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */NULL,
    /* .tp_seq           = */&dir_seq,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */NULL,
    /* .tp_members       = */NULL,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */dir_class_members
};


/* To a wildcard-enabled string comparison on `string' using `pattern'
 * Taken from one of my other projects: `KOS' - `/libs/libc/string.c:libc_wildstrcmp' */
PRIVATE int DCALL
wild_match(LPWSTR string, LPWSTR pattern) {
 WCHAR card_post;
 for (;;) {
  if (!*string) {
   /* End of string (if the patter is empty, or only contains '*', we have a match) */
   while (*pattern == '*') ++pattern;
   return -(int)*pattern;
  }
  if (!*pattern) return (int)*string; /* Pattern end doesn't match */
  if (*pattern == '*') {
   /* Skip starts */
   do ++pattern; while (*pattern == '*');
   if ((card_post = *pattern++) == '\0')
        return 0; /* Pattern ends with '*' (matches everything) */
   if (card_post == '?') goto next; /* Match any --> already found */
   card_post = (WCHAR)DeeUni_ToLower(card_post);
   for (;;) {
    WCHAR ch = *string++;
    if ((WCHAR)DeeUni_ToLower(ch) == card_post) {
     /* Recursively check if the rest of the string and pattern match */
     if (!wild_match(string,pattern))
          return 0;
    } else if (!ch) {
     return -(int)card_post; /* Wildcard suffix not found */
    }
   }
  }
  if (DeeUni_ToLower(*pattern) == DeeUni_ToLower(*string) ||
      *pattern == '?') {
next: ++string,++pattern;
   continue; /* single character match */
  }
  break; /* mismatch */
 }
 return *string-*pattern;
}


/* query() types. */
typedef struct {
    DirIterator q_iter; /* The underlying iterator. */
    LPWSTR      q_wild; /* The wildcard pattern string with which to match filenames. */
} QueryIterator;

PRIVATE DREF DeeStringObject *DCALL
queryiter_next(QueryIterator *__restrict self) {
 WCHAR *result_string,*begin;
 size_t length; DWORD dwError;
again:
 if (DeeThread_CheckInterrupt()) goto err;
#ifndef CONFIG_NO_THREADS
 rwlock_write(&self->q_iter.d_lock);
#endif
 /* Quick check: Has the iterator been exhausted. */
 if (self->q_iter.d_hnd == INVALID_HANDLE_VALUE) {
#ifndef CONFIG_NO_THREADS
  rwlock_endwrite(&self->q_iter.d_lock);
#endif
iter_done:
  return (DREF DeeStringObject *)ITER_DONE;
 }
read_filename:
 begin = self->q_iter.d_data.cFileName,length = 0;
 for (; length < COMPILER_LENOF(self->q_iter.d_data.cFileName)-1 &&
      begin[length]; ++length);
 COMPILER_ENDOF(self->q_iter.d_data.cFileName)[-1] = 0; /* Ensure validity. */
 if ((length <= 2 && begin[0] == '.' &&
     (length == 1 || begin[1] == '.')) ||
      wild_match(begin,self->q_wild) != 0) {
  /* Skip self/parent directories. */
  DBG_ALIGNMENT_DISABLE();
  if (!FindNextFileW(self->q_iter.d_hnd,&self->q_iter.d_data)) {
   dwError = GetLastError();
   DBG_ALIGNMENT_ENABLE();
   if (dwError == ERROR_NO_MORE_FILES) {
    HANDLE hnd = self->q_iter.d_hnd;
    self->q_iter.d_hnd = INVALID_HANDLE_VALUE;
#ifndef CONFIG_NO_THREADS
    rwlock_endwrite(&self->q_iter.d_lock);
#endif
    FindClose(hnd);
    goto iter_done;
   }
#ifndef CONFIG_NO_THREADS
   rwlock_endwrite(&self->q_iter.d_lock);
#endif
   if (nt_IsBadAlloc(dwError)) {
    if (Dee_CollectMemory(1))
        goto again;
    goto err;
   }
   goto err_read_failed;
  }
  DBG_ALIGNMENT_ENABLE();
  goto read_filename;
 }
 result_string = (WCHAR *)Dee_TryMalloc(sizeof(size_t)+4+length*2);
 if unlikely(!result_string) {
#ifndef CONFIG_NO_THREADS
  rwlock_endwrite(&self->q_iter.d_lock);
#endif
  if (Dee_CollectMemory(sizeof(size_t)+4+length*2))
      goto again;
  goto err;
 }
 /* Create an encoding string. */
 *(*(size_t **)&result_string)++ = length;
 memcpy(result_string,self->q_iter.d_data.cFileName,length*2);
 result_string[length] = 0;
 /* Advance the directory by one. */
 DBG_ALIGNMENT_DISABLE();
 if (!FindNextFileW(self->q_iter.d_hnd,&self->q_iter.d_data)) {
  HANDLE hnd;
  dwError = GetLastError();
  DBG_ALIGNMENT_ENABLE();
  if unlikely(dwError != ERROR_NO_MORE_FILES) {
#ifndef CONFIG_NO_THREADS
   rwlock_endwrite(&self->q_iter.d_lock);
#endif
   if (nt_IsBadAlloc(dwError)) {
    if (Dee_CollectMemory(1))
        goto again;
    goto err;
   }
   goto err_read_failed;
  }
  hnd = self->q_iter.d_hnd;
  self->q_iter.d_hnd = INVALID_HANDLE_VALUE;
#ifndef CONFIG_NO_THREADS
  rwlock_endwrite(&self->q_iter.d_lock);
#endif
  DBG_ALIGNMENT_DISABLE();
  FindClose(hnd);
  DBG_ALIGNMENT_ENABLE();
 } else {
  DBG_ALIGNMENT_ENABLE();
#ifndef CONFIG_NO_THREADS
  rwlock_endwrite(&self->q_iter.d_lock);
#endif
 }
 /* Manually construct a string object and fill
  * it with data read from the directory entry. */
 return (DREF DeeStringObject *)DeeString_PackWideBuffer(result_string,STRING_ERROR_FREPLAC);
err_read_failed:
 err_handle_findnextfile(dwError,(DeeObject *)self->q_iter.d_dir->d_path);
err:
 return NULL;
}


INTERN DeeTypeObject DeeQueryIterator_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"query.iterator",
    /* .tp_doc      = */DeeQueryIterator_TP_DOC,
    /* .tp_flags    = */TP_FNORMAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeDirIterator_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */NULL,
                /* .tp_copy_ctor = */NULL, /* (void *)&diriter_copy, */
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */NULL,
                /* .tp_free      = */NULL,
                {
                    /* .tp_instance_size = */sizeof(QueryIterator)
                }
            }
        },
        /* .tp_dtor        = */NULL,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */NULL,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */NULL,
    /* .tp_seq           = */NULL,
    /* .tp_iter_next     = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&queryiter_next,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */NULL,
    /* .tp_members       = */NULL,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */NULL
};

PRIVATE struct type_member query_class_members[] = {
    TYPE_MEMBER_CONST("iterator",&DeeQueryIterator_Type),
    TYPE_MEMBER_END
};


PRIVATE DREF QueryIterator *DCALL
query_iter(Dir *__restrict self) {
 DREF QueryIterator *result;
 LPWSTR wname,wpattern; size_t i,wname_length;
 if (DeeThread_CheckInterrupt()) goto err;
 result = DeeObject_MALLOC(QueryIterator);
 if unlikely(!result) goto err;
 wname = (LPWSTR)DeeString_AsWide((DeeObject *)self->d_path);
 if unlikely(!wname) goto err_r;
again_wname:
 /* Append the `\\*' to the given path and fix forward-slashes. */
 wname_length = WSTR_LENGTH(wname);
 /* Locate the previous directory. */
 while (wname_length &&
       (wname[wname_length-1] != '/' &&
        wname[wname_length-1] != '\\'))
      --wname_length;
 /* Set up the wildcard patter string pointer
  * of the resulting query iterator. */
 result->q_wild = wname+wname_length;
 wpattern = (LPWSTR)Dee_AMalloc(8+wname_length*2);
 if unlikely(!wpattern) goto err_r;
 for (i = 0; i < wname_length; ++i) {
  WCHAR ch = wname[i];
  /* FindFirstFile() actually fails when handed forward-slashes.
   * That's something I didn't notice in the old deemon, which
   * caused fs.dir() to (seemingly) fail at random. */
  if (ch == '/')
      ch = '\\';
  wpattern[i] = ch;
 }
 /* Use the current directory if the given name is empty. */
 if (!wname_length)
     wpattern[wname_length++] = '.';
 /* Append a trailing backslash. */
 wpattern[wname_length++] = '\\';
 /* Append a match-all wildcard. */
 wpattern[wname_length++] = '*';
 wpattern[wname_length]   = 0;
 DBG_ALIGNMENT_DISABLE();
 result->q_iter.d_hnd = FindFirstFileExW(wpattern,FindExInfoBasic,
                                        &result->q_iter.d_data,
                                         FindExSearchNameMatch,NULL,0);
 DBG_ALIGNMENT_ENABLE();
 Dee_AFree(wpattern);
 if unlikely(result->q_iter.d_hnd == INVALID_HANDLE_VALUE) {
  DWORD dwError;
  DBG_ALIGNMENT_DISABLE();
  dwError = GetLastError();
  DBG_ALIGNMENT_ENABLE();
  if (dwError != ERROR_NO_MORE_FILES) {
   DREF DeeObject *query_path;
   if (nt_IsBadAlloc(dwError)) {
    if (Dee_CollectMemory(1))
        goto again_wname;
    goto err;
   }
   query_path = DeeString_NewWide(wname,wname_length,
                                  STRING_ERROR_FIGNORE);
   if unlikely(!query_path) goto err_r;
   err_handle_opendir(dwError,query_path);
   Dee_Decref(query_path);
   goto err_r;
  }
  /* Empty directory? ok... */
 }
 /* Finish initializing misc. members. */
 Dee_Incref(self);
 result->q_iter.d_dir = self;
#ifndef CONFIG_NO_THREADS
 rwlock_init(&result->q_iter.d_lock);
#endif
 DeeObject_Init(&result->q_iter,&DeeQueryIterator_Type);
 return result;
err_r:
 DeeObject_Free(result);
err:
 return NULL;
}

PRIVATE struct type_seq query_seq = {
    /* .tp_iter_self = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&query_iter,
    /* .tp_size      = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))NULL,
    /* .tp_contains  = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))NULL,
    /* .tp_get       = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))NULL,
    /* .tp_del       = */(int(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))NULL,
    /* .tp_set       = */(int(DCALL *)(DeeObject *__restrict,DeeObject *__restrict,DeeObject *__restrict))NULL,
    /* .tp_range_get = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict,DeeObject *__restrict))NULL,
    /* .tp_range_del = */(int(DCALL *)(DeeObject *__restrict,DeeObject *__restrict,DeeObject *__restrict))NULL,
    /* .tp_range_set = */(int(DCALL *)(DeeObject *__restrict,DeeObject *__restrict,DeeObject *__restrict,DeeObject *__restrict))NULL
};

INTERN DeeTypeObject DeeQuery_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"query",
    /* .tp_doc      = */DeeQuery_TP_DOC,
    /* .tp_flags    = */TP_FNORMAL|TP_FINHERITCTOR,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeDir_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */NULL,
                /* .tp_copy_ctor = */NULL,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */NULL,
                /* .tp_free      = */NULL,
                {
                    /* .tp_instance_size = */sizeof(Dir)
                }
            }
        },
        /* .tp_dtor        = */NULL,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */NULL,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */NULL,
    /* .tp_seq           = */&query_seq,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */NULL,
    /* .tp_members       = */NULL,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */query_class_members
};


DECL_END

#ifndef __INTELLISENSE__
#include "nt.inl"
#endif

#endif /* !GUARD_DEX_FS_WINDOWS_C_INL */
