/* Copyright (c) 2019 Griefer@Work                                            *
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
#ifndef GUARD_DEX_FS_GENERIC_C_INL
#define GUARD_DEX_FS_GENERIC_C_INL 1
#define DEE_SOURCE 1

#include "libfs.h"
/**/

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/error.h>
#include <deemon/seq.h>

DECL_BEGIN

PRIVATE char const fs_unsupported_message[] = "A filesystem is not supported";

PRIVATE ATTR_NOINLINE int DCALL fs_unsupported(void) {
	return DeeError_Throwf(&DeeError_UnsupportedAPI,
	                       fs_unsupported_message);
}

PRIVATE int DCALL
env_init(DeeObject *__restrict UNUSED(self)) {
	return fs_unsupported();
}

PRIVATE int DCALL
env_bool(DeeObject *__restrict UNUSED(self)) {
	return fs_unsupported();
}

PRIVATE WUNUSED DREF DeeObject *DCALL
env_next(DeeObject *__restrict UNUSED(self)) {
	return ITER_DONE;
}

INTERN WUNUSED DREF DeeObject *DCALL
enviterator_next_key(DeeObject *__restrict UNUSED(self)) {
	return ITER_DONE;
}

INTERN WUNUSED DREF DeeObject *DCALL
enviterator_next_value(DeeObject *__restrict UNUSED(self)) {
	return ITER_DONE;
}

PRIVATE struct type_member env_members[] = {
	TYPE_MEMBER_CONST("seq", &DeeEnv_Singleton),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject DeeEnvIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "env.Iterator",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (void *)&env_init,
				/* .tp_copy_ctor = */ NULL,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ NULL,
				TYPE_FIXED_ALLOCATOR_S(DeeObject)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ &env_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ &env_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ env_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

INTERN bool DCALL
fs_hasenv(/*String*/ DeeObject *__restrict UNUSED(name)) {
	return false;
}

INTERN WUNUSED DREF DeeObject *DCALL
fs_getenv(DeeObject *__restrict UNUSED(name), bool try_get) {
	if (!try_get)
		fs_unsupported();
	return NULL;
}

INTERN int DCALL
fs_printenv(char const *__restrict UNUSED(name),
            struct unicode_printer *__restrict UNUSED(printer),
            bool try_get) {
	if (try_get)
		return 1;
	return fs_unsupported();
}

INTERN int DCALL
fs_delenv(DeeObject *__restrict UNUSED(name)) {
	return -1;
}

INTERN int DCALL
fs_setenv(DeeObject *__restrict UNUSED(name),
          DeeObject *__restrict UNUSED(value)) {
	return fs_unsupported();
}

INTERN WUNUSED DREF /*String*/ DeeObject *DCALL fs_gethostname(void) {
	return fs_getcwd();
}

INTERN WUNUSED DREF DeeObject *DCALL fs_gettmp(void) {
	return fs_getcwd();
}

INTERN int DCALL
fs_printcwd(struct unicode_printer *__restrict UNUSED(printer)) {
	return fs_unsupported();
}

INTERN WUNUSED DREF DeeObject *DCALL fs_getcwd(void) {
	fs_unsupported();
	return NULL;
}

INTERN int DCALL
fs_chdir(DeeObject *__restrict UNUSED(path)) {
	return fs_unsupported();
}

#ifndef __INTELLISENSE__
#include "generic-user.c.inl"
#endif /* !__INTELLISENSE__ */

PRIVATE int DCALL
stat_ctor(DeeObject *__restrict UNUSED(self),
          size_t UNUSED(argc),
          DeeObject **__restrict UNUSED(argv)) {
	return fs_unsupported();
}

PRIVATE WUNUSED DREF DeeObject *DCALL
stat_getxxx(DeeObject *__restrict UNUSED(self)) {
	return fs_getcwd();
}

PRIVATE struct type_getset stat_getsets[] = {
	{ "st_dev", &stat_getxxx, NULL, NULL, DeeStat_st_dev_doc },
	{ "st_ino", &stat_getxxx, NULL, NULL, DeeStat_st_ino_doc },
	{ "st_mode", &stat_getxxx, NULL, NULL, DeeStat_st_mode_doc },
	{ "st_nlink", &stat_getxxx, NULL, NULL, DeeStat_st_nlink_doc },
	{ "st_uid", &stat_getxxx, NULL, NULL, DeeStat_st_uid_doc },
	{ "st_gid", &stat_getxxx, NULL, NULL, DeeStat_st_gid_doc },
	{ "st_rdev", &stat_getxxx, NULL, NULL, DeeStat_st_rdev_doc },
	{ "st_size", &stat_getxxx, NULL, NULL, DeeStat_st_size_doc },
	{ "st_atime", &stat_getxxx, NULL, NULL, DeeStat_st_atime_doc },
	{ "st_mtime", &stat_getxxx, NULL, NULL, DeeStat_st_mtime_doc },
	{ "st_ctime", &stat_getxxx, NULL, NULL, DeeStat_st_ctime_doc },
	{ NULL }
};

PRIVATE WUNUSED DREF DeeObject *DCALL
stat_isxxx(DeeObject *__restrict UNUSED(self),
           size_t UNUSED(argc),
           DeeObject **__restrict UNUSED(argv)) {
	return fs_getcwd();
}

PRIVATE struct type_method stat_methods[] = {
	{ "isdir", &stat_isxxx, DeeStat_isdir_doc },
	{ "ischr", &stat_isxxx, DeeStat_ischr_doc },
	{ "isblk", &stat_isxxx, DeeStat_isblk_doc },
	{ "isreg", &stat_isxxx, DeeStat_isreg_doc },
	{ "isfifo", &stat_isxxx, DeeStat_isfifo_doc },
	{ "islnk", &stat_isxxx, DeeStat_islnk_doc },
	{ "issock", &stat_isxxx, DeeStat_issock_doc },
	{ NULL }
};

#define DEFINE_STATIC_QUERY(funnam, name, return_)       \
	PRIVATE WUNUSED DREF DeeObject *DCALL                        \
	funnam(DeeObject *__restrict UNUSED(self),           \
	       size_t argc, DeeObject **argv) {   \
		DeeObject *path;                                 \
		if (DeeArg_Unpack(argc, argv, "o:" name, &path)) \
			return NULL;                                 \
		return_;                                         \
	}
DEFINE_STATIC_QUERY(stat_class_exists, "exists", return_false)
DEFINE_STATIC_QUERY(stat_class_isdir, "isdir", return_false)
DEFINE_STATIC_QUERY(stat_class_ischr, "ischr", return_false)
DEFINE_STATIC_QUERY(stat_class_isblk, "isblk", return_false)
DEFINE_STATIC_QUERY(stat_class_isreg, "isreg", return_false)
DEFINE_STATIC_QUERY(stat_class_isfifo, "isfifo", return_false)
DEFINE_STATIC_QUERY(stat_class_islnk, "islnk", return_false)
DEFINE_STATIC_QUERY(stat_class_issock, "issock", return_false)
DEFINE_STATIC_QUERY(stat_class_ishidden, "ishidden", return_false)
DEFINE_STATIC_QUERY(stat_class_isexe, "isexe", return_false)
#undef DEFINE_STATIC_QUERY


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
	/* .tp_name     = */ "stat",
	/* .tp_doc      = */ DeeStat_TP_DOC,
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
				/* .tp_any_ctor  = */ (void *)&stat_ctor,
				TYPE_FIXED_ALLOCATOR_S(DeeObject)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ stat_methods,
	/* .tp_getsets       = */ stat_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ stat_class_methods,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

INTERN DeeTypeObject DeeLStat_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "lstat",
	/* .tp_doc      = */ DeeLStat_TP_DOC,
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeStat_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ NULL,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ (void *)&stat_ctor,
				TYPE_FIXED_ALLOCATOR_S(DeeObject)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};


/* Filesystem write operations. */
INTERN int DCALL
fs_chtime(DeeObject *__restrict UNUSED(path),
          DeeObject *__restrict UNUSED(atime),
          DeeObject *__restrict UNUSED(mtime),
          DeeObject *__restrict UNUSED(ctime)) {
	return fs_unsupported();
}

INTERN int DCALL
fs_chmod(DeeObject *__restrict UNUSED(path),
         DeeObject *__restrict UNUSED(mode)) {
	return fs_unsupported();
}

INTERN int DCALL
fs_chown(DeeObject *__restrict UNUSED(path),
         DeeObject *__restrict UNUSED(user),
         DeeObject *__restrict UNUSED(group)) {
	return fs_unsupported();
}

INTERN int DCALL
fs_mkdir(DeeObject *__restrict UNUSED(path),
         DeeObject *__restrict UNUSED(perm)) {
	return fs_unsupported();
}

INTERN int DCALL
fs_rmdir(DeeObject *__restrict UNUSED(path)) {
	return fs_unsupported();
}

INTERN int DCALL
fs_unlink(DeeObject *__restrict UNUSED(path)) {
	return fs_unsupported();
}

INTERN int DCALL
fs_remove(DeeObject *__restrict UNUSED(path)) {
	return fs_unsupported();
}

INTERN int DCALL
fs_rename(DeeObject *__restrict UNUSED(existing_path),
          DeeObject *__restrict UNUSED(new_path)) {
	return fs_unsupported();
}

INTERN int DCALL
fs_link(DeeObject *__restrict UNUSED(existing_path),
        DeeObject *__restrict UNUSED(new_path)) {
	return fs_unsupported();
}

INTERN int DCALL
fs_symlink(DeeObject *__restrict UNUSED(target_text),
           DeeObject *__restrict UNUSED(link_path),
           bool UNUSED(format_target)) {
	return fs_unsupported();
}

INTERN WUNUSED DREF DeeObject *DCALL
fs_readlink(DeeObject *__restrict UNUSED(path)) {
	return fs_getcwd();
}

#ifndef __INTELLISENSE__
#include "generic-dir.c.inl"
#endif

DECL_END

#endif /* !GUARD_DEX_FS_GENERIC_C_INL */
