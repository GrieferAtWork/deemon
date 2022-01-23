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
#ifndef GUARD_DEX_FS__RES_H
#define GUARD_DEX_FS__RES_H 1

#include <deemon/api.h>

DECL_BEGIN

/* Resource strings. */

#define S_EnvIterator_tp_name "_EnvIterator"
#define S_EnvIterator_tp_doc NULL

#define S_User_tp_name "User"
#define S_User_tp_doc NULL
#define S_User_getset_home_name "home"
#define S_User_getset_home_doc                                  \
	DOC("->?Dstring\n"                                          \
	    "@throw SystemError Failed to retrieve the home path\n" \
	    "Returns the home path of @this user")
#define S_User_getset_name_name "name"
#define S_User_getset_name_doc                             \
	DOC("->?Dstring\n"                                     \
	    "@throw SystemError Failed to retrieve the name\n" \
	    "Returns the name of @this user")
#define S_User_class_getset_home_name "home"
#define S_User_class_getset_home_doc                                                \
	DOC("->?Dstring\n"                                                              \
	    "@throw SystemError Failed to retrieve the home path of the current user\n" \
	    "Returns the home path of the current user (same as ${fs.user().home})")
#define S_User_class_getset_name_name "name"
#define S_User_class_getset_name_doc                                           \
	DOC("->?Dstring\n"                                                         \
	    "@throw SystemError Failed to retrieve the name of the current user\n" \
	    "Returns the name of the current user (same as ${fs.user().name})")


#define S_Stat_tp_name "stat"
#define S_Stat_tp_doc                                                           \
	DOC("(path:?Dstring)\n"                                                     \
	    "(fp:?DFile)\n"                                                         \
	    "(fd:?Dint)\n"                                                          \
	    "(dfd:?Dint,path:?Dstring,atflags=!0)\n"                                \
	    "@interrupt\n"                                                          \
	    "@throw FileNotFound The given @path or @fp could not be found\n"       \
	    "@throw SystemError Failed to query file information for some reason\n" \
	    "Query information on a given @path, file stream @fp "                  \
	    "or file descriptor @fd (if supported by the host)\n"                   \
	    "If you wish to test the existing and type of a type, "                 \
	    "consider using stat's class methods such as ?#{isdir}. "               \
	    "Note however that stat instances also implement these "                \
	    "methods as general purpose property checks that do not "               \
	    "require calculation of ?#st_mode")
#define S_LStat_tp_name "lstat"
#define S_LStat_tp_doc                                                          \
	DOC("(path:?Dstring)\n"                                                     \
	    "(fp:?DFile)\n"                                                         \
	    "(fd:?Dint)\n"                                                          \
	    "(dfd:?Dint,path:?Dstring,atflags=!0)\n"                                \
	    "@interrupt\n"                                                          \
	    "@throw FileNotFound The given @path or @fp could not be found\n"       \
	    "@throw SystemError Failed to query file information for some reason\n" \
	    "Same as its base type ?Gstat, but query information without "          \
	    "dereferencing the final link")
#define S_Stat_getset_st_dev_name "st_dev"
#define S_Stat_getset_st_dev_doc                                                        \
	DOC("->?Dint\n"                                                                     \
	    "@throw ValueError @this stat-file does not contain valid device information\n" \
	    "Return the device number of the storage device on which the stat-file is located")
#define S_Stat_getset_st_ino_name "st_ino"
#define S_Stat_getset_st_ino_doc                                                       \
	DOC("->?Dint\n"                                                                    \
	    "@throw ValueError @this stat-file does not contain valid inode information\n" \
	    "Returns the inode number or file-id of the stat-file")
#define S_Stat_getset_st_mode_name "st_mode"
#define S_Stat_getset_st_mode_doc                                                        \
	DOC("->?Dint\n"                                                                      \
	    "Returns a bitset describing the access permissions and mode of the stat-file. " \
	    "For more information, see ?GS_IFMT")
#define S_Stat_getset_st_nlink_name "st_nlink"
#define S_Stat_getset_st_nlink_doc \
	DOC("->?Dint\n"                \
	    "Returns the number of existing hard-links to this stat-file")
#define S_Stat_getset_st_uid_name "st_uid"
#define S_Stat_getset_st_uid_doc \
	DOC("->?Guser\n"             \
	    "Returns a descriptor for the user owning this file")
#define S_Stat_getset_st_gid_name "st_gid"
#define S_Stat_getset_st_gid_doc \
	DOC("->?Ggroup\n"            \
	    "Returns a descriptor for the group owning this file")
#define S_Stat_getset_st_rdev_name "st_rdev"
#define S_Stat_getset_st_rdev_doc                                                      \
	DOC("->?Dint\n"                                                                    \
	    "@throw ValueError @this stat-file does not contain valid r-dev information\n" \
	    "Returns the device ID of the character/block device described by this stat-file")
#define S_Stat_getset_st_size_name "st_size"
#define S_Stat_getset_st_size_doc                                                     \
	DOC("->?Dint\n"                                                                   \
	    "@throw ValueError @this stat-file does not contain valid size information\n" \
	    "Returns the size of the stat-file in bytes")
#define S_Stat_getset_st_atime_name "st_atime"
#define S_Stat_getset_st_atime_doc                                                    \
	DOC("->time\n"                                                                    \
	    "@throw ValueError @this stat-file does not contain valid time information\n" \
	    "Return the last-accessed time of the stat-file")
#define S_Stat_getset_st_mtime_name "st_mtime"
#define S_Stat_getset_st_mtime_doc                                                    \
	DOC("->time\n"                                                                    \
	    "@throw ValueError @this stat-file does not contain valid time information\n" \
	    "Return the last-modified time of the stat-file")
#define S_Stat_getset_st_ctime_name "st_ctime"
#define S_Stat_getset_st_ctime_doc                                                    \
	DOC("->time\n"                                                                    \
	    "@throw ValueError @this stat-file does not contain valid time information\n" \
	    "Return the creation time of the stat-file")

#define S_Stat_getset_isdir_name "isdir"
#define S_Stat_getset_isdir_doc \
	DOC("->?Dbool\n"            \
	    "@interrupt\n"          \
	    "Check if @this stat-file refers to a directory")
#define S_Stat_getset_ischr_name "ischr"
#define S_Stat_getset_ischr_doc \
	DOC("->?Dbool\n"            \
	    "@interrupt\n"          \
	    "Check if @this stat-file refers to a character device")
#define S_Stat_getset_isblk_name "isblk"
#define S_Stat_getset_isblk_doc \
	DOC("->?Dbool\n"            \
	    "@interrupt\n"          \
	    "Check if @this stat-file refers to a block device")
#define S_Stat_getset_isreg_name "isreg"
#define S_Stat_getset_isreg_doc \
	DOC("->?Dbool\n"            \
	    "@interrupt\n"          \
	    "Check if @this stat-file refers to a regular file")
#define S_Stat_getset_isfifo_name "isfifo"
#define S_Stat_getset_isfifo_doc \
	DOC("->?Dbool\n"             \
	    "@interrupt\n"           \
	    "Check if @this stat-file refers to a pipe")
#define S_Stat_getset_islnk_name "islnk"
#define S_Stat_getset_islnk_doc \
	DOC("->?Dbool\n"            \
	    "@interrupt\n"          \
	    "Check if @this stat-file refers to a symbolic link")
#define S_Stat_getset_issock_name "issock"
#define S_Stat_getset_issock_doc \
	DOC("->?Dbool\n"             \
	    "@interrupt\n"           \
	    "Check if @this stat-file refers to a socket")
#define S_Stat_class_function_exists_name "exists"
#define S_Stat_class_function_exists_doc                                     \
	DOC("(path:?Dstring)->?Dbool\n"                                          \
	    "(fp:?DFile)->?Dbool\n"                                              \
	    "(fd:?Dint)->?Dbool\n"                                               \
	    "(dfd:?Dint,path:?Dstring,atflags=!0)\n"                             \
	    "@interrupt\n"                                                       \
	    "Taking the same arguments as the constructor of ?Gstat, "           \
	    "check if the referred file exists, or if the given file described " \
	    "can be used with ?Gstat")
#define S_Stat_class_function_isdir_name "isdir"
#define S_Stat_class_function_isdir_doc                            \
	DOC("(path:?Dstring)->?Dbool\n"                                \
	    "(fp:?DFile)->?Dbool\n"                                    \
	    "(fd:?Dint)->?Dbool\n"                                     \
	    "(dfd:?Dint,path:?Dstring,atflags=!0)\n"                   \
	    "@interrupt\n"                                             \
	    "Taking the same arguments as the constructor of ?Gstat, " \
	    "check if the passed parameters refer to an existing directory")
#define S_Stat_class_function_ischr_name "ischr"
#define S_Stat_class_function_ischr_doc                            \
	DOC("(path:?Dstring)->?Dbool\n"                                \
	    "(fp:?DFile)->?Dbool\n"                                    \
	    "(fd:?Dint)->?Dbool\n"                                     \
	    "(dfd:?Dint,path:?Dstring,atflags=!0)\n"                   \
	    "@interrupt\n"                                             \
	    "Taking the same arguments as the constructor of ?Gstat, " \
	    "check if the passed parameters refer to an existing character device")
#define S_Stat_class_function_isblk_name "isblk"
#define S_Stat_class_function_isblk_doc                            \
	DOC("(path:?Dstring)->?Dbool\n"                                \
	    "(fp:?DFile)->?Dbool\n"                                    \
	    "(fd:?Dint)->?Dbool\n"                                     \
	    "(dfd:?Dint,path:?Dstring,atflags=!0)\n"                   \
	    "@interrupt\n"                                             \
	    "Taking the same arguments as the constructor of ?Gstat, " \
	    "check if the passed parameters refer to an existing block device")
#define S_Stat_class_function_isreg_name "isreg"
#define S_Stat_class_function_isreg_doc                            \
	DOC("(path:?Dstring)->?Dbool\n"                                \
	    "(fp:?DFile)->?Dbool\n"                                    \
	    "(fd:?Dint)->?Dbool\n"                                     \
	    "(dfd:?Dint,path:?Dstring,atflags=!0)\n"                   \
	    "@interrupt\n"                                             \
	    "Taking the same arguments as the constructor of ?Gstat, " \
	    "check if the passed parameters refer to an existing regular file")
#define S_Stat_class_function_isfifo_name "isfifo"
#define S_Stat_class_function_isfifo_doc                           \
	DOC("(path:?Dstring)->?Dbool\n"                                \
	    "(fp:?DFile)->?Dbool\n"                                    \
	    "(fd:?Dint)->?Dbool\n"                                     \
	    "(dfd:?Dint,path:?Dstring,atflags=!0)\n"                   \
	    "@interrupt\n"                                             \
	    "Taking the same arguments as the constructor of ?Gstat, " \
	    "check if the passed parameters refer to an existing pipe")
#define S_Stat_class_function_islnk_name "islnk"
#define S_Stat_class_function_islnk_doc                            \
	DOC("(path:?Dstring)->?Dbool\n"                                \
	    "(fp:?DFile)->?Dbool\n"                                    \
	    "(fd:?Dint)->?Dbool\n"                                     \
	    "(dfd:?Dint,path:?Dstring,atflags=!0)\n"                   \
	    "@interrupt\n"                                             \
	    "Taking the same arguments as the constructor of ?Gstat, " \
	    "check if the passed parameters refer to an existing symbolic link")
#define S_Stat_class_function_issock_name "issock"
#define S_Stat_class_function_issock_doc                           \
	DOC("(path:?Dstring)->?Dbool\n"                                \
	    "(fp:?DFile)->?Dbool\n"                                    \
	    "(fd:?Dint)->?Dbool\n"                                     \
	    "(dfd:?Dint,path:?Dstring,atflags=!0)\n"                   \
	    "@interrupt\n"                                             \
	    "Taking the same arguments as the constructor of ?Gstat, " \
	    "check if the passed parameters refer to an existing socket")
#define S_Stat_class_function_ishidden_name "ishidden"
#define S_Stat_class_function_ishidden_doc                               \
	DOC("(path:?Dstring)->?Dbool\n"                                      \
	    "(fp:?DFile)->?Dbool\n"                                          \
	    "(fd:?Dint)->?Dbool\n"                                           \
	    "(dfd:?Dint,path:?Dstring,atflags=!0)\n"                         \
	    "@interrupt\n"                                                   \
	    "Taking the same arguments as the constructor of "               \
	    ":stat, check if the passed parameters refer to a hidden file. " \
	    "If the filesystem encodes the hidden-attribute as part of the " \
	    "filename, this function always returns ?f if the path-string "  \
	    "of the file described by the passed arguments cannot be determined")
#define S_Stat_class_function_isexe_name "isexe"
#define S_Stat_class_function_isexe_doc                                      \
	DOC("(path:?Dstring)->?Dbool\n"                                          \
	    "(fp:?DFile)->?Dbool\n"                                              \
	    "(fd:?Dint)->?Dbool\n"                                               \
	    "(dfd:?Dint,path:?Dstring,atflags=!0)\n"                             \
	    "@interrupt\n"                                                       \
	    "Taking the same arguments as the constructor of ?Gstat, "           \
	    "check if the passed parameters refer to an executable file. "       \
	    "If the filesystem encodes the executable-attribute as part of the " \
	    "filename, this function always returns ?f if the path-string "      \
	    "of the file described by the passed arguments cannot be determined")


#define S_DirIterator_tp_name "_DirIterator"
#define S_DirIterator_tp_doc                                                \
	DOC("Construct a directory iteration descriptor that yields the names " \
	    "of all filesystem objects found within a the associated directory")

#define S_Dir_tp_name "_Dir"
#define S_Dir_tp_doc        \
	DOC("(path:?Dstring)\n" \
	    "(fp:?DFile)\n"     \
	    "(fd:?Dint)\n"      \
	    "@interrupt\n"      \
	    "Construct a sequence for enumerating the contents of a directory")

#define S_QueryIterator_tp_name "_QueryIterator"
#define S_QueryIterator_tp_doc                                              \
	DOC("Construct a directory iteration descriptor that yields the names " \
	    "of all filesystem objects found within a the associated directory")

#define S_Query_tp_name "_Query"
#define S_Query_tp_doc                                                         \
	DOC("(pattern:?Dstring)\n"                                                 \
	    "@interrupt\n"                                                         \
	    "Construct a directory query that yields the names of all filesystem " \
	    "objects addressed by the given wildcard-enabled @pattern string")


DECL_END

#endif /* !GUARD_DEX_FS__RES_H */
