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
#ifndef GUARD_DEX_FS_LIBFS_H
#define GUARD_DEX_FS_LIBFS_H 1

#include <deemon/api.h>

#ifdef CONFIG_HOST_WINDOWS
#define CONFIG_LIBFS_GROUPTYPE_IS_USERTYPE 1
#endif

#include <deemon/dex.h>
#include <deemon/object.h>
#include <deemon/string.h>

#ifndef CONFIG_HOST_WINDOWS
#include <sys/stat.h>
#endif

#include <stdbool.h>

#include "libfs-doc.h"

DECL_BEGIN

struct ascii_printer;
typedef struct user_object DeeUserObject;
typedef struct stat_object DeeStatObject;

/* Get/Set the current working directory. */
INTDEF DREF /*String*/DeeObject *DCALL fs_getcwd(void);
INTDEF int DCALL fs_printcwd(struct ascii_printer *__restrict printer);
INTDEF int DCALL fs_chdir(DeeObject *__restrict path);

/* Return a temporary directory, such as `/tmp' */
INTDEF DREF /*String*/DeeObject *DCALL fs_gettmp(void);

/* Return a user-defined, global name for the hosting machine. */
INTDEF DREF /*String*/DeeObject *DCALL fs_gethostname(void);

/* System-specific environment variable implementation. */
INTDEF DREF /*String*/DeeObject *DCALL fs_getenv(/*String*/DeeObject *__restrict name, bool try_get);
INTDEF bool DCALL fs_hasenv(/*String*/DeeObject *__restrict name);
INTDEF int DCALL fs_delenv(/*String*/DeeObject *__restrict name);
INTDEF int DCALL fs_setenv(/*String*/DeeObject *__restrict name, /*String*/DeeObject *__restrict value);
/* @return:  1: Failed to retrieve the home path and `try_get' was true.
 * @return:  0: Successfully printed the home path.
 * @return: -1: An error occurred. */
INTDEF int DCALL fs_printenv(char const *__restrict name,
                             struct ascii_printer *__restrict printer, bool try_get);

/* The descriptor for a user account.
 * When default-constructed, return a descriptor for the current user.
 * Otherwise, this type is used by stat(), as well as other objects.
 * Additional its class function XXX:TODO can be used to enumerate known (or rather visible) users. */
INTDEF DeeTypeObject DeeUser_Type;
#ifdef CONFIG_LIBFS_GROUPTYPE_IS_USERTYPE
#define DeeGroup_Type DeeUser_Type
#else
INTDEF DeeTypeObject  DeeGroup_Type;
#endif

/* Lookup the home folder of the current user.
 * >> Same as `fs.user().home()' */
INTDEF DREF DeeObject *DCALL fs_gethome(bool try_get);
/* @return:  1: Failed to retrieve the home path and `try_get' was true.
 * @return:  0: Successfully printed the home path.
 * @return: -1: An error occurred. */
INTDEF int DCALL fs_printhome(struct ascii_printer *__restrict printer, bool try_get);

/* STAT bitflags. */

/* (Mostly) standard UNIX stat flag bits. (We try to mirror these on NT) */
#ifdef CONFIG_HOST_WINDOWS
#   define STAT_IFMT   0170000 /* These bits determine file type. */
#   define STAT_IFDIR  0040000 /* Directory. */
#   define STAT_IFCHR  0020000 /* Character device. */
#   define STAT_IFBLK  0060000 /* Block device. */
#   define STAT_IFREG  0100000 /* Regular file. */
#   define STAT_IFIFO  0010000 /* FIFO. */
#   define STAT_IFLNK  0120000 /* Symbolic link. */
#   define STAT_IFSOCK 0140000 /* Socket. */
#   define STAT_ISUID  0004000 /* Set user ID on execution. */
#   define STAT_ISGID  0002000 /* Set group ID on execution. */
#   define STAT_ISVTX  0001000 /* Save swapped text after use (sticky). */
#else
#ifdef __S_IFDIR
#   define STAT_IFDIR  __S_IFDIR
#elif defined(S_IFDIR)
#   define STAT_IFDIR  S_IFDIR
#else
#   define STAT_IFDIR  0040000 /* Directory. */
#endif
#ifdef __STAT_IFCHR
#   define STAT_IFCHR  __S_IFCHR
#elif defined(S_IFCHR)
#   define STAT_IFCHR  S_IFCHR
#else
#   define STAT_IFCHR  0020000 /* Character device. */
#endif
#ifdef __STAT_IFBLK
#   define STAT_IFBLK  __S_IFBLK
#elif defined(S_IFBLK)
#   define STAT_IFBLK  S_IFBLK
#else
#   define STAT_IFBLK  0060000 /* Block device. */
#endif
#ifdef __STAT_IFREG
#   define STAT_IFREG  __S_IFREG
#elif defined(S_IFREG)
#   define STAT_IFREG  S_IFREG
#else
#   define STAT_IFREG  0100000 /* Regular file. */
#endif
#ifdef __STAT_IFIFO
#   define STAT_IFIFO  __S_IFIFO
#elif defined(S_IFIFO)
#   define STAT_IFIFO  S_IFIFO
#else
#   define STAT_IFIFO  0010000 /* FIFO. */
#endif
#ifdef __STAT_IFLNK
#   define STAT_IFLNK  __S_IFLNK
#elif defined(S_IFLNK)
#   define STAT_IFLNK  S_IFLNK
#else
#   define STAT_IFLNK  0120000 /* Symbolic link. */
#endif
#ifdef __STAT_IFSOCK
#   define STAT_IFSOCK __S_IFSOC
#elif defined(S_IFSOC)
#   define STAT_IFSOCK S_IFSOC
#else
#   define STAT_IFSOCK 0140000 /* Socket. */
#endif
#ifdef __STAT_ISUID
#   define STAT_ISUID  __S_ISUID
#elif defined(S_ISUID)
#   define STAT_ISUID  S_ISUID
#else
#   define STAT_ISUID  0004000 /* Set user ID on execution. */
#endif
#ifdef __STAT_ISGID
#   define STAT_ISGID  __S_ISGID
#elif defined(S_ISGID)
#   define STAT_ISGID  S_ISGID
#else
#   define STAT_ISGID  0002000 /* Set group ID on execution. */
#endif
#ifdef __STAT_ISVTX
#   define STAT_ISVTX  __S_ISVTX
#elif defined(S_ISVTX)
#   define STAT_ISVTX  S_ISVTX
#else
#   define STAT_ISVTX  0001000 /* Save swapped text after use (sticky). */
#endif
#endif

#define STAT_IRUSR  0000400 /* Read by owner. */
#define STAT_IWUSR  0000200 /* Write by owner. */
#define STAT_IXUSR  0000100 /* Execute by owner. */
#define STAT_IRGRP (STAT_IRUSR >> 3) /* Read by group. */
#define STAT_IWGRP (STAT_IWUSR >> 3) /* Write by group. */
#define STAT_IXGRP (STAT_IXUSR >> 3) /* Execute by group. */
#define STAT_IROTH (STAT_IRUSR >> 6) /* Read by other. */
#define STAT_IWOTH (STAT_IWUSR >> 6) /* Write by other. */
#define STAT_IXOTH (STAT_IXUSR >> 6) /* Execute by other. */


/* Directory iterators. */
INTDEF DeeTypeObject DeeDirIterator_Type;
INTDEF DeeTypeObject DeeDir_Type;
INTDEF DeeTypeObject DeeQueryIterator_Type;
INTDEF DeeTypeObject DeeQuery_Type;


/* An os-specific type `stat' implementing at the very least the following:
 * >> stat(string path);
 * >> stat(file path);
 * >> @throw FileNotFound The given @path could not be found
 * >>
 * >> property st_dev   -> int;
 * >> property st_ino   -> int;
 * >> property st_mode  -> int;
 * >> property st_nlink -> int;
 * >> property st_uid   -> user;
 * >> property st_gid   -> group;
 * >> property st_rdev  -> int;
 * >> property st_size  -> int;
 * >> property st_atime -> time from time;
 * >> property st_mtime -> time from time;
 * >> property st_ctime -> time from time;
 */
INTDEF DeeTypeObject DeeStat_Type;
INTDEF DeeTypeObject DeeLStat_Type;


/* Filesystem write operations. */
INTDEF int DCALL fs_chtime(DeeObject *__restrict path, DeeObject *__restrict atime,
                           DeeObject *__restrict mtime, DeeObject *__restrict ctime);
INTDEF int DCALL fs_chmod(DeeObject *__restrict path, DeeObject *__restrict mode);
#ifdef CONFIG_HOST_WINDOWS
INTDEF int DCALL fs_chattr_np(DeeObject *__restrict path, DeeObject *__restrict new_attr);
#endif /* CONFIG_HOST_WINDOWS */
INTDEF int DCALL fs_chown(DeeObject *__restrict path, DeeObject *__restrict user, DeeObject *__restrict group);
INTDEF int DCALL fs_mkdir(DeeObject *__restrict path, DeeObject *__restrict perm);
INTDEF int DCALL fs_rmdir(DeeObject *__restrict path);
INTDEF int DCALL fs_unlink(DeeObject *__restrict path);
INTDEF int DCALL fs_remove(DeeObject *__restrict path);
INTDEF int DCALL fs_rename(DeeObject *__restrict existing_path, DeeObject *__restrict new_path);
INTDEF int DCALL fs_copyfile(DeeObject *__restrict existing_file, DeeObject *__restrict new_file,
                             DeeObject *__restrict progress_callback);
INTDEF int DCALL fs_link(DeeObject *__restrict existing_path, DeeObject *__restrict new_path);
INTDEF int DCALL fs_symlink(DeeObject *__restrict target_text, DeeObject *__restrict link_path, bool format_target);
INTDEF DREF DeeObject *DCALL fs_readlink(DeeObject *__restrict path);


/* Return the mask and flags to be applied
 * using chmod() for conforming to `mode':
 * >> if (chmod_mask) {
 * >>     chmod_flags |= (stat(path).st_mode &
 * >>                     chmod_mask);
 * >> }
 * >> chmod(path,chmod_flags);
 */
INTDEF int DCALL
fs_getchmod_mask(DeeObject *__restrict mode,
                 uint16_t *__restrict pchmod_mask,
                 uint16_t *__restrict pchmod_flags);


/* A generic mapping-style sequence that can be used to access environment variables:
 * >> import env from fs;
 * >> print env["PATH"]; // "/bin:/usr/bin"  -- You get the idea... */
INTDEF DeeTypeObject DeeEnv_Type;
INTDEF DeeTypeObject DeeEnvIterator_Type; /* Can be default-constructed. */
/* A singleton instance of `DeeEnv_Type' that is exported from this module as `env'. */
INTDEF DeeObject DeeEnv_Singleton;


/* Path manipulation functions */
INTDEF DREF DeeObject *DCALL fs_pathhead(DeeObject *__restrict path);
INTDEF DREF DeeObject *DCALL fs_pathtail(DeeObject *__restrict path);
INTDEF DREF DeeObject *DCALL fs_pathfile(DeeObject *__restrict path);
INTDEF DREF DeeObject *DCALL fs_pathext(DeeObject *__restrict path);
INTDEF DREF DeeObject *DCALL fs_pathdrive(DeeObject *__restrict path);
INTDEF DREF DeeObject *DCALL fs_pathinctrail(DeeObject *__restrict path);
INTDEF DREF DeeObject *DCALL fs_pathexctrail(DeeObject *__restrict path);
INTDEF DREF DeeObject *DCALL fs_pathabs(DeeObject *__restrict path, DeeObject *pwd);
INTDEF DREF DeeObject *DCALL fs_pathrel(DeeObject *__restrict path, DeeObject *pwd);
INTDEF bool DCALL fs_pathisabs(DeeObject *__restrict path);
INTDEF DREF DeeObject *DCALL fs_pathjoin(size_t pathc, DeeObject **__restrict pathv);

/* @param: options: Set of `FS_EXPAND_F*' */
INTDEF DREF DeeObject *DCALL fs_pathexpand(DeeObject *__restrict path, uint16_t options,
                                           DeeObject *__restrict environ_mapping);
#define FS_EXPAND_FHOME   0x0001 /* `h': Expand a `~' prefix with `fs.user().home()' */
#define FS_EXPAND_FVARS   0x0002 /* `v': Expand `$nam' and `${nam}' with `fs.env[nam]' */
#define FS_EXPAND_FWVARS  0x0004 /* `V': Expand `%nam%' with `fs.env[nam]' */
#define FS_EXPAND_FPATH   0x0008 /* `p': Expand `.' and `..' folders and delete multiple consecutive
                                  *      slashes, as well as whitespace surrounding them. */
#define FS_EXPAND_FABS    0x0010 /* `a': Force the path to become absolute (same as `fs_pathabs(path,NULL)'). */
#define FS_EXPAND_FREL    0x0020 /* `r': Force the path to become relative (same as `fs_pathrel(path,NULL)'). - Overrules `FS_EXPAND_FABS' */
#define FS_EXPAND_FCASE   0x0040 /* `c': Force all parts of the path to share the same casing when the host filesystem is case-insensitive. */
#define FS_EXPAND_FNOFAIL 0x1000 /* `f': Skip expansion when a variable isn't known, or the home path cannot be determined. */
#define EXPAND_DEFAULT_OPTIONS \
    (FS_EXPAND_FHOME|FS_EXPAND_FVARS|FS_EXPAND_FPATH|FS_EXPAND_FNOFAIL)


/* New error classes added for the filesystem. */
INTDEF DeeTypeObject DeeError_NoDirectory;
INTDEF DeeTypeObject DeeError_IsDirectory;
INTDEF DeeTypeObject DeeError_CrossDevice;
INTDEF DeeTypeObject DeeError_NotEmpty;
INTDEF DeeTypeObject DeeError_BusyFile;
INTDEF DeeTypeObject DeeError_NoLink;


/* Imported module access. */
#define TIME_MODULE   DEX.d_imports[0]

/* Construct a new time object from `microseconds' */
INTDEF DREF DeeObject *(DCALL DeeTime_New)(uint64_t microseconds);

DECL_END

#endif /* !GUARD_DEX_FS_LIBFS_H */
