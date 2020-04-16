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
#ifndef GUARD_DEX_FS_STDIO_C_INL
#define GUARD_DEX_FS_STDIO_C_INL 1
#define DEE_SOURCE 1

#include "libfs.h"
/**/

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/error.h>
#include <deemon/file.h>
#include <deemon/int.h>
#include <deemon/seq.h>
#include <deemon/thread.h>

#include "_res.h"

/* Everything we can do originates from these 2 files. */
#include <stdio.h>
#include <stdlib.h>

#ifndef __INTELLISENSE__
/* Pull in definitions for an stdlib-style environ. */
#include "environ.c.inl"
#endif /* !__INTELLISENSE__ */

DECL_BEGIN

PRIVATE char const fs_unsupported_message[] = "This functionality is not supported by the host filesystem";

PRIVATE ATTR_NOINLINE int DCALL fs_unsupported(void) {
	return DeeError_Throwf(&DeeError_UnsupportedAPI,
	                       fs_unsupported_message);
}

INTERN WUNUSED DREF /*String*/ DeeObject *DCALL fs_gethostname(void) {
	fs_unsupported();
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL fs_gettmp(void) {
	return fs_gethostname();
}

INTERN int DCALL
fs_chdir(DeeObject *__restrict UNUSED(path)) {
	return fs_unsupported();
}

#include "generic-user.c.inl"


PRIVATE WUNUSED NONNULL((1)) int DCALL
file_exists_str(char const *__restrict filename) {
	FILE *fp;
	DBG_ALIGNMENT_DISABLE();
	fp = fopen(filename, "r");
	if (!fp) {
		DBG_ALIGNMENT_ENABLE();
		return 0;
	}
	fclose(fp);
	DBG_ALIGNMENT_ENABLE();
	return 1;
}


/* @return: -1: An error occurred.
 * @return:  0: The file does not exist.
 * @return:  1: The file does exist. */
PRIVATE WUNUSED NONNULL((1)) int DCALL
file_exists(DeeObject *__restrict filename) {
	int result;
	if (DeeString_Check(filename))
		return file_exists_str(DeeString_STR(filename));
	filename = DeeFile_Filename(filename);
	if unlikely(!filename)
		return -1;
	result = file_exists_str(DeeString_STR(filename));
	Dee_Decref(filename);
	return result;
}

PRIVATE FILE *DCALL
file_open(DeeObject *__restrict filename, char const *mode) {
	FILE *result;
	char const *name;
	if (DeeString_Check(filename)) {
		Dee_Incref(filename);
	} else {
		filename = DeeFile_Filename(filename);
		if unlikely(!filename)
			goto err;
	}
	name = DeeString_AsUtf8(filename);
	if unlikely(!name)
		goto err_filename;
	DBG_ALIGNMENT_DISABLE();
	result = fopen(name, mode);
	DBG_ALIGNMENT_ENABLE();
	if (!result) {
		DeeError_Throwf(&DeeError_FileNotFound,
		                "File %r could not be found",
		                filename);
	}
	Dee_Decref(filename);
	return result;
err_filename:
	Dee_Decref(filename);
err:
	return NULL;
}


typedef struct {
	OBJECT_HEAD
	FILE *st_file; /* [1..1][const][owned] File file that is being stat-ed. */
} Stat;

PRIVATE WUNUSED NONNULL((1)) int DCALL
stat_ctor(Stat *__restrict self,
          size_t argc, DeeObject *const *argv) {
	DeeObject *path;
	int error;
	if (DeeArg_Unpack(argc, argv, "o:" S_Stat_tp_name, &path))
		goto err;
	self->st_file = file_open(path, "r");
	if unlikely(!self->st_file)
		goto err;
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
stat_fini(Stat *__restrict self) {
	DBG_ALIGNMENT_DISABLE();
	fclose(self->st_file);
	DBG_ALIGNMENT_ENABLE();
}

PRIVATE WUNUSED DREF DeeObject *DCALL
stat_getxxx(DeeObject *__restrict UNUSED(self)) {
	return fs_gethostname();
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_get_size(Stat *__restrict self) {
	long result;
	DBG_ALIGNMENT_DISABLE();
	if (fseek(self->st_file, 0, SEEK_END))
		goto err;
	result = ftell(self->st_file);
	if (result < 0)
		goto err;
	DBG_ALIGNMENT_ENABLE();
	return DeeInt_NewULong((unsigned long)result);
err:
	DBG_ALIGNMENT_ENABLE();
	DeeError_Throwf(&DeeError_FSError,
	                "Failed to query file size");
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
stat_return_true(DeeObject *__restrict UNUSED(self)) {
	return_true;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
stat_return_false(DeeObject *__restrict UNUSED(self)) {
	return_false;
}

PRIVATE struct type_getset stat_getsets[] = {
	{ S_Stat_getset_st_dev_name, &stat_getxxx, NULL, NULL, S_Stat_getset_st_dev_doc },
	{ S_Stat_getset_st_ino_name, &stat_getxxx, NULL, NULL, S_Stat_getset_st_ino_doc },
	{ S_Stat_getset_st_mode_name, &stat_getxxx, NULL, NULL, S_Stat_getset_st_mode_doc },
	{ S_Stat_getset_st_nlink_name, &stat_getxxx, NULL, NULL, S_Stat_getset_st_nlink_doc },
	{ S_Stat_getset_st_uid_name, &stat_getxxx, NULL, NULL, S_Stat_getset_st_uid_doc },
	{ S_Stat_getset_st_gid_name, &stat_getxxx, NULL, NULL, S_Stat_getset_st_gid_doc },
	{ S_Stat_getset_st_rdev_name, &stat_getxxx, NULL, NULL, S_Stat_getset_st_rdev_doc },
	{ S_Stat_getset_st_size_name, (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&stat_get_size, NULL, NULL, S_Stat_getset_st_size_doc },
	{ S_Stat_getset_st_atime_name, &stat_getxxx, NULL, NULL, S_Stat_getset_st_atime_doc },
	{ S_Stat_getset_st_mtime_name, &stat_getxxx, NULL, NULL, S_Stat_getset_st_mtime_doc },
	{ S_Stat_getset_st_ctime_name, &stat_getxxx, NULL, NULL, S_Stat_getset_st_ctime_doc },
	{ S_Stat_getset_isdir_name, &stat_return_false, NULL, NULL, S_Stat_getset_isdir_doc },
	{ S_Stat_getset_ischr_name, &stat_return_false, NULL, NULL, S_Stat_getset_ischr_doc },
	{ S_Stat_getset_isblk_name, &stat_return_false, NULL, NULL, S_Stat_getset_isblk_doc },
	{ S_Stat_getset_isreg_name, &stat_return_true, NULL, NULL, S_Stat_getset_isreg_doc },
	{ S_Stat_getset_isfifo_name, &stat_return_false, NULL, NULL, S_Stat_getset_isfifo_doc },
	{ S_Stat_getset_islnk_name, &stat_return_false, NULL, NULL, S_Stat_getset_islnk_doc },
	{ S_Stat_getset_issock_name, &stat_return_false, NULL, NULL, S_Stat_getset_issock_doc },
	{ NULL }
};

PRIVATE WUNUSED DREF DeeObject *DCALL
stat_class_exists(DeeObject *__restrict UNUSED(self),
                  size_t argc, DeeObject *const *argv) {
	DeeObject *path;
	int error;
	if (DeeArg_Unpack(argc, argv, "o:" S_Stat_class_function_exists_name, &path))
		goto err;
	error = file_exists(path);
	if unlikely(error < 0)
		goto err;
	return_bool_(error != 0);
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
stat_class_isreg(DeeObject *__restrict UNUSED(self),
                 size_t argc, DeeObject *const *argv) {
	DeeObject *path;
	int error;
	if (DeeArg_Unpack(argc, argv, "o:" S_Stat_class_function_isreg_name, &path))
		goto err;
	error = file_exists(path);
	if unlikely(error < 0)
		goto err;
	return_bool_(error != 0);
err:
	return NULL;
}

#define DEFINE_STATIC_QUERY(funnam, name, return_)       \
	PRIVATE WUNUSED DREF DeeObject *DCALL                \
	funnam(DeeObject *UNUSED(self),                      \
	       size_t argc, DeeObject *const *argv) {        \
		DeeObject *path;                                 \
		if (DeeArg_Unpack(argc, argv, "o:" name, &path)) \
			return NULL;                                 \
		return_;                                         \
	}
DEFINE_STATIC_QUERY(stat_class_isdir, S_Stat_class_function_isdir_name, return_false)
DEFINE_STATIC_QUERY(stat_class_ischr, S_Stat_class_function_ischr_name, return_false)
DEFINE_STATIC_QUERY(stat_class_isblk, S_Stat_class_function_isblk_name, return_false)
DEFINE_STATIC_QUERY(stat_class_isfifo, S_Stat_class_function_isfifo_name, return_false)
DEFINE_STATIC_QUERY(stat_class_islnk, S_Stat_class_function_islnk_name, return_false)
DEFINE_STATIC_QUERY(stat_class_issock, S_Stat_class_function_issock_name, return_false)
DEFINE_STATIC_QUERY(stat_class_ishidden, S_Stat_class_function_ishidden_name, return_false)
DEFINE_STATIC_QUERY(stat_class_isexe, S_Stat_class_function_isexe_name, return_false)
#undef DEFINE_STATIC_QUERY


PRIVATE struct type_method stat_class_methods[] = {
	{ S_Stat_class_function_exists_name, &stat_class_exists, S_Stat_class_function_exists_doc },
	{ S_Stat_class_function_isdir_name, &stat_class_isdir, S_Stat_class_function_isdir_doc },
	{ S_Stat_class_function_ischr_name, &stat_class_ischr, S_Stat_class_function_ischr_doc },
	{ S_Stat_class_function_isblk_name, &stat_class_isblk, S_Stat_class_function_isblk_doc },
	{ S_Stat_class_function_isreg_name, &stat_class_isreg, S_Stat_class_function_isreg_doc },
	{ S_Stat_class_function_isfifo_name, &stat_class_isfifo, S_Stat_class_function_isfifo_doc },
	{ S_Stat_class_function_islnk_name, &stat_class_islnk, S_Stat_class_function_islnk_doc },
	{ S_Stat_class_function_issock_name, &stat_class_issock, S_Stat_class_function_issock_doc },
	{ S_Stat_class_function_ishidden_name, &stat_class_ishidden, S_Stat_class_function_ishidden_doc },
	{ S_Stat_class_function_isexe_name, &stat_class_isexe, S_Stat_class_function_isexe_doc },
	{ NULL }
};

INTERN DeeTypeObject DeeStat_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ S_Stat_tp_name,
	/* .tp_doc      = */ S_Stat_tp_doc,
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
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ stat_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ stat_class_methods,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

INTERN DeeTypeObject DeeLStat_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ S_LStat_tp_name,
	/* .tp_doc      = */ S_LStat_tp_doc,
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
fs_rmdir(DeeObject *__restrict UNUSED(path)) {
	return fs_unsupported();
}

INTERN WUNUSED NONNULL((1)) int DCALL
fs_unlink(DeeObject *__restrict path) {
	return fs_remove(path);
}

INTERN WUNUSED NONNULL((1)) int DCALL
fs_remove(DeeObject *__restrict path) {
	char *name;
	if (!DeeString_Check(path)) {
		int result;
		path = DeeFile_Filename(path);
		if unlikely(!path)
			goto err;
		result = fs_remove(path);
		Dee_Decref(path);
		return result;
	}
	if (DeeThread_CheckInterrupt())
		goto err;
	name = DeeString_AsUtf8(path);
	if unlikely(!name)
		goto err;
	DBG_ALIGNMENT_DISABLE();
	if (remove(name)) {
		DBG_ALIGNMENT_ENABLE();
		DeeError_Throwf(&DeeError_FileNotFound,
		                "File %r could not be found",
		                path);
		goto err;
	}
	DBG_ALIGNMENT_ENABLE();
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
fs_rename(DeeObject *__restrict existing_path,
          DeeObject *__restrict new_path) {
	char *old_name, *new_name;
	if (!DeeString_Check(existing_path)) {
		int result;
		existing_path = DeeFile_Filename(existing_path);
		if unlikely(!existing_path)
			goto err;
		result = fs_rename(existing_path, new_path);
		Dee_Decref(existing_path);
		return result;
	}
	if (DeeObject_AssertTypeExact(new_path, &DeeString_Type))
		goto err;
	old_name = DeeString_AsUtf8(existing_path);
	if unlikely(!old_name)
		goto err;
	new_name = DeeString_AsUtf8(new_path);
	if unlikely(!new_name)
		goto err;
	DBG_ALIGNMENT_DISABLE();
	if (rename(old_name, new_name)) {
		DBG_ALIGNMENT_ENABLE();
		DeeError_Throwf(&DeeError_FileNotFound,
		                "File %r could not be found or the path of %r does not exist",
		                existing_path, new_path);
		goto err;
	}
	DBG_ALIGNMENT_ENABLE();
	return 0;
err:
	return -1;
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
	return fs_gethostname();
}

#include "generic-dir.c.inl"

DECL_END

#endif /* !GUARD_DEX_FS_STDIO_C_INL */
