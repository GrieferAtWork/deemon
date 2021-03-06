/* Copyright (c) 2018-2021 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2021 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEX_POSIX_LIBPOSIX_C
#define GUARD_DEX_POSIX_LIBPOSIX_C 1
#define CONFIG_BUILDING_LIBPOSIX 1
#define DEE_SOURCE 1

#include "libposix.h"
/**/

#ifndef __INTELLISENSE__
#include "p-access.c.inl"
#include "p-cpucount.c.inl"
#include "p-errno.c.inl"
#include "p-exit.c.inl"
#include "p-fd.c.inl"
#include "p-open.c.inl"
#include "p-pipe.c.inl"
#include "p-readwrite.c.inl"
#include "p-realpath.c.inl"
#include "p-sched.c.inl"
#include "p-sync.c.inl"
#include "p-truncate.c.inl"

/* Include fs.c.inl last, since this once #undef's a
 * bunch of stuff that may be needed by other components. */
#include "p-fs.c.inl"
#endif /* !__INTELLISENSE__ */

#include <deemon/seq.h>
#include <deemon/string.h>


DECL_BEGIN



#if defined(NEED_ERR_UNSUPPORTED) || defined(__INTELLISENSE__)
#undef NEED_ERR_UNSUPPORTED
INTERN ATTR_NOINLINE ATTR_UNUSED ATTR_COLD int DCALL
posix_err_unsupported(char const *__restrict name) {
	return DeeError_Throwf(&DeeError_UnsupportedAPI,
	                       "Unsupported function `%s'",
	                       name);
}
#endif /* NEED_ERR_UNSUPPORTED */

#if defined(NEED_GET_DFD_FILENAME) || defined(__INTELLISENSE__)
#undef NEED_GET_DFD_FILENAME
INTERN WUNUSED DREF /*String*/ DeeObject *DCALL
libposix_get_dfd_filename(int dfd, /*utf-8*/ char const *filename, int atflags) {
	(void)dfd;
	(void)filename;
	(void)atflags;
	/* TODO: `joinpath(frealpath(dfd), filename)' */
	DeeError_NOTIMPLEMENTED();
	return NULL;
}
#endif /* NEED_GET_DFD_FILENAME */

#ifndef CONFIG_HAVE_errno
#define posix_errno_USE_STUB
#endif /* !CONFIG_HAVE_errno */


/*[[[deemon
local ALL_STUBS = {
	("posix_truncate_USE_STUB",     { "truncate" }),
	("posix_ftruncate_USE_STUB",    { "ftruncate" }),
	("posix_pipe_USE_STUB",         { "pipe" }),
	("posix_pipe2_USE_STUB",        { "pipe2" }),
	("posix_strerror_USE_STUB",     { "strerror" }),
	("posix_strerrorname_USE_STUB", { "strerrorname" }),
	("posix_open_USE_STUB",         { "open", "_open" }),
	("posix_creat_USE_STUB",        { "creat", "_creat" }),
	("posix_close_USE_STUB",        { "close" }),
	("posix_isatty_USE_STUB",       { "isatty" }),
	("posix_umask_USE_STUB",        { "umask" }),
	("posix_dup_USE_STUB",          { "dup" }),
	("posix_dup2_USE_STUB",         { "dup2" }),
	("posix_dup3_USE_STUB",         { "dup3" }),
	("posix_errno_USE_STUB",        { "errno" }),
	("posix_fsync_USE_STUB",        { "fsync" }),
	("posix_fdatasync_USE_STUB",    { "fdatasync" }),
	("posix_system_USE_STUB",       { "system" }),
	("posix_getpid_USE_STUB",       { "getpid" }),
	("posix_read_USE_STUB",         { "read" }),
	("posix_lseek_USE_STUB",        { "lseek" }),
	("posix_write_USE_STUB",        { "write" }),
	("posix_pread_USE_STUB",        { "pread" }),
	("posix_pwrite_USE_STUB",       { "pwrite" }),
	("posix_access_USE_STUB",       { "access" }),
	("posix_euidaccess_USE_STUB",   { "euidaccess" }),
	("posix_cpu_count_USE_STUB",    { "cpu_count" }),
	("posix_opendir_USE_STUB",      { "dirent", "DIR", "opendir", "fdopendir" }),
};
for (local test, functions: ALL_STUBS) {
	functions = "\0".join(functions) + "\0";
	print("#ifdef ", test);
	print("#define len_", test, " +", #functions);
	print("#define str_", test, " '", "', '".join(for (local c: functions) c.encode("c-escape")), "',");
	print("#else /" "* ", test, " *" "/");
	print("#define len_", test, " /" "* nothing *" "/");
	print("#define str_", test, " /" "* nothing *" "/");
	print("#endif /" "* !", test, " *" "/");
}
print("#define POSIX_STUBS_TOTLEN 0 \\");
for (local test, none: ALL_STUBS)
	print("\tlen_", test, " \\");
print("/" "**" "/");
print("#if POSIX_STUBS_TOTLEN != 0");
print("PRIVATE struct {");
print("	OBJECT_HEAD");
print("	struct string_utf *s_data;");
print("	dhash_t            s_hash;");
print("	size_t             s_len;");
print("	char               s_str[POSIX_STUBS_TOTLEN];");
print("	struct string_utf  s_utf;");
print("} posix_missing_features = {");
print("	OBJECT_HEAD_INIT(&DeeString_Type),");
print("	&posix_missing_features.s_utf,");
print("	DEE_STRING_HASH_UNSET,");
print("	POSIX_STUBS_TOTLEN - 1, {");
for (local test, none: ALL_STUBS)
	print("\t\tstr_", test);
print("	},");
print("	{ Dee_STRING_WIDTH_1BYTE,");
print("	  Dee_STRING_UTF_FASCII,");
print("	  { (size_t *)posix_missing_features.s_str },");
print("	  posix_missing_features.s_str }");
print("};");
print("#else /" "* POSIX_STUBS_TOTLEN != 0 *" "/");
print("#undef POSIX_STUBS_TOTLEN");
print("#define POSIX_STUBS_TOTLEN 0");
print("#endif /" "* POSIX_STUBS_TOTLEN == 0 *" "/");
]]]*/
#ifdef posix_truncate_USE_STUB
#define len_posix_truncate_USE_STUB +9
#define str_posix_truncate_USE_STUB 't', 'r', 'u', 'n', 'c', 'a', 't', 'e', '\0',
#else /* posix_truncate_USE_STUB */
#define len_posix_truncate_USE_STUB /* nothing */
#define str_posix_truncate_USE_STUB /* nothing */
#endif /* !posix_truncate_USE_STUB */
#ifdef posix_ftruncate_USE_STUB
#define len_posix_ftruncate_USE_STUB +10
#define str_posix_ftruncate_USE_STUB 'f', 't', 'r', 'u', 'n', 'c', 'a', 't', 'e', '\0',
#else /* posix_ftruncate_USE_STUB */
#define len_posix_ftruncate_USE_STUB /* nothing */
#define str_posix_ftruncate_USE_STUB /* nothing */
#endif /* !posix_ftruncate_USE_STUB */
#ifdef posix_pipe_USE_STUB
#define len_posix_pipe_USE_STUB +5
#define str_posix_pipe_USE_STUB 'p', 'i', 'p', 'e', '\0',
#else /* posix_pipe_USE_STUB */
#define len_posix_pipe_USE_STUB /* nothing */
#define str_posix_pipe_USE_STUB /* nothing */
#endif /* !posix_pipe_USE_STUB */
#ifdef posix_pipe2_USE_STUB
#define len_posix_pipe2_USE_STUB +6
#define str_posix_pipe2_USE_STUB 'p', 'i', 'p', 'e', '2', '\0',
#else /* posix_pipe2_USE_STUB */
#define len_posix_pipe2_USE_STUB /* nothing */
#define str_posix_pipe2_USE_STUB /* nothing */
#endif /* !posix_pipe2_USE_STUB */
#ifdef posix_strerror_USE_STUB
#define len_posix_strerror_USE_STUB +9
#define str_posix_strerror_USE_STUB 's', 't', 'r', 'e', 'r', 'r', 'o', 'r', '\0',
#else /* posix_strerror_USE_STUB */
#define len_posix_strerror_USE_STUB /* nothing */
#define str_posix_strerror_USE_STUB /* nothing */
#endif /* !posix_strerror_USE_STUB */
#ifdef posix_strerrorname_USE_STUB
#define len_posix_strerrorname_USE_STUB +13
#define str_posix_strerrorname_USE_STUB 's', 't', 'r', 'e', 'r', 'r', 'o', 'r', 'n', 'a', 'm', 'e', '\0',
#else /* posix_strerrorname_USE_STUB */
#define len_posix_strerrorname_USE_STUB /* nothing */
#define str_posix_strerrorname_USE_STUB /* nothing */
#endif /* !posix_strerrorname_USE_STUB */
#ifdef posix_open_USE_STUB
#define len_posix_open_USE_STUB +11
#define str_posix_open_USE_STUB 'o', 'p', 'e', 'n', '\0', '_', 'o', 'p', 'e', 'n', '\0',
#else /* posix_open_USE_STUB */
#define len_posix_open_USE_STUB /* nothing */
#define str_posix_open_USE_STUB /* nothing */
#endif /* !posix_open_USE_STUB */
#ifdef posix_creat_USE_STUB
#define len_posix_creat_USE_STUB +13
#define str_posix_creat_USE_STUB 'c', 'r', 'e', 'a', 't', '\0', '_', 'c', 'r', 'e', 'a', 't', '\0',
#else /* posix_creat_USE_STUB */
#define len_posix_creat_USE_STUB /* nothing */
#define str_posix_creat_USE_STUB /* nothing */
#endif /* !posix_creat_USE_STUB */
#ifdef posix_close_USE_STUB
#define len_posix_close_USE_STUB +6
#define str_posix_close_USE_STUB 'c', 'l', 'o', 's', 'e', '\0',
#else /* posix_close_USE_STUB */
#define len_posix_close_USE_STUB /* nothing */
#define str_posix_close_USE_STUB /* nothing */
#endif /* !posix_close_USE_STUB */
#ifdef posix_isatty_USE_STUB
#define len_posix_isatty_USE_STUB +7
#define str_posix_isatty_USE_STUB 'i', 's', 'a', 't', 't', 'y', '\0',
#else /* posix_isatty_USE_STUB */
#define len_posix_isatty_USE_STUB /* nothing */
#define str_posix_isatty_USE_STUB /* nothing */
#endif /* !posix_isatty_USE_STUB */
#ifdef posix_umask_USE_STUB
#define len_posix_umask_USE_STUB +6
#define str_posix_umask_USE_STUB 'u', 'm', 'a', 's', 'k', '\0',
#else /* posix_umask_USE_STUB */
#define len_posix_umask_USE_STUB /* nothing */
#define str_posix_umask_USE_STUB /* nothing */
#endif /* !posix_umask_USE_STUB */
#ifdef posix_dup_USE_STUB
#define len_posix_dup_USE_STUB +4
#define str_posix_dup_USE_STUB 'd', 'u', 'p', '\0',
#else /* posix_dup_USE_STUB */
#define len_posix_dup_USE_STUB /* nothing */
#define str_posix_dup_USE_STUB /* nothing */
#endif /* !posix_dup_USE_STUB */
#ifdef posix_dup2_USE_STUB
#define len_posix_dup2_USE_STUB +5
#define str_posix_dup2_USE_STUB 'd', 'u', 'p', '2', '\0',
#else /* posix_dup2_USE_STUB */
#define len_posix_dup2_USE_STUB /* nothing */
#define str_posix_dup2_USE_STUB /* nothing */
#endif /* !posix_dup2_USE_STUB */
#ifdef posix_dup3_USE_STUB
#define len_posix_dup3_USE_STUB +5
#define str_posix_dup3_USE_STUB 'd', 'u', 'p', '3', '\0',
#else /* posix_dup3_USE_STUB */
#define len_posix_dup3_USE_STUB /* nothing */
#define str_posix_dup3_USE_STUB /* nothing */
#endif /* !posix_dup3_USE_STUB */
#ifdef posix_errno_USE_STUB
#define len_posix_errno_USE_STUB +6
#define str_posix_errno_USE_STUB 'e', 'r', 'r', 'n', 'o', '\0',
#else /* posix_errno_USE_STUB */
#define len_posix_errno_USE_STUB /* nothing */
#define str_posix_errno_USE_STUB /* nothing */
#endif /* !posix_errno_USE_STUB */
#ifdef posix_fsync_USE_STUB
#define len_posix_fsync_USE_STUB +6
#define str_posix_fsync_USE_STUB 'f', 's', 'y', 'n', 'c', '\0',
#else /* posix_fsync_USE_STUB */
#define len_posix_fsync_USE_STUB /* nothing */
#define str_posix_fsync_USE_STUB /* nothing */
#endif /* !posix_fsync_USE_STUB */
#ifdef posix_fdatasync_USE_STUB
#define len_posix_fdatasync_USE_STUB +10
#define str_posix_fdatasync_USE_STUB 'f', 'd', 'a', 't', 'a', 's', 'y', 'n', 'c', '\0',
#else /* posix_fdatasync_USE_STUB */
#define len_posix_fdatasync_USE_STUB /* nothing */
#define str_posix_fdatasync_USE_STUB /* nothing */
#endif /* !posix_fdatasync_USE_STUB */
#ifdef posix_system_USE_STUB
#define len_posix_system_USE_STUB +7
#define str_posix_system_USE_STUB 's', 'y', 's', 't', 'e', 'm', '\0',
#else /* posix_system_USE_STUB */
#define len_posix_system_USE_STUB /* nothing */
#define str_posix_system_USE_STUB /* nothing */
#endif /* !posix_system_USE_STUB */
#ifdef posix_getpid_USE_STUB
#define len_posix_getpid_USE_STUB +7
#define str_posix_getpid_USE_STUB 'g', 'e', 't', 'p', 'i', 'd', '\0',
#else /* posix_getpid_USE_STUB */
#define len_posix_getpid_USE_STUB /* nothing */
#define str_posix_getpid_USE_STUB /* nothing */
#endif /* !posix_getpid_USE_STUB */
#ifdef posix_read_USE_STUB
#define len_posix_read_USE_STUB +5
#define str_posix_read_USE_STUB 'r', 'e', 'a', 'd', '\0',
#else /* posix_read_USE_STUB */
#define len_posix_read_USE_STUB /* nothing */
#define str_posix_read_USE_STUB /* nothing */
#endif /* !posix_read_USE_STUB */
#ifdef posix_lseek_USE_STUB
#define len_posix_lseek_USE_STUB +6
#define str_posix_lseek_USE_STUB 'l', 's', 'e', 'e', 'k', '\0',
#else /* posix_lseek_USE_STUB */
#define len_posix_lseek_USE_STUB /* nothing */
#define str_posix_lseek_USE_STUB /* nothing */
#endif /* !posix_lseek_USE_STUB */
#ifdef posix_write_USE_STUB
#define len_posix_write_USE_STUB +6
#define str_posix_write_USE_STUB 'w', 'r', 'i', 't', 'e', '\0',
#else /* posix_write_USE_STUB */
#define len_posix_write_USE_STUB /* nothing */
#define str_posix_write_USE_STUB /* nothing */
#endif /* !posix_write_USE_STUB */
#ifdef posix_pread_USE_STUB
#define len_posix_pread_USE_STUB +6
#define str_posix_pread_USE_STUB 'p', 'r', 'e', 'a', 'd', '\0',
#else /* posix_pread_USE_STUB */
#define len_posix_pread_USE_STUB /* nothing */
#define str_posix_pread_USE_STUB /* nothing */
#endif /* !posix_pread_USE_STUB */
#ifdef posix_pwrite_USE_STUB
#define len_posix_pwrite_USE_STUB +7
#define str_posix_pwrite_USE_STUB 'p', 'w', 'r', 'i', 't', 'e', '\0',
#else /* posix_pwrite_USE_STUB */
#define len_posix_pwrite_USE_STUB /* nothing */
#define str_posix_pwrite_USE_STUB /* nothing */
#endif /* !posix_pwrite_USE_STUB */
#ifdef posix_access_USE_STUB
#define len_posix_access_USE_STUB +7
#define str_posix_access_USE_STUB 'a', 'c', 'c', 'e', 's', 's', '\0',
#else /* posix_access_USE_STUB */
#define len_posix_access_USE_STUB /* nothing */
#define str_posix_access_USE_STUB /* nothing */
#endif /* !posix_access_USE_STUB */
#ifdef posix_euidaccess_USE_STUB
#define len_posix_euidaccess_USE_STUB +11
#define str_posix_euidaccess_USE_STUB 'e', 'u', 'i', 'd', 'a', 'c', 'c', 'e', 's', 's', '\0',
#else /* posix_euidaccess_USE_STUB */
#define len_posix_euidaccess_USE_STUB /* nothing */
#define str_posix_euidaccess_USE_STUB /* nothing */
#endif /* !posix_euidaccess_USE_STUB */
#ifdef posix_cpu_count_USE_STUB
#define len_posix_cpu_count_USE_STUB +10
#define str_posix_cpu_count_USE_STUB 'c', 'p', 'u', '_', 'c', 'o', 'u', 'n', 't', '\0',
#else /* posix_cpu_count_USE_STUB */
#define len_posix_cpu_count_USE_STUB /* nothing */
#define str_posix_cpu_count_USE_STUB /* nothing */
#endif /* !posix_cpu_count_USE_STUB */
#ifdef posix_opendir_USE_STUB
#define len_posix_opendir_USE_STUB +29
#define str_posix_opendir_USE_STUB 'd', 'i', 'r', 'e', 'n', 't', '\0', 'D', 'I', 'R', '\0', 'o', 'p', 'e', 'n', 'd', 'i', 'r', '\0', 'f', 'd', 'o', 'p', 'e', 'n', 'd', 'i', 'r', '\0',
#else /* posix_opendir_USE_STUB */
#define len_posix_opendir_USE_STUB /* nothing */
#define str_posix_opendir_USE_STUB /* nothing */
#endif /* !posix_opendir_USE_STUB */
#define POSIX_STUBS_TOTLEN 0 \
	len_posix_truncate_USE_STUB \
	len_posix_ftruncate_USE_STUB \
	len_posix_pipe_USE_STUB \
	len_posix_pipe2_USE_STUB \
	len_posix_strerror_USE_STUB \
	len_posix_strerrorname_USE_STUB \
	len_posix_open_USE_STUB \
	len_posix_creat_USE_STUB \
	len_posix_close_USE_STUB \
	len_posix_isatty_USE_STUB \
	len_posix_umask_USE_STUB \
	len_posix_dup_USE_STUB \
	len_posix_dup2_USE_STUB \
	len_posix_dup3_USE_STUB \
	len_posix_errno_USE_STUB \
	len_posix_fsync_USE_STUB \
	len_posix_fdatasync_USE_STUB \
	len_posix_system_USE_STUB \
	len_posix_getpid_USE_STUB \
	len_posix_read_USE_STUB \
	len_posix_lseek_USE_STUB \
	len_posix_write_USE_STUB \
	len_posix_pread_USE_STUB \
	len_posix_pwrite_USE_STUB \
	len_posix_access_USE_STUB \
	len_posix_euidaccess_USE_STUB \
	len_posix_cpu_count_USE_STUB \
	len_posix_opendir_USE_STUB \
/**/
#if POSIX_STUBS_TOTLEN != 0
PRIVATE struct {
	OBJECT_HEAD
	struct string_utf *s_data;
	dhash_t            s_hash;
	size_t             s_len;
	char               s_str[POSIX_STUBS_TOTLEN];
	struct string_utf  s_utf;
} posix_missing_features = {
	OBJECT_HEAD_INIT(&DeeString_Type),
	&posix_missing_features.s_utf,
	DEE_STRING_HASH_UNSET,
	POSIX_STUBS_TOTLEN - 1, {
		str_posix_truncate_USE_STUB
		str_posix_ftruncate_USE_STUB
		str_posix_pipe_USE_STUB
		str_posix_pipe2_USE_STUB
		str_posix_strerror_USE_STUB
		str_posix_strerrorname_USE_STUB
		str_posix_open_USE_STUB
		str_posix_creat_USE_STUB
		str_posix_close_USE_STUB
		str_posix_isatty_USE_STUB
		str_posix_umask_USE_STUB
		str_posix_dup_USE_STUB
		str_posix_dup2_USE_STUB
		str_posix_dup3_USE_STUB
		str_posix_errno_USE_STUB
		str_posix_fsync_USE_STUB
		str_posix_fdatasync_USE_STUB
		str_posix_system_USE_STUB
		str_posix_getpid_USE_STUB
		str_posix_read_USE_STUB
		str_posix_lseek_USE_STUB
		str_posix_write_USE_STUB
		str_posix_pread_USE_STUB
		str_posix_pwrite_USE_STUB
		str_posix_access_USE_STUB
		str_posix_euidaccess_USE_STUB
		str_posix_cpu_count_USE_STUB
		str_posix_opendir_USE_STUB
	},
	{ Dee_STRING_WIDTH_1BYTE,
	  Dee_STRING_UTF_FASCII,
	  { (size_t *)posix_missing_features.s_str },
	  posix_missing_features.s_str }
};
#else /* POSIX_STUBS_TOTLEN != 0 */
#undef POSIX_STUBS_TOTLEN
#define POSIX_STUBS_TOTLEN 0
#endif /* POSIX_STUBS_TOTLEN == 0 */
/*[[[end]]]*/


#if POSIX_STUBS_TOTLEN != 0

PRIVATE WUNUSED DREF DeeObject *DCALL pst_ctor(void);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
pst_Iterator_get(DeeObject *__restrict UNUSED(self)) {
	return DeeModule_GetExtern("rt", "StringSplitIterator");
}

PRIVATE DEFINE_STRING(str_nul, "\0");
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
pst_iter_self(DeeObject *__restrict UNUSED(self)) {
	DeeObject *argv[] = { (DeeObject *)&str_nul };
	DREF DeeObject *seq, *iter;
	seq = DeeObject_CallAttrString((DeeObject *)&posix_missing_features,
	                               "split", 1, argv);
	if unlikely(!seq)
		goto err;
	iter = DeeObject_IterSelf(seq);
	Dee_Decref(seq);
	return iter;
err:
	return NULL;
}

#ifndef CONFIG_HAVE_memmem
#define memmem dee_memmem
DeeSystem_DEFINE_memmem(dee_memmem)
#endif /* !CONFIG_HAVE_memmem */


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
pst_contains(DeeObject *__restrict UNUSED(self),
             DeeObject *__restrict function_name) {
	void *p;
	if unlikely(!DeeString_Check(function_name))
		return_false;
	p = memmem(posix_missing_features.s_str,
	           sizeof(posix_missing_features.s_str),
	           DeeString_STR(function_name),
	           (DeeString_SIZE(function_name) + 1) * sizeof(char));
	return_bool_(p != NULL);
}

PRIVATE struct type_seq pst_seq = {
	/* .tp_iter_self = */ &pst_iter_self,
	/* .tp_size      = */ NULL,
	/* .tp_contains  = */ &pst_contains
};

PRIVATE struct type_getset tpconst pst_class_getsets[] = {
	{ "Iterator", &pst_Iterator_get, NULL, NULL },
	{ NULL }
};


PRIVATE DeeTypeObject PosixStubsList_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_PosixStubsList",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FVARIABLE | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_var = */ {
				/* .tp_ctor      = */ (dfunptr_t)&pst_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&DeeObject_NewRef,
				/* .tp_deep_ctor = */ (dfunptr_t)&DeeObject_NewRef,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				/* .tp_free      = */ (dfunptr_t)NULL
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
	/* .tp_seq           = */ &pst_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ pst_class_getsets,
	/* .tp_class_members = */ NULL
};

PRIVATE DeeObject PosixStubsList_Singleton = { OBJECT_HEAD_INIT(&PosixStubsList_Type) };
PRIVATE WUNUSED DREF DeeObject *DCALL pst_ctor(void) {
	return_reference_(&PosixStubsList_Singleton);
}
#else /* POSIX_STUBS_TOTLEN != 0 */
/* Special case: There aren't _any_ stubs! */
#define PosixStubsList_Singleton DeeTuple_Empty
#endif /* POSIX_STUBS_TOTLEN == 0 */


PRIVATE char const *import_table[] = {
	/* NOTE: Indices in this table must match those used by `*_MODULE' macros! */
	"fs", /* #define FS_MODULE   DEX.d_imports[0] */
	NULL
};

#ifdef __INTELLISENSE__
#define D(...) /* nothing */
#else          /* __INTELLISENSE__ */
#define D(...) __VA_ARGS__
#endif /* !__INTELLISENSE__ */


/* DT_* constants. */
INTDEF DeeObject posix_DT_UNKNOWN;
INTDEF DeeObject posix_DT_FIFO;
INTDEF DeeObject posix_DT_CHR;
INTDEF DeeObject posix_DT_DIR;
INTDEF DeeObject posix_DT_BLK;
INTDEF DeeObject posix_DT_REG;
INTDEF DeeObject posix_DT_LNK;
INTDEF DeeObject posix_DT_SOCK;
INTDEF DeeObject posix_DT_WHT;
INTDEF DeeObject posix_DTTOIF;
INTDEF DeeObject posix_IFTODT;
INTDEF DeeObject posix_fdopendir;

PRIVATE struct dex_symbol symbols[] = {

	{ "stubs", &PosixStubsList_Singleton, MODSYM_FNORMAL,
	  DOC("->?S?Dstring\n"
	      "List of the names of all of the functions that are implemented as stubs\n"
	      "The names contained within this list are identical to the names of the "
	      "resp. symbols exported by this module. e.g. ${'open' in posix.stubs} would "
	      "mean that ?Gopen will unconditionally throw :{UnsupportedAPI}. Other functions "
	      "may behave differently and always behave as no-ops, but the general behavior "
	      "is that the function will simply be implemented as a stub") },

	/* File control */
	D(POSIX_OPEN_DEF_DOC("Open a given @filename using @oflags (a set of ${O_*} flags), and @mode (describing "
	                     "the posix permissions to apply to a newly created file when ?GO_CREAT is given)"))
	D(POSIX__OPEN_DEF_DOC("Same as ?Gopen, but whereas ?Gopen will automatically set the ?GO_OBTAIN_DIR and ?GO_BINARY "
	                      "flags on platforms that define them in order to better standartize behavior of this "
	                      "function on those system, this function will not make any changes to the given @oflags"))
	D(POSIX_CREAT_DEF_DOC("Create a new file (same as ${open(filename, O_CREAT | O_WRONLY | O_TRUNC, mode)})"))
	D(POSIX__CREAT_DEF_DOC("Same as ?Gcreat, but on systems that define ?GO_BINARY, that flag is also passed "
	                       "via the internal @oflags list eventually passed to ?Gopen (or rather ?G_open)"))
	D(POSIX_READ_DEF_DOC("Read up to @count bytes into @buf\n"
	                     "When @buf is given, return the actual number of read bytes. "
	                     "Otherwise, read into a new :Bytes object that is then returned"))
	D(POSIX_WRITE_DEF_DOC("Write up to @count bytes from @buf, returning the actual number of bytes written"))
	D(POSIX_LSEEK_DEF)
	D(POSIX_FSYNC_DEF)
	D(POSIX_FDATASYNC_DEF)
	D(POSIX_CLOSE_DEF)
	D(POSIX_UMASK_DEF)
	D(POSIX_DUP_DEF)
	D(POSIX_DUP2_DEF)
	D(POSIX_DUP3_DEF)
	/* TODO: lockf() */
	D(POSIX_PREAD_DEF)
	/* TODO: readv() */
	/* TODO: preadv() */
	D(POSIX_PWRITE_DEF)
	/* TODO: writev() */
	/* TODO: pwritev() */
	D(POSIX_ISATTY_DEF)
	D(POSIX_PIPE_DEF)
	D(POSIX_PIPE2_DEF)
	/* TODO: fcntl() */
	/* TODO: ioctl() */
	/* TODO: posix_fallocate() */
	/* TODO: posix_fadvise() */

	/* Filesystem control */
	D(POSIX_TRUNCATE_DEF)
	D(POSIX_FTRUNCATE_DEF)
	D(POSIX_ACCESS_DEF)
	D(POSIX_EUIDACCESS_DEF)
	D(POSIX_FACCESSAT_DEF)
	D(POSIX_FCHOWNAT_DEF)
	D(POSIX_FCHMODAT_DEF)
	/* TODO: chflags() */
	/* TODO: lchflags() */
	/* TODO: chroot() */
	/* TODO: mkfifo() */
	/* TODO: mknod() */
	/* TODO: major() */
	/* TODO: minor() */
	/* TODO: mkdev() */
	D(POSIX_SYNC_DEF)
	/* TODO: utime() */
	/* TODO: pathconf() */
	/* TODO: fpathconf() */
	/* TODO: statvfs() */
	/* TODO: fstatvfs() */
	/* TODO: getxattr() */
	/* TODO: lgetxattr() */
	/* TODO: fgetxattr() */
	/* TODO: setxattr() */
	/* TODO: lsetxattr() */
	/* TODO: fsetxattr() */
	/* TODO: removexattr() */
	/* TODO: lremovexattr() */
	/* TODO: flremovexattr() */
	/* TODO: listxattr() */
	/* TODO: llistxattr() */
	/* TODO: flistxattr() */

	/* Path normalization */
	D(POSIX_REALPATH_DEF)

	/* System information */
	/* TODO: uname() */
	/* TODO: confstr() */
	/* TODO: sysconf() */
	/* TODO: times() */
	/* TODO: getloadavg() */

	/* Terminal control */
	/* TODO: ttyname() */
	/* TODO: ctermid() */
	/* TODO: openpty() */
	/* TODO: forkpty() */
	/* TODO: getlogin() */
	/* TODO: tcgetpgrp() */
	/* TODO: tcsetpgrp() */

	/* Process control */
	D(POSIX_GETPID_DEF)
	D(POSIX_SYSTEM_DEF)
	/* TODO: execl() */
	/* TODO: execle() */
	/* TODO: execlp() */
	/* TODO: execlpe() */
	/* TODO: execv() */
	/* TODO: execve() */
	/* TODO: execvp() */
	/* TODO: execvpe() */
	/* TODO: fexecve() */
	/* TODO: cwait() */
	/* TODO: spawnl() */
	/* TODO: spawnle() */
	/* TODO: spawnlp() */
	/* TODO: spawnlpe() */
	/* TODO: spawnv() */
	/* TODO: spawnve() */
	/* TODO: spawnvp() */
	/* TODO: spawnvpe() */

	/* Scheduling control */
	D(POSIX_SCHED_YIELD_DEF)
	/* TODO: sched_get_priority_min() */
	/* TODO: sched_get_priority_max() */
	/* TODO: sched_getparam() */
	/* TODO: sched_setparam() */
	/* TODO: sched_getscheduler() */
	/* TODO: sched_setscheduler() */
	/* TODO: sched_rr_get_interval() */
	/* TODO: sched_getaffinity() */
	/* TODO: sched_getaffinity() */
	/* TODO: nice() */
	/* TODO: getpriority() */
	/* TODO: setpriority() */
	/* TODO: fork() */
	/* TODO: wait() */
	/* TODO: wait3() */
	/* TODO: wait4() */
	/* TODO: waitid() */
	/* TODO: waitpid() */
	/* TODO: kill() */
	/* TODO: killpg() */
	/* TODO: getppid() */
	/* TODO: getpgrp() */
	/* TODO: setpgrp() */
	/* TODO: getpgid() */
	/* TODO: setpgid() */
	/* TODO: getsid() */
	/* TODO: setsid() */
	/* TODO: WCOREDUMP() */
	/* TODO: WIFCONTINUED() */
	/* TODO: WIFSTOPPED() */
	/* TODO: WIFSIGNALED() */
	/* TODO: WIFEXITED() */
	/* TODO: WEXITSTATUS() */
	/* TODO: WTERMSIG() */
	/* TODO: WSTOPSIG() */

	/* User/Permission control */
	/* TODO: geteuid() */
	/* TODO: seteuid() */
	/* TODO: getegid() */
	/* TODO: setegid() */
	/* TODO: getgid() */
	/* TODO: setgid() */
	/* TODO: getuid() */
	/* TODO: setuid() */
	/* TODO: setreuid() */
	/* TODO: setregid() */
	/* TODO: getgrouplist() */
	/* TODO: getgroups() */
	/* TODO: setgroups() */
	/* TODO: initgroups() */
	/* TODO: setresuid() */
	/* TODO: setresgid() */
	/* TODO: getresuid() */
	/* TODO: getresgid() */

	/* Random number generation */
	/* TODO: urandom() */
	/* TODO: getrandom() */

	/* Environ control */
	D(POSIX_GETENV_DEF)
	D(POSIX_SETENV_DEF)
	D(POSIX_PUTENV_DEF)
	D(POSIX_UNSETENV_DEF)
	D(POSIX_CLEARENV_DEF)

	/* Python-like helper functions */
	D(POSIX_CPU_COUNT_DEF_DOC("Returns the ## of available processors on the host machine"))
	/* TODO: get_inheritable() */
	/* TODO: set_inheritable() */

	/* Higher-level wrapper functions */
	/* TODO: popen() */
	/* TODO: fdopen() (Basically just a wrapper around `DeeFile_OpenFd') */


	/* Directory access */
	{ "dirent", (DeeObject *)&DeeDirIterator_Type, MODSYM_FNORMAL },
	{ "DIR", (DeeObject *)&DeeDir_Type, MODSYM_FNORMAL },
	{ "opendir", (DeeObject *)&DeeDir_Type, MODSYM_FNORMAL,
	  DOC("(path:?X3?Dstring?DFile?Dint,skipdots=!t,inheritfd=!f)->?GDIR\n"
	      "Read the contents of a given directory. The returned "
	      "object may be iterated to yield ?Gdirent objects.\n"
	      "Additionally, you may specify @skipdots as !f if you "
	      "wish to include the special $'.' and $'..' entires.") },
	{ "fdopendir", &posix_fdopendir, MODSYM_FNORMAL,
	  DOC("(path:?X3?Dstring?DFile?Dint,skipdots=!t,inheritfd=!t)->?GDIR\n"
	      "Same as ?Gopendir, but the default value of @inheritfd is !t, "
	      "mimicking the behavior of the native $fdopendir function") },

	/* File type constants. */
	{ "DT_UNKNOWN", &posix_DT_UNKNOWN, MODSYM_FNORMAL },
	{ "DT_FIFO", &posix_DT_FIFO, MODSYM_FNORMAL },
	{ "DT_CHR", &posix_DT_CHR, MODSYM_FNORMAL },
	{ "DT_DIR", &posix_DT_DIR, MODSYM_FNORMAL },
	{ "DT_BLK", &posix_DT_BLK, MODSYM_FNORMAL },
	{ "DT_REG", &posix_DT_REG, MODSYM_FNORMAL },
	{ "DT_LNK", &posix_DT_LNK, MODSYM_FNORMAL },
	{ "DT_SOCK", &posix_DT_SOCK, MODSYM_FNORMAL },
	{ "DT_WHT", &posix_DT_WHT, MODSYM_FNORMAL },
	{ "DTTOIF", &posix_DTTOIF, MODSYM_FNORMAL,
	  DOC("(dt:?Dint)->?Dint\n"
	      "Convert a ${DT_*} constant to ${S_IF*}") },
	{ "IFTODT", &posix_IFTODT, MODSYM_FNORMAL,
	  DOC("(if:?Dint)->?Dint\n"
	      "Convert an ${S_IF*} constant to ${DT_*}") },


/* Forward-aliases to `libfs' */
#define DEFINE_LIBFS_ALIAS_ALT(altname, name, libfs_name, proto)                           \
	D({ altname, (DeeObject *)&libposix_getfs_##name, MODSYM_FPROPERTY | MODSYM_FREADONLY, \
	    DOC(proto "Alias for ?Efs:" libfs_name) }, )
#define DEFINE_LIBFS_ALIAS_S_ALT(altname, name, proto)                            \
	D({ altname,                                                                  \
	    (DeeObject *)&libposix_getfs_##name, MODSYM_FPROPERTY | MODSYM_FREADONLY, \
	    DOC(proto "Alias for ?Efs:" #name) }, )
#define DEFINE_LIBFS_ALIAS(name, libfs_name, proto) \
	DEFINE_LIBFS_ALIAS_ALT(#name, name, libfs_name, proto)
#define DEFINE_LIBFS_ALIAS_S(name, proto) \
	DEFINE_LIBFS_ALIAS_S_ALT(DeeString_STR(&libposix_libfs_name_##name), name, proto)
	DEFINE_LIBFS_ALIAS_S(environ, "->?S?T2?Dstring?Dstring\n")
	DEFINE_LIBFS_ALIAS_S(stat, "(path:?Dstring)\n")
	DEFINE_LIBFS_ALIAS_S(lstat, "(path:?Dstring)\n")
	DEFINE_LIBFS_ALIAS_S(getcwd, "->?Dstring\n")
	DEFINE_LIBFS_ALIAS_S(gethostname, "->?Dstring\n")
	DEFINE_LIBFS_ALIAS_S(chdir, "(path:?Dstring)\n")
	DEFINE_LIBFS_ALIAS_S(chmod, "(path:?Dstring,mode:?X2?Dstring?Dint)\n")
	DEFINE_LIBFS_ALIAS_S(lchmod, "(path:?Dstring,mode:?X2?Dstring?Dint)\n")
	DEFINE_LIBFS_ALIAS_S(chown, "(path:?Dstring,user:?X3?Efs:User?Dstring?Dint,group:?X3?Efs:Group?Dstring?Dint)\n")
	DEFINE_LIBFS_ALIAS_S(lchown, "(path:?Dstring,user:?X3?Efs:User?Dstring?Dint,group:?X3?Efs:Group?Dstring?Dint)\n")
	DEFINE_LIBFS_ALIAS_S(mkdir, "(path:?Dstring,permissions:?X2?Dstring?Dint=!N)\n")
	DEFINE_LIBFS_ALIAS_S(rmdir, "(path:?Dstring)\n")
	DEFINE_LIBFS_ALIAS_S(unlink, "(path:?Dstring)\n")
	DEFINE_LIBFS_ALIAS_S(remove, "(path:?Dstring)\n")
	DEFINE_LIBFS_ALIAS_S(rename, "(existing_path:?Dstring,new_path:?Dstring)\n")
	DEFINE_LIBFS_ALIAS_S(link, "(existing_path:?X3?Dstring?DFile?Dint,new_path:?Dstring)\n")
	DEFINE_LIBFS_ALIAS_S(symlink, "(target_text:?Dstring,link_path:?Dstring,format_target=!t)\n")
	DEFINE_LIBFS_ALIAS_S(readlink, "(path:?Dstring)->?Dstring\n(fp:?DFile)->?Dstring\n(fd:?Dint)->?Dstring\n")
	DEFINE_LIBFS_ALIAS_S_ALT("fstat", stat, "(fp:?DFile)\n(fd:?Dint)\n")
	DEFINE_LIBFS_ALIAS_S_ALT("fstatat", stat, "(dfd:?Dint,path:?Dstring,atflags=!0)\n")
	DEFINE_LIBFS_ALIAS_S_ALT("fchdir", chdir, "(fp:?DFile)\n(fd:?Dint)\n")
	DEFINE_LIBFS_ALIAS_S_ALT("fchmod", chmod, "(fp:?DFile,mode:?X2?Dstring?Dint)\n"
	                                          "(fd:?Dint,mode:?X2?Dstring?Dint)\n")
	DEFINE_LIBFS_ALIAS_S_ALT("fchown", chown, "(fp:?DFile,user:?X3?Efs:User?Dstring?Dint,group:?X3?Efs:Group?Dstring?Dint)\n"
	                                          "(fd:?Dint,user:?X3?Efs:User?Dstring?Dint,group:?X3?Efs:Group?Dstring?Dint)\n")


	/* stat.st_mode bits. */
	DEFINE_LIBFS_ALIAS_S(S_IFMT, "->?Dint\n")
	DEFINE_LIBFS_ALIAS_S(S_IFDIR, "->?Dint\n")
	DEFINE_LIBFS_ALIAS_S(S_IFCHR, "->?Dint\n")
	DEFINE_LIBFS_ALIAS_S(S_IFBLK, "->?Dint\n")
	DEFINE_LIBFS_ALIAS_S(S_IFREG, "->?Dint\n")
	DEFINE_LIBFS_ALIAS_S(S_IFIFO, "->?Dint\n")
	DEFINE_LIBFS_ALIAS_S(S_IFLNK, "->?Dint\n")
	DEFINE_LIBFS_ALIAS_S(S_IFSOCK, "->?Dint\n")
	DEFINE_LIBFS_ALIAS_S(S_ISUID, "->?Dint\n")
	DEFINE_LIBFS_ALIAS_S(S_ISGID, "->?Dint\n")
	DEFINE_LIBFS_ALIAS_S(S_ISVTX, "->?Dint\n")
	DEFINE_LIBFS_ALIAS_S(S_IRUSR, "->?Dint\n")
	DEFINE_LIBFS_ALIAS_S(S_IWUSR, "->?Dint\n")
	DEFINE_LIBFS_ALIAS_S(S_IXUSR, "->?Dint\n")
	DEFINE_LIBFS_ALIAS_S(S_IRGRP, "->?Dint\n")
	DEFINE_LIBFS_ALIAS_S(S_IWGRP, "->?Dint\n")
	DEFINE_LIBFS_ALIAS_S(S_IXGRP, "->?Dint\n")
	DEFINE_LIBFS_ALIAS_S(S_IROTH, "->?Dint\n")
	DEFINE_LIBFS_ALIAS_S(S_IWOTH, "->?Dint\n")
	DEFINE_LIBFS_ALIAS_S(S_IXOTH, "->?Dint\n")

	/* stat.st_mode helper functions. */
	DEFINE_LIBFS_ALIAS_S(S_ISDIR, "(mode:?Dint)->?Dbool\n")
	DEFINE_LIBFS_ALIAS_S(S_ISCHR, "(mode:?Dint)->?Dbool\n")
	DEFINE_LIBFS_ALIAS_S(S_ISBLK, "(mode:?Dint)->?Dbool\n")
	DEFINE_LIBFS_ALIAS_S(S_ISREG, "(mode:?Dint)->?Dbool\n")
	DEFINE_LIBFS_ALIAS_S(S_ISFIFO, "(mode:?Dint)->?Dbool\n")
	DEFINE_LIBFS_ALIAS_S(S_ISLNK, "(mode:?Dint)->?Dbool\n")
	DEFINE_LIBFS_ALIAS_S(S_ISSOCK, "(mode:?Dint)->?Dbool\n")

#undef DEFINE_LIBFS_ALIAS_S
#undef DEFINE_LIBFS_ALIAS
#undef DEFINE_LIBFS_ALIAS_S_ALT
#undef DEFINE_LIBFS_ALIAS_ALT

	/* Application exit control */
	D(POSIX_ATEXIT_DEF_DOC("Register a callback to-be invoked before ?Gexit (Same as :AppExit.atexit)"))
	D(POSIX_EXIT_DEF_DOC("Terminate execution of deemon after invoking ?Gatexit callbacks\n"
	                     "Termination is done using the C $exit or $_Exit functions, if available. However if these "
	                     "functions are not provided by the host, an :AppExit error is thrown instead\n"
	                     "When no @exitcode is given, the host's default default value of ?GEXIT_FAILURE, or $1 is used\n"
	                     "This function never returns normally"))
	D(POSIX__EXIT_DEF_DOC("Terminate execution of deemon without invoking ?Gatexit callbacks (s.a. ?Gexit)"))
	D(POSIX_ABORT_DEF_DOC("Same as ?G_Exit when passing ?GEXIT_FAILURE"))

	/* *_FILENO values */
	D(POSIX_FD_DEFS)

	/* O_* and AT_* values */
	D(POSIX_OPEN_DEFS)

	/* SEEK_* values */
	D(POSIX_READWRITE_DEFS)

	/* EXIT_* values */
	D(POSIX_EXIT_DEFS)

	/* E* errno codes */
	D(POSIX_ERRNO_DEFS)

	/* *_OK codes for `access()' and friends */
	D(POSIX_ACCESS_DEFS)

	D({ "errno", (DeeObject *)&posix_errno_get, MODSYM_FPROPERTY,
	    DOC("->?Dint\n"
	        "Read/write the C errno thread-local variable") }, )
	D({ NULL, (DeeObject *)&posix_errno_del, MODSYM_FNORMAL }, )
	D({ NULL, (DeeObject *)&posix_errno_set, MODSYM_FNORMAL }, )
	D(POSIX_STRERROR_DEF_DOC("Return the name of a given @errnum (which defaults to ?Gerrno), "
	                         "or return ?N if the error doesn't have an associated name"))
	D(POSIX_STRERRORNAME_DEF_DOC("Similar to ?Gstrerror, but instead of returning the message "
	                             "associated with a given @errnum (which defaults to ?Gerrno), "
	                             "return the name (e.g. $\"ENOENT\") of the error as a string\n"
	                             "If the given error number is not recognized, return ?N instead"))

	{ NULL }
};
#undef D

PUBLIC struct dex DEX = {
	/* .d_symbols      = */ symbols,
	/* .d_init         = */ NULL,
	/* .d_fini         = */ NULL,
	/* .d_import_names = */ { import_table }
};

DECL_END

#endif /* !GUARD_DEX_POSIX_LIBPOSIX_C */
