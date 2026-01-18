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
#ifndef GUARD_DEEMON_MAPFILE_H
#define GUARD_DEEMON_MAPFILE_H 1

#include "api.h"
/**/

#include "file.h"
#include "system-features.h" /* E*, CONFIG_HAVE_* */
/**/

#include <stdbool.h> /* bool */
#include <stddef.h>  /* size_t */

DECL_BEGIN

#ifdef DEE_SOURCE
#define Dee_map_file_object map_file_object
#endif /* DEE_SOURCE */


/************************************************************************/
/* MMAP API                                                             */
/************************************************************************/

/* Implementable via one of:
 * - CreateFileMapping()+MapViewOfFile()  (Windows)
 * - mmap()                               (Unix)
 * - malloc()+read()                      (Fallback)
 */
#undef DeeMapFile_IS_CreateFileMapping /* Windows's `CreateFileMapping' */
#undef DeeMapFile_IS_mmap              /* Unix's `mmap(2)' */
#undef DeeMapFile_IS_malloc            /* Fallback-only support */
#if defined(Dee_fd_t_IS_HANDLE) && defined(CONFIG_HOST_WINDOWS)
#define DeeMapFile_IS_CreateFileMapping
#elif ((defined(CONFIG_HAVE_fstat) || defined(CONFIG_HAVE_fstat64)) &&       \
       (defined(CONFIG_HAVE_mmap) || defined(CONFIG_HAVE_mmap64)) &&         \
       (defined(CONFIG_HAVE_lseek) || defined(CONFIG_HAVE_lseek64)) &&       \
       defined(CONFIG_HAVE_MAP_PRIVATE) && defined(CONFIG_HAVE_PROT_READ) && \
       defined(CONFIG_HAVE_PROT_WRITE) && defined(Dee_fd_t_IS_int))
#define DeeMapFile_IS_mmap
#else /* ... */
#define DeeMapFile_IS_malloc
#endif /* !... */


struct DeeMapFile {
#ifdef DeeMapFile_IS_CreateFileMapping
#define Dee_SIZEOF_DeeMapFile (4 * __SIZEOF_POINTER__)
	void   *dmf_addr;    /* [0..dmf_size][owned] Base address of the file mapping. */
	size_t  dmf_size;    /* Mapping size (in bytes, excluding trailing NUL-bytes) */
	void  *_dmf_hmap;    /* [0..1] file mapping handle */
	size_t _dmf_vfre;    /* [valid_if(_dmf_hmap != NULL)] When non-zero, must VirtualFree() this many bytes at `CEIL_ALIGN(dmf_addr + dmf_size, getpagesize())' */
#define DeeMapFile_SETHEAP(self)  (void)((self)->_dmf_hmap = NULL)
#define DeeMapFile_UsesMmap(self) ((self)->_dmf_hmap != NULL)
#elif defined(DeeMapFile_IS_mmap)
#define Dee_SIZEOF_DeeMapFile (3 * __SIZEOF_POINTER__)
	void   *dmf_addr;    /* [0..dmf_size][owned] Base address of the file mapping. */
	size_t  dmf_size;    /* Mapping size (in bytes, excluding trailing NUL-bytes) */
	size_t _dmf_mapsize; /* Used internally: the mmap'd file size, or `0' if `dmf_addr' was malloc'd */
#define DeeMapFile_SETHEAP(self)  (void)((self)->_dmf_mapsize = 0)
#define DeeMapFile_UsesMmap(self) ((self)->_dmf_mapsize != 0)
#else /* ... */
#define Dee_SIZEOF_DeeMapFile (2 * __SIZEOF_POINTER__)
	void   *dmf_addr;    /* [0..dmf_size][owned] Base address of the file mapping. */
	size_t  dmf_size;    /* Mapping size (in bytes, excluding trailing NUL-bytes) */
#define DeeMapFile_SETHEAP(self)  (void)0
#define DeeMapFile_UsesMmap(self) 0
#define DeeMapFile_UsesMmap_IS_ALWAYS_ZERO
#endif /* !... */
};

#define DeeMapFile_GetAddr(self)    (self)->dmf_addr
#define DeeMapFile_GetSize(self)    (self)->dmf_size
#define DeeMapFile_SETADDR(self, p) (void)((self)->dmf_addr = (void *)(p))
#define DeeMapFile_SETSIZE(self, s) (void)((self)->dmf_size = (s))

/* Finalize a given file map */
DFUNDEF NONNULL((1)) void DCALL
DeeMapFile_Fini(struct DeeMapFile *__restrict self);
#define DeeMapFile_Move(dst, src) (void)(*(dst) = *(src))

/* Try to inplace-realloc-truncate the buffer of `self' ("inplace"
 * meaning that `DeeMapFile_GetAddr(self)' will never change) such
 * that `DeeMapFile_GetSize(self)' will be set to `newsize'.
 *
 * Note that this will **NOT** retain trailing NUL-bytes that may
 * have been allocated by `DeeMapFile_InitSysFd()', and that more
 * memory than `newsize' may need to be retained due to pagesize
 * requirements (though this function will never increase the size
 * of the file mapping).
 *
 * @return: true : Success: `DeeMapFile_GetSize(self)' has been lowered from its
 *                          previous value to some value that is `>= newsize'.
 * @return: false: Failure: Mapping size could not be reduced. Note that this is
 *                          **NOT** an error (and also should not be treated as
 *                          such), since the mapping is, and remains, up-and-
 *                          running (but will just remain so with its previous
 *                          size). */
DFUNDEF NONNULL((1)) bool DCALL
DeeMapFile_TryTruncate(struct DeeMapFile *__restrict self,
                       size_t newsize);

/* Initialize a file mapping from a given system FD.
 * @param: fd:        The file that should be loaded into memory.
 * @param: self:      Filled with mapping information. This structure contains at least 2 fields:
 *                     - DeeMapFile_GetAddr: Filled with the base address of a mapping of the file's contents
 *                     - DeeMapFile_GetSize: The actual number of mapped bytes (excluding `num_trailing_nulbytes')
 *                                           This will always be `>= min_bytes && <= max_bytes'.
 *                     - Other fields are implementation-specific
 *                    Note that the memory located at `DeeMapFile_GetAddr' is writable, though changes to
 *                    it are guarantied not to be written back to `fd'. iow: it behaves like MAP_PRIVATE
 *                    mapped as PROT_READ|PROT_WRITE.
 * @param: offset:    File offset / number of leading bytes that should not be mapped
 *                    When set to `(Dee_pos_t)-1', use the fd's current file position.
 * @param: min_bytes: The  min number of bytes (excluding num_trailing_nulbytes) that should be mapped
 *                    starting  at `offset'. If the file is smaller than this, or indicates EOF before
 *                    this number of bytes has been reached,  nul bytes are mapped for its  remainder.
 *                    Note that this doesn't include `num_trailing_nulbytes', meaning that (e.g.) when
 *                    an entirely empty file is mapped you get a buffer like:
 *                    >> mf_addr = calloc(min_bytes + num_trailing_nulbytes);
 *                    >> mf_size = min_bytes;
 *                    This argument essentially acts as if `fd' was at least `min_bytes' bytes large
 *                    by filling the non-present address range with all zeroes.
 * @param: max_bytes: The max number of bytes (excluding num_trailing_nulbytes) that should be mapped
 *                    starting at `offset'. If the file is smaller than this, or indicates EOF before
 *                    this number of bytes has been reached, simply stop there. - The actual number of
 *                    mapped bytes (excluding `num_trailing_nulbytes') is `DeeMapFile_GetSize'.
 * @param: num_trailing_nulbytes: When non-zero, append this many trailing NUL-bytes at the end of
 *                    the mapping. More bytes than this may be appended if necessary, but at least
 *                    this many are guarantied to be. - Useful if you want to load a file as a
 *                    string, in which case you can specify `1' to always have a trailing '\0' be
 *                    appended:
 *                    >> bzero(DeeMapFile_GetAddr + DeeMapFile_GetSize, num_trailing_nulbytes);
 * @param: flags:     Set of `DeeMapFile_F_*'
 * @return:  1: Both `DeeMapFile_F_MUSTMMAP' and `DeeMapFile_F_TRYMMAP' were set, but mmap failed.
 * @return:  0: Success (`self' must be deleted using `DeeMapFile_Fini(3)')
 * @return: -1: Error (an exception was thrown) */
DFUNDEF WUNUSED NONNULL((1)) int DCALL
DeeMapFile_InitSysFd(struct DeeMapFile *__restrict self, Dee_fd_t fd,
                     Dee_pos_t offset, size_t min_bytes, size_t max_bytes,
                     size_t num_trailing_nulbytes, unsigned int flags);

/* Same as `DeeMapFile_InitSysFd()', but initialize from a deemon File object. */
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL
DeeMapFile_InitFile(struct DeeMapFile *__restrict self,
                    DeeObject *__restrict file,
                    Dee_pos_t offset, size_t min_bytes, size_t max_bytes,
                    size_t num_trailing_nulbytes, unsigned int flags);

/* Bits for the `flags' argument of `DeeMapFile_InitSysFd()' and `DeeMapFile_InitFile()' */
#define DeeMapFile_F_NORMAL    0x0000 /* Normal flags */
#define DeeMapFile_F_READALL   0x0001 /* Flag: use `preadall(3)' / `readall(3)' instead of `pread(2)' / `read(2)' */
#define DeeMapFile_F_MUSTMMAP  0x0002 /* Flag: require the use of a mmap(2) */
#define DeeMapFile_F_MAPSHARED 0x0004 /* Flag: when using mmap, don't map as MAP_PRIVATE, but use MAP_SHARED (don't pass a non-zero `num_trailing_nulbytes' in this case!) */
#define DeeMapFile_F_ATSTART   0x0008 /* Flag: assume that the given file's pointer is located at the file's beginning */
#define DeeMapFile_F_TRYMMAP   0x8000 /* Flag: Don't throw an exception when mmap fails, but return `1' */




/* High-level wrapper for `DeeMapFile'
 * NOTE: This type of object implements the buffer interfaces, meaning that
 *       deemon code can access the mapped  */
typedef struct Dee_map_file_object DeeMapFileObject;
struct Dee_map_file_object {
	Dee_OBJECT_HEAD
	struct DeeMapFile mf_map;   /* The file map owned by this object. */
	size_t            mf_rsize; /* Real file map size (including trailing NUL bytes) */
};

DDATDEF DeeTypeObject DeeMapFile_Type;
#define DeeMapFile_Check(ob)      DeeObject_InstanceOfExact(ob, &DeeMapFile_Type) /* `_MapFile' is final! */
#define DeeMapFile_CheckExact(ob) DeeObject_InstanceOfExact(ob, &DeeMapFile_Type)


DECL_END

#endif /* !GUARD_DEEMON_MAPFILE_H */
