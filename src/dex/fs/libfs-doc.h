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
#ifndef GUARD_DEX_FS_LIBFS_DOC_H
#define GUARD_DEX_FS_LIBFS_DOC_H 1

#include <deemon/api.h>

DECL_BEGIN

/* Resource strings. */
#define DeeUser_home_doc         DOC("->?Dstring\n" \
                                     "@throw SystemError Failed to retrieve the home path\n" \
                                     "Returns the home path of @this user")
#define DeeUser_name_doc         DOC("->?Dstring\n" \
                                     "@throw SystemError Failed to retrieve the name\n" \
                                     "Returns the name of @this user")
#define DeeUser_static_home_doc  DOC("->?Dstring\n" \
                                     "@throw SystemError Failed to retrieve the home path of the current user\n" \
                                     "Returns the home path of the current user (same as ${fs.user().home})")
#define DeeUser_static_name_doc  DOC("->?Dstring\n" \
                                     "@throw SystemError Failed to retrieve the name of the current user\n" \
                                     "Returns the name of the current user (same as ${fs.user().name})")

#define DeeStat_st_dev_doc       DOC("->?Dint\n" \
                                     "@throw ValueError @this stat-file does not contain valid device information\n" \
                                     "Return the device number of the storage device on which the stat-file is located")
#define DeeStat_st_ino_doc       DOC("->?Dint\n" \
                                     "@throw ValueError @this stat-file does not contain valid inode information\n" \
                                     "Returns the inode number or file-id of the stat-file")
#define DeeStat_st_mode_doc      DOC("->?Dint\n" \
                                     "Returns a bitset describing the access permissions and mode of the stat-file. " \
                                     "For more information, see :S_IFMT")
#define DeeStat_st_nlink_doc     DOC("->?Dint\n" \
                                     "Returns the number of existing hard-links to this stat-file")
#define DeeStat_st_uid_doc       DOC("->user\n" \
                                     "Returns a descriptor for the user owning this file")
#define DeeStat_st_gid_doc       DOC("->group\n" \
                                     "Returns a descriptor for the group owning this file")
#define DeeStat_st_rdev_doc      DOC("->?Dint\n" \
                                     "@throw ValueError @this stat-file does not contain valid r-dev information\n" \
                                     "Returns the device ID of the character/block device described by this stat-file")
#define DeeStat_st_size_doc      DOC("->?Dint\n" \
                                     "@throw ValueError @this stat-file does not contain valid size information\n" \
                                     "Returns the size of the stat-file in bytes")
#define DeeStat_st_atime_doc     DOC("->time\n" \
                                     "@throw ValueError @this stat-file does not contain valid time information\n" \
                                     "Return the last-accessed time of the stat-file")
#define DeeStat_st_mtime_doc     DOC("->time\n" \
                                     "@throw ValueError @this stat-file does not contain valid time information\n" \
                                     "Return the last-modified time of the stat-file")
#define DeeStat_st_ctime_doc     DOC("->time\n" \
                                     "@throw ValueError @this stat-file does not contain valid time information\n" \
                                     "Return the creation time of the stat-file")

#define DeeStat_isdir_doc          DOC("->?Dbool\n" \
                                       "@interrupt\n" \
                                       "Check if @this stat-file refers to a directory")
#define DeeStat_ischr_doc          DOC("->?Dbool\n" \
                                       "@interrupt\n" \
                                       "Check if @this stat-file refers to a character device")
#define DeeStat_isblk_doc          DOC("->?Dbool\n" \
                                       "@interrupt\n" \
                                       "Check if @this stat-file refers to a block device")
#define DeeStat_isreg_doc          DOC("->?Dbool\n" \
                                       "@interrupt\n" \
                                       "Check if @this stat-file refers to a regular file")
#define DeeStat_isfifo_doc         DOC("->?Dbool\n" \
                                       "@interrupt\n" \
                                       "Check if @this stat-file refers to a pipe")
#define DeeStat_islnk_doc          DOC("->?Dbool\n" \
                                       "@interrupt\n" \
                                       "Check if @this stat-file refers to a symbolic link")
#define DeeStat_issock_doc         DOC("->?Dbool\n" \
                                       "@interrupt\n" \
                                       "Check if @this stat-file refers to a socket")
#define DeeStat_class_exists_doc   DOC("(path:?Dstring)->?Dbool\n" \
                                       "(fp:?Dfile)->?Dbool\n" \
                                       "(fd:?Dint)->?Dbool\n" \
                                       "@interrupt\n" \
                                       "Taking the same arguments as the constructor of :stat, " \
                                       "check if the referred file exists, or if the given file described " \
                                       "can be used with :stat")
#define DeeStat_class_isdir_doc    DOC("(path:?Dstring)->?Dbool\n" \
                                       "(fp:?Dfile)->?Dbool\n" \
                                       "(fd:?Dint)->?Dbool\n" \
                                       "@interrupt\n" \
                                       "Taking the same arguments as the constructor of :stat, " \
                                       "check if the passed parameters refer to an existing directory")
#define DeeStat_class_ischr_doc    DOC("(path:?Dstring)->?Dbool\n" \
                                       "(fp:?Dfile)->?Dbool\n" \
                                       "(fd:?Dint)->?Dbool\n" \
                                       "@interrupt\n" \
                                       "Taking the same arguments as the constructor of :stat, " \
                                       "check if the passed parameters refer to an existing character device")
#define DeeStat_class_isblk_doc    DOC("(path:?Dstring)->?Dbool\n" \
                                       "(fp:?Dfile)->?Dbool\n" \
                                       "(fd:?Dint)->?Dbool\n" \
                                       "@interrupt\n" \
                                       "Taking the same arguments as the constructor of :stat, " \
                                       "check if the passed parameters refer to an existing block device")
#define DeeStat_class_isreg_doc    DOC("(path:?Dstring)->?Dbool\n" \
                                       "(fp:?Dfile)->?Dbool\n" \
                                       "(fd:?Dint)->?Dbool\n" \
                                       "@interrupt\n" \
                                       "Taking the same arguments as the constructor of :stat, " \
                                       "check if the passed parameters refer to an existing regular file")
#define DeeStat_class_isfifo_doc   DOC("(path:?Dstring)->?Dbool\n" \
                                       "(fp:?Dfile)->?Dbool\n" \
                                       "(fd:?Dint)->?Dbool\n" \
                                       "@interrupt\n" \
                                       "Taking the same arguments as the constructor of :stat, " \
                                       "check if the passed parameters refer to an existing pipe")
#define DeeStat_class_islnk_doc    DOC("(path:?Dstring)->?Dbool\n" \
                                       "(fp:?Dfile)->?Dbool\n" \
                                       "(fd:?Dint)->?Dbool\n" \
                                       "@interrupt\n" \
                                       "Taking the same arguments as the constructor of :stat, " \
                                       "check if the passed parameters refer to an existing symbolic link")
#define DeeStat_class_issock_doc   DOC("(path:?Dstring)->?Dbool\n" \
                                       "(fp:?Dfile)->?Dbool\n" \
                                       "(fd:?Dint)->?Dbool\n" \
                                       "@interrupt\n" \
                                       "Taking the same arguments as the constructor of :stat, " \
                                       "check if the passed parameters refer to an existing socket")
#define DeeStat_class_ishidden_doc DOC("(path:?Dstring)->?Dbool\n" \
                                       "(fp:?Dfile)->?Dbool\n" \
                                       "(fd:?Dint)->?Dbool\n" \
                                       "@interrupt\n" \
                                       "Taking the same arguments as the constructor of " \
                                       ":stat, check if the passed parameters refer to a hidden file. " \
                                       "If the filesystem encodes the hidden-attribute as part of the " \
                                       "filename, this function always returns :false if the path-string " \
                                       "of the file described by the passed arguments cannot be determined")
#define DeeStat_class_isexe_doc    DOC("(path:?Dstring)->?Dbool\n" \
                                       "(fp:?Dfile)->?Dbool\n" \
                                       "(fd:?Dint)->?Dbool\n" \
                                       "@interrupt\n" \
                                       "Taking the same arguments as the constructor of " \
                                       ":stat, check if the passed parameters refer to an executable file. " \
                                       "If the filesystem encodes the executable-attribute as part of the " \
                                       "filename, this function always returns :false if the path-string " \
                                       "of the file described by the passed arguments cannot be determined")
#define DeeStat_TP_DOC             DOC("(path:?Dstring)\n" \
                                       "(fp:?Dfile)\n" \
                                       "(fd:?Dint)\n" \
                                       "@interrupt\n" \
                                       "@throw FileNotFound The given @path or @fp could not be found\n" \
                                       "@throw SystemError Failed to query file information for some reason\n" \
                                       "Query information on a given @path, file stream @fp " \
                                       "or file descriptor @fd (if supported by the host)\n" \
                                       "If you wish to test the existing and type of a type, " \
                                       "consider using stat's class methods such as #{isdir}. " \
                                       "Note however that stat instances also implement these " \
                                       "methods as general purpose property checks that do not " \
                                       "require calculation of #st_mode")
#define DeeLStat_TP_DOC            DOC("(path:?Dstring)\n" \
                                       "(fp:?Dfile)\n" \
                                       "(fd:?Dint)\n" \
                                       "@interrupt\n" \
                                       "@throw FileNotFound The given @path or @fp could not be found\n" \
                                       "@throw SystemError Failed to query file information for some reason\n" \
                                       "Same as its base type :stat, but query information without dereferencing the final link")
#define DeeDirIterator_TP_DOC      DOC("Construct a directory iteration descriptor that yields the names " \
                                       "of all filesystem objects found within a the associated directory")
#define DeeDir_TP_DOC              DOC("(path:?Dstring)\n" \
                                       "(fp:?Dfile)\n" \
                                       "(fd:?Dint)\n" \
                                       "@interrupt\n" \
                                       "Construct a sequence for enumerating the contents of a directory")
#define DeeQueryIterator_TP_DOC    DOC("Construct a directory iteration descriptor that yields the names " \
                                       "of all filesystem objects found within a the associated directory")
#define DeeQuery_TP_DOC            DOC("(pattern:?Dstring)\n" \
                                       "@interrupt\n" \
                                       "Construct a directory query that yields the names of all filesystem " \
                                       "objects addressed by the given wildcard-enabled @pattern string")


DECL_END

#endif /* !GUARD_DEX_FS_LIBFS_DOC_H */
