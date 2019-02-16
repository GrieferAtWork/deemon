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
#ifndef GUARD_DEX_FS_LIBFS_C
#define GUARD_DEX_FS_LIBFS_C 1
#define DEE_SOURCE 1
#define _KOS_SOURCE 1

#include "libfs.h"

#include <deemon/api.h>
#include <deemon/alloc.h>
#include <deemon/dex.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/error.h>
#include <deemon/error_types.h>
#include <deemon/bool.h>
#include <deemon/file.h>
#include <deemon/seq.h>
#include <deemon/map.h>
#include <deemon/arg.h>
#include <deemon/objmethod.h>

DECL_BEGIN


PRIVATE DREF DeeObject *DCALL
open_file_for_copy(DeeObject *__restrict name, int oflags, int mode) {
 dsysfd_t fd;
 /* Default case: The name is a string, meaning we need to open a file. */
 if (DeeString_Check(name))
     return DeeFile_Open(name,oflags,mode);
 /* Simple case: the name already is a file, so we should re-use it directly. */
 if (DeeFile_Check(name))
     return_reference_(name);
#ifdef CONFIG_FILENO_DENY_ARBITRARY_INTEGERS
 DeeObject_TypeAssertFailed(self,&DeeFile_Type);
 return NULL;
#else
 /* Fallback: Interpret the name as an integer file descriptor number. */
 if (DeeObject_AsFd(name,&fd))
     return NULL;
 /* Open the file descriptor, but don't inherit it. */
 return DeeFile_OpenFd(fd,NULL,oflags,false);
#endif
}

#ifdef CONFIG_HOST_WINDOWS
#define COPYFILE_BUFSIZE 4096
#elif defined(BUFSIZ)
#define COPYFILE_BUFSIZE BUFSIZ
#else
#define COPYFILE_BUFSIZE 512
#endif

INTERN int DCALL
fs_copyfile(DeeObject *__restrict existing_file,
            DeeObject *__restrict new_file,
            DeeObject *__restrict progress_callback) {
 DREF DeeObject *src,*dst; uint8_t *buffer;
 dpos_t file_size = 0,total = 0;
 /* Open a source and a destination stream. */
 src = open_file_for_copy(existing_file,OPEN_FRDONLY,0);
 if unlikely(!src) goto err;
 dst = open_file_for_copy(new_file,OPEN_FWRONLY|OPEN_FCREAT|OPEN_FEXCL|OPEN_FTRUNC,0644);
 if unlikely(!dst) goto err_src;
 buffer = (uint8_t *)Dee_Malloc(COPYFILE_BUFSIZE);
 if unlikely(!buffer) goto err_dst;
 if (!DeeNone_Check(progress_callback)) {
  /* Determine the full size of the source data stream. */
  file_size = DeeFile_GetSize(src);
  if unlikely(file_size == (dpos_t)-1) {
   /* Failed to determine stream size.
    * If this is because the source stream doesn't implement
    * seeking, ignore the error and proceed to copy file data. */
   if (!DeeError_Catch(&DeeError_NotImplemented))
        goto err_dst;
  }
 }
 for (;;) {
  dssize_t read_size;
  read_size = DeeFile_Read(src,buffer,COPYFILE_BUFSIZE);
  if unlikely(read_size < 0) goto err_dst;
  if (!read_size) break; /* EOF */
  read_size = DeeFile_WriteAll(dst,buffer,(size_t)read_size);
  if unlikely(read_size < 0) goto err_dst;
  if (read_size != COPYFILE_BUFSIZE) break; /* EOF */
  total += read_size;
  if (!DeeNone_Check(progress_callback)) {
   /* TODO: Invoke the given callback. */
  }
 }
 Dee_Decref(dst);
 Dee_Decref(src);
 return 0;
err_dst: Dee_Decref(dst);
err_src: Dee_Decref(src);
err:     return -1;
}



PRIVATE DEFINE_STRING(str_makeanon,"makeanon");
PRIVATE DREF DeeObject *default_DeeTime_New(uint64_t microseconds) {
 return DeeObject_CallAttrf(TIME_MODULE,
                           (DeeObject *)&str_makeanon,
                            "I64u",microseconds);
}

typedef DREF DeeObject *(*PDeeTime_New)(uint64_t microseconds);
PRIVATE PDeeTime_New p_DeeTime_New = NULL;

INTERN DREF DeeObject *DCALL
DeeTime_New(uint64_t microseconds) {
 PDeeTime_New funp = p_DeeTime_New;
 if (!funp) {
  /* Try to lookup the object as a native function pointer. */
  *(void **)&funp = DeeModule_GetNativeSymbol(TIME_MODULE,"DeeTime_New");
  /* Not a native dex, the wrong dex, or a user-implemented dex.
   * In any case, the specs describe a user-function `makeanon'
   * that does what we want to do. */
  if (!funp) funp = &default_DeeTime_New;
  p_DeeTime_New = funp;
  COMPILER_WRITE_BARRIER();
 }
 return (*funp)(microseconds);
}


PRIVATE DREF DeeObject *DCALL env_ctor(void) {
 return_reference_(&DeeEnv_Singleton);
}

PRIVATE DREF DeeObject *DCALL
env_iter(DeeObject *__restrict UNUSED(self)) {
 return DeeObject_NewDefault(&DeeEnvIterator_Type);
}

PRIVATE DREF DeeObject *DCALL
env_getitem(DeeObject *__restrict UNUSED(self),
            DeeObject *__restrict key) {
 if (DeeObject_AssertTypeExact(key,&DeeString_Type))
     return NULL;
 return fs_getenv(key,false);
}
PRIVATE int DCALL
env_delitem(DeeObject *__restrict UNUSED(self),
            DeeObject *__restrict key) {
 if (DeeObject_AssertTypeExact(key,&DeeString_Type))
     return -1;
 return fs_delenv(key);
}
PRIVATE int DCALL
env_setitem(DeeObject *__restrict UNUSED(self),
            DeeObject *__restrict key,
            DeeObject *__restrict value) {
 if (DeeObject_AssertTypeExact(key,&DeeString_Type) ||
     DeeObject_AssertTypeExact(value,&DeeString_Type))
     return -1;
 return fs_setenv(key,value);
}
PRIVATE DREF DeeObject *DCALL
env_contains(DeeObject *__restrict UNUSED(self),
             DeeObject *__restrict key) {
 if (DeeObject_AssertTypeExact(key,&DeeString_Type))
     return NULL;
 return_bool(fs_hasenv(key));
}

PRIVATE size_t DCALL
env_nsi_getsize(DeeObject *__restrict self) {
 return (*DeeSeq_Type.tp_seq->tp_nsi->nsi_seqlike.nsi_getsize)(self);
}

PRIVATE DREF DeeObject *DCALL
env_nsi_getdefault(DeeObject *__restrict UNUSED(self),
                   DeeObject *__restrict key,
                   DeeObject *__restrict defl) {
 DREF DeeObject *result;
 if (DeeObject_AssertTypeExact(key,&DeeString_Type))
     goto err;
 result = fs_getenv(key,true);
 if (result) return result;
 if (defl != ITER_DONE)
     Dee_Incref(defl);
 return defl;
err:
 return NULL;
}

PRIVATE struct type_nsi env_nsi = {
    /* .nsi_class = */TYPE_SEQX_CLASS_MAP,
    /* .nsi_flags = */TYPE_SEQX_FMUTABLE | TYPE_SEQX_FRESIZABLE,
    {
        /* .nsi_maplike = */{
            /* .nsi_getsize    = */(void *)&env_nsi_getsize, /* Must be defined because this one's mandatory... */
            /* .nsi_nextkey    = */(void *)&enviterator_next_key,
            /* .nsi_nextvalue  = */(void *)&enviterator_next_value,
            /* .nsi_getdefault = */(void *)&env_nsi_getdefault,
            /* .nsi_setdefault = */(void *)NULL,
            /* .nsi_updateold  = */(void *)NULL,
            /* .nsi_insertnew  = */(void *)NULL
        }        
    }
};

PRIVATE struct type_seq env_seq = {
    /* .tp_iter_self = */&env_iter,
    /* .tp_size      = */NULL,
    /* .tp_contains  = */&env_contains,
    /* .tp_get       = */&env_getitem,
    /* .tp_del       = */&env_delitem,
    /* .tp_set       = */&env_setitem,
    /* .tp_range_get = */NULL,
    /* .tp_range_del = */NULL,
    /* .tp_range_set = */NULL,
    /* .tp_nsi       = */&env_nsi
};

PRIVATE struct type_member env_members[] = {
    TYPE_MEMBER_CONST("iterator",&DeeEnvIterator_Type),
    TYPE_MEMBER_END
};

INTERN DeeTypeObject DeeEnv_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"environ",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FNORMAL|TP_FVARIABLE|TP_FFINAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeMapping_Type,
    /* .tp_init = */{
        {
            /* .tp_var = */{
                /* .tp_ctor      = */(void *)&env_ctor,
                /* .tp_copy_ctor = */NULL,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */NULL,
                /* .tp_free      = */NULL
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
    /* .tp_seq           = */&env_seq,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */NULL,
    /* .tp_members       = */NULL,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */env_members
};

INTERN DeeObject DeeEnv_Singleton = { OBJECT_HEAD_INIT(&DeeEnv_Type) };

PRIVATE struct keyword path_kwlist[] = { K(path), KEND };
PRIVATE struct keyword path_pwd_kwlist[] = { K(path), K(pwd), KEND };


PRIVATE DREF DeeObject *DCALL
f_libfs_gettmp(size_t argc, DeeObject **__restrict argv) {
 if (DeeArg_Unpack(argc,argv,":gettmp"))
     return NULL;
 return fs_gettmp();
}
PRIVATE DREF DeeObject *DCALL
f_libfs_getcwd(size_t argc, DeeObject **__restrict argv) {
 if (DeeArg_Unpack(argc,argv,":getcwd"))
     return NULL;
 return fs_getcwd();
}
PRIVATE DREF DeeObject *DCALL

f_libfs_chdir(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
 DeeObject *path;
 if (DeeArg_UnpackKw(argc,argv,kw,path_kwlist,"o:chdir",&path) ||
     fs_chdir(path))
     return NULL;
 return_none;
}
PRIVATE DREF DeeObject *DCALL
f_libfs_gethostname(size_t argc, DeeObject **__restrict argv) {
 if (DeeArg_Unpack(argc,argv,":gethostname"))
     return NULL;
 return fs_gethostname();
}

INTERN DREF DeeObject *DCALL
f_libfs_headof(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
 DeeObject *path;
 if (DeeArg_UnpackKw(argc,argv,kw,path_kwlist,"o:headof",&path) ||
     DeeObject_AssertTypeExact(path,&DeeString_Type))
     return NULL;
 return fs_pathhead(path);
}
INTERN DREF DeeObject *DCALL
f_libfs_tailof(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
 DeeObject *path;
 if (DeeArg_UnpackKw(argc,argv,kw,path_kwlist,"o:tailof",&path) ||
     DeeObject_AssertTypeExact(path,&DeeString_Type))
     return NULL;
 return fs_pathtail(path);
}
INTERN DREF DeeObject *DCALL
f_libfs_fileof(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
 DeeObject *path;
 if (DeeArg_UnpackKw(argc,argv,kw,path_kwlist,"o:fileof",&path) ||
     DeeObject_AssertTypeExact(path,&DeeString_Type))
     return NULL;
 return fs_pathfile(path);
}
INTERN DREF DeeObject *DCALL
f_libfs_extof(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
 DeeObject *path;
 if (DeeArg_UnpackKw(argc,argv,kw,path_kwlist,"o:extof",&path) ||
     DeeObject_AssertTypeExact(path,&DeeString_Type))
     return NULL;
 return fs_pathext(path);
}
INTERN DREF DeeObject *DCALL
f_libfs_driveof(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
 DeeObject *path;
 if (DeeArg_UnpackKw(argc,argv,kw,path_kwlist,"o:driveof",&path) ||
     DeeObject_AssertTypeExact(path,&DeeString_Type))
     return NULL;
 return fs_pathdrive(path);
}
INTERN DREF DeeObject *DCALL
f_libfs_inctrail(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
 DeeObject *path;
 if (DeeArg_UnpackKw(argc,argv,kw,path_kwlist,"o:inctrail",&path) ||
     DeeObject_AssertTypeExact(path,&DeeString_Type))
     return NULL;
 return fs_pathinctrail(path);
}
INTERN DREF DeeObject *DCALL
f_libfs_exctrail(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
 DeeObject *path;
 if (DeeArg_UnpackKw(argc,argv,kw,path_kwlist,"o:exctrail",&path) ||
     DeeObject_AssertTypeExact(path,&DeeString_Type))
     return NULL;
 return fs_pathinctrail(path);
}
INTERN DREF DeeObject *DCALL
f_libfs_abspath(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
 DeeObject *path,*pwd = NULL;
 if (DeeArg_UnpackKw(argc,argv,kw,path_pwd_kwlist,"o|o:abspath",&path,&pwd) ||
     DeeObject_AssertTypeExact(path,&DeeString_Type) ||
    (pwd && DeeObject_AssertTypeExact(pwd,&DeeString_Type)))
     return NULL;
 return fs_pathabs(path,pwd);
}
INTERN DREF DeeObject *DCALL
f_libfs_relpath(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
 DeeObject *path,*pwd = NULL;
 if (DeeArg_UnpackKw(argc,argv,kw,path_pwd_kwlist,"o|o:relpath",&path,&pwd) ||
     DeeObject_AssertTypeExact(path,&DeeString_Type) ||
    (pwd && DeeObject_AssertTypeExact(pwd,&DeeString_Type)))
     return NULL;
 return fs_pathrel(path,pwd);
}
INTERN DREF DeeObject *DCALL
f_libfs_isabs(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
 DeeObject *path;
 if (DeeArg_UnpackKw(argc,argv,kw,path_kwlist,"o:isabs",&path) ||
     DeeObject_AssertTypeExact(path,&DeeString_Type))
     return NULL;
 return_bool(fs_pathisabs(path));
}
INTERN DREF DeeObject *DCALL
f_libfs_isrel(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
 DeeObject *path;
 if (DeeArg_UnpackKw(argc,argv,kw,path_kwlist,"o:isrel",&path) ||
     DeeObject_AssertTypeExact(path,&DeeString_Type))
     return NULL;
 return_bool(!fs_pathisabs(path));
}
INTDEF DREF DeeObject *DCALL
f_libfs_issep(size_t argc, DeeObject **__restrict argv)
#ifdef CONFIG_HOST_WINDOWS
{
 DeeObject *str;
 if (DeeArg_Unpack(argc,argv,"o:issep",&str) ||
     DeeObject_AssertTypeExact(str,&DeeString_Type))
     return NULL;
 return_bool(DeeString_SIZE(str) == 0 &&
            (DeeString_STR(str)[0] == '\\' ||
             DeeString_STR(str)[0] == '/'));
}
INTERN DEFINE_STRING(libfs_sep,"\\");
INTERN DEFINE_STRING(libfs_altsep,"/");
INTERN DEFINE_STRING(libfs_delim,";");
#else
{
 DeeObject *str;
 if (DeeArg_Unpack(argc,argv,"o:issep",&str) ||
     DeeObject_AssertTypeExact(str,&DeeString_Type))
     return NULL;
 return_bool(DeeString_SIZE(str) == 0 &&
             DeeString_STR(str)[0] == '/');
}
INTERN DEFINE_STRING(libfs_sep,"/");
#define libfs_altsep libfs_sep
INTERN DEFINE_STRING(libfs_delim,":");
#endif

#define f_libfs_joinpath fs_pathjoin

struct expand_option {
 char     eo_name; /* The single character identifying this option. */
 uint16_t eo_flag; /* One, or a set of `FS_EXPAND_F*' */
};


/*[[[deemon
#include <file>
#include <fs>
#include <util>
fs.chdir(fs.path.head(__FILE__));
local options = [7]*256;
for (local line: file.open("libfs.h")) {
    local mask,id;
    try none,mask,id = line.scanf(" # define FS_EXPAND_F%[^ ] 0x%[0-9a-fA-F] /" "* `%[^']'")...;
    catch (...) continue;
    if (#id > 1) continue;
    mask = (int)("0x"+mask);
    local bit = 0;
    while (mask) mask >>= 1,++bit;
    options[id.ord()] = bit-1;
}
print "PRIVATE uint8_t expand_options_matrix[64] = {";
print "    ",;
for (local x: util.range(64)) {
    local a = options[x*2+0];
    local b = options[x*2+1];
    local val = (b << 4)|a;
    print "0x%x%s" % (val,x != 63 ? "," : ""),;
    if ((x%16) == 15 && x != 63) print "\n    ",;
}
print;
print "};";
]]]*/
PRIVATE uint8_t expand_options_matrix[64] = {
    0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,
    0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,
    0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x72,0x77,0x77,0x77,0x77,
    0x47,0x67,0x77,0x7c,0x70,0x77,0x77,0x77,0x73,0x75,0x77,0x71,0x77,0x77,0x77,0x77
};
//[[[end]]]

/* Return the options mask for a given character. */
#define GET_OPTION(x) \
   (1 << ((x) >= 128 ? 7 : ((x)&1 ? (expand_options_matrix[(x) >> 1] >> 4) \
                                  : (expand_options_matrix[(x) >> 1] & 0xf))))


INTERN DREF DeeObject *DCALL
f_libfs_expand(size_t argc, DeeObject **__restrict argv) {
 uint16_t options = EXPAND_DEFAULT_OPTIONS;
 DREF DeeObject *path,*options_ob = NULL,*env_ob = NULL;
 if (DeeArg_Unpack(argc,argv,"o|oo:expand",&path,&options_ob,&env_ob) ||
     DeeObject_AssertTypeExact(path,&DeeString_Type))
     goto err;
 if (options_ob) {
  if (DeeString_Check(options_ob)) {
   char const *iter;
   options = 0;
   iter = DeeString_STR(options_ob);
   /* Parse the given options-string. */
   while (*iter) {
    uint8_t id = (uint8_t)*iter++;
    uint16_t opt = GET_OPTION(id);
    if unlikely(opt == 0x8000)
       goto err_invalid_opt;
    options |= opt;
   }
  } else if (!DeeInt_Check(options_ob) && !env_ob) {
   /* `expand(path:?Dstring, sequence env_mapping)' */
   env_ob = options_ob;
  } else {
   /* `expand(path:?Dstring, int options, sequence env_mapping = environ)' */
   if (DeeObject_AsUInt16(options_ob,&options))
       goto err;
  }
 }
 /* Use environment variables by default. */
 if (!env_ob) env_ob = &DeeEnv_Singleton;
 return fs_pathexpand(path,options,env_ob);
err_invalid_opt:
 DeeError_Throwf(&DeeError_ValueError,
                 "Invalid expand options %r",
                 options_ob);
err:
 return NULL;
}

#ifdef CONFIG_HOST_WINDOWS
PRIVATE DREF DeeObject *DCALL
f_libfs_nt_fixunc(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
 DeeObject *path;
 if (DeeArg_UnpackKw(argc,argv,kw,path_kwlist,"o:fixunc_np",&path) ||
     DeeObject_AssertTypeExact(path,&DeeString_Type))
     return NULL;
 return nt_FixUncPath(path);
}
PRIVATE DEFINE_KWCMETHOD(libfs_fixunc_np,&f_libfs_nt_fixunc);
PRIVATE DREF DeeObject *DCALL
f_libfs_chattr_np(size_t argc, DeeObject **__restrict argv) {
 DeeObject *arg,*mode;
 if (DeeArg_Unpack(argc,argv,"oo:chattr_np",&arg,&mode) ||
     fs_chattr_np(arg,mode))
     return NULL;
 return_none;
}
PRIVATE DEFINE_CMETHOD(libfs_chattr_np,&f_libfs_chattr_np);
#endif /* CONFIG_HOST_WINDOWS */

PRIVATE DEFINE_CMETHOD(libfs_gettmp,&f_libfs_gettmp);
PRIVATE DEFINE_CMETHOD(libfs_getcwd,&f_libfs_getcwd);
PRIVATE DEFINE_KWCMETHOD(libfs_chdir,&f_libfs_chdir);
PRIVATE DEFINE_CMETHOD(libfs_gethostname,&f_libfs_gethostname);
PRIVATE DEFINE_KWCMETHOD(libfs_headof,&f_libfs_headof);
PRIVATE DEFINE_KWCMETHOD(libfs_tailof,&f_libfs_tailof);
PRIVATE DEFINE_KWCMETHOD(libfs_fileof,&f_libfs_fileof);
PRIVATE DEFINE_KWCMETHOD(libfs_extof,&f_libfs_extof);
PRIVATE DEFINE_KWCMETHOD(libfs_driveof,&f_libfs_driveof);
PRIVATE DEFINE_KWCMETHOD(libfs_inctrail,&f_libfs_inctrail);
PRIVATE DEFINE_KWCMETHOD(libfs_exctrail,&f_libfs_exctrail);
PRIVATE DEFINE_KWCMETHOD(libfs_abspath,&f_libfs_abspath);
PRIVATE DEFINE_KWCMETHOD(libfs_relpath,&f_libfs_relpath);
PRIVATE DEFINE_KWCMETHOD(libfs_isabs,&f_libfs_isabs);
PRIVATE DEFINE_KWCMETHOD(libfs_isrel,&f_libfs_isrel);
PRIVATE DEFINE_CMETHOD(libfs_issep,&f_libfs_issep);
PRIVATE DEFINE_CMETHOD(libfs_joinpath,&f_libfs_joinpath);
PRIVATE DEFINE_CMETHOD(libfs_expand,&f_libfs_expand);


/* Filesystem write operations. */
PRIVATE DREF DeeObject *DCALL
f_libfs_chtime(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
 DeeObject *path;
 DeeObject *atime = Dee_None;
 DeeObject *mtime = Dee_None;
 DeeObject *ctime = Dee_None;
 PRIVATE struct keyword kwlist[] = { K(path), K(atime), K(mtime), K(ctime), KEND };
 if (DeeArg_UnpackKw(argc,argv,kw,kwlist,"o|ooo:chtime",&path,&atime,&mtime,&ctime))
     goto err;
 if (fs_chtime(path,atime,mtime,ctime))
     goto err;
 return_none;
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
f_libfs_chmod(size_t argc, DeeObject **__restrict argv) {
 DeeObject *path,*mode;
 if (DeeArg_Unpack(argc,argv,"oo:chmod",&path,&mode))
     goto err;
 if (fs_chmod(path,mode))
     goto err;
 return_none;
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
f_libfs_lchmod(size_t argc, DeeObject **__restrict argv) {
 DeeObject *path,*mode;
 if (DeeArg_Unpack(argc,argv,"oo:lchmod",&path,&mode))
     goto err;
 if (fs_lchmod(path,mode))
     goto err;
 return_none;
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
f_libfs_chown(size_t argc, DeeObject **__restrict argv) {
 DeeObject *path,*user,*group;
 if (DeeArg_Unpack(argc,argv,"ooo:chown",&path,&user,&group))
     goto err;
 if (fs_chown(path,user,group))
     goto err;
 return_none;
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
f_libfs_lchown(size_t argc, DeeObject **__restrict argv) {
 DeeObject *path,*user,*group;
 if (DeeArg_Unpack(argc,argv,"ooo:lchown",&path,&user,&group))
     goto err;
 if (fs_lchown(path,user,group))
     goto err;
 return_none;
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
f_libfs_mkdir(size_t argc, DeeObject **__restrict argv) {
 DeeObject *path,*perm = Dee_None;
 if (DeeArg_Unpack(argc,argv,"o|o:mkdir",&path,&perm))
     goto err;
 if (fs_mkdir(path,perm))
     goto err;
 return_none;
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
f_libfs_rmdir(size_t argc, DeeObject **__restrict argv) {
 DeeObject *path;
 if (DeeArg_Unpack(argc,argv,"o:rmdir",&path))
     goto err;
 if (fs_rmdir(path))
     goto err;
 return_none;
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
f_libfs_unlink(size_t argc, DeeObject **__restrict argv) {
 DeeObject *path;
 if (DeeArg_Unpack(argc,argv,"o:unlink",&path))
     goto err;
 if (fs_unlink(path))
     goto err;
 return_none;
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
f_libfs_remove(size_t argc, DeeObject **__restrict argv) {
 DeeObject *path;
 if (DeeArg_Unpack(argc,argv,"o:remove",&path))
     goto err;
 if (fs_remove(path))
     goto err;
 return_none;
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
f_libfs_rename(size_t argc, DeeObject **__restrict argv) {
 DeeObject *existing_path,*new_path;
 if (DeeArg_Unpack(argc,argv,"oo:rename",&existing_path,&new_path))
     goto err;
 if (fs_rename(existing_path,new_path))
     goto err;
 return_none;
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
f_libfs_copyfile(size_t argc, DeeObject **__restrict argv) {
 DeeObject *existing_file,*new_file,*progress_callback = Dee_None;
 if (DeeArg_Unpack(argc,argv,"oo|o:copyfile",&existing_file,&new_file,&progress_callback))
     goto err;
 if (fs_copyfile(existing_file,new_file,progress_callback))
     goto err;
 return_none;
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
f_libfs_link(size_t argc, DeeObject **__restrict argv) {
 DeeObject *existing_path,*new_path;
 if (DeeArg_Unpack(argc,argv,"oo:link",&existing_path,&new_path))
     goto err;
 if (fs_link(existing_path,new_path))
     goto err;
 return_none;
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
f_libfs_symlink(size_t argc, DeeObject **__restrict argv) {
 DeeObject *target_text,*link_path; bool format_target = true;
 if (DeeArg_Unpack(argc,argv,"oo|b:symlink",&target_text,&link_path,&format_target))
     goto err;
 if (fs_symlink(target_text,link_path,format_target))
     goto err;
 return_none;
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
f_libfs_readlink(size_t argc, DeeObject **__restrict argv) {
 DeeObject *path;
 if (DeeArg_Unpack(argc,argv,"o:readlink",&path))
     goto err;
 return fs_readlink(path);
err:
 return NULL;
}

PRIVATE DEFINE_KWCMETHOD(libfs_chtime,&f_libfs_chtime);
PRIVATE DEFINE_CMETHOD(libfs_chmod,&f_libfs_chmod);
PRIVATE DEFINE_CMETHOD(libfs_lchmod,&f_libfs_lchmod);
PRIVATE DEFINE_CMETHOD(libfs_chown,&f_libfs_chown);
PRIVATE DEFINE_CMETHOD(libfs_lchown,&f_libfs_lchown);
PRIVATE DEFINE_CMETHOD(libfs_mkdir,&f_libfs_mkdir);
PRIVATE DEFINE_CMETHOD(libfs_rmdir,&f_libfs_rmdir);
PRIVATE DEFINE_CMETHOD(libfs_unlink,&f_libfs_unlink);
PRIVATE DEFINE_CMETHOD(libfs_remove,&f_libfs_remove);
PRIVATE DEFINE_CMETHOD(libfs_rename,&f_libfs_rename);
PRIVATE DEFINE_CMETHOD(libfs_copyfile,&f_libfs_copyfile);
PRIVATE DEFINE_CMETHOD(libfs_link,&f_libfs_link);
PRIVATE DEFINE_CMETHOD(libfs_symlink,&f_libfs_symlink);
PRIVATE DEFINE_CMETHOD(libfs_readlink,&f_libfs_readlink);



/*[[[deemon
local names = {
    "IFMT", "IFDIR", "IFCHR", "IFBLK", "IFREG",
    "IFIFO", "IFLNK", "IFSOCK", "ISUID", "ISGID",
    "ISVTX", "IRUSR", "IWUSR", "IXUSR", "IRGRP",
    "IWGRP", "IXGRP", "IROTH", "IWOTH", "IXOTH"
};
import * from _dexutils;
include("constants.def");
for (local x: names)
    gi("S_"+x,"STAT_"+x);
]]]*/
#include "constants.def"
//[[[end]]]



/* Stat helper functions. */
INTERN DREF DeeObject *DCALL
f_libfs_S_ISDIR(size_t argc, DeeObject **__restrict argv) {
 uint16_t arg;
 if (DeeArg_Unpack(argc,argv,"I16u:S_ISDIR",&arg))
     return NULL;
#ifdef S_ISDIR
 return_bool(S_ISDIR(arg));
#else
 return_bool((arg&STAT_IFMT) == STAT_IFDIR);
#endif
}
INTERN DREF DeeObject *DCALL
f_libfs_S_ISCHR(size_t argc, DeeObject **__restrict argv) {
 uint16_t arg;
 if (DeeArg_Unpack(argc,argv,"I16u:S_ISCHR",&arg))
     return NULL;
#ifdef S_ISCHR
 return_bool(S_ISCHR(arg));
#else
 return_bool((arg&STAT_IFMT) == STAT_IFCHR);
#endif
}
INTERN DREF DeeObject *DCALL
f_libfs_S_ISBLK(size_t argc, DeeObject **__restrict argv) {
 uint16_t arg;
 if (DeeArg_Unpack(argc,argv,"I16u:S_ISBLK",&arg))
     return NULL;
#ifdef S_ISBLK
 return_bool(S_ISBLK(arg));
#else
 return_bool((arg&STAT_IFMT) == STAT_IFBLK);
#endif
}
INTERN DREF DeeObject *DCALL
f_libfs_S_ISREG(size_t argc, DeeObject **__restrict argv) {
 uint16_t arg;
 if (DeeArg_Unpack(argc,argv,"I16u:S_ISREG",&arg))
     return NULL;
#ifdef S_ISREG
 return_bool(S_ISREG(arg));
#else
 return_bool((arg&STAT_IFMT) == STAT_IFREG);
#endif
}
INTERN DREF DeeObject *DCALL
f_libfs_S_ISFIFO(size_t argc, DeeObject **__restrict argv) {
 uint16_t arg;
 if (DeeArg_Unpack(argc,argv,"I16u:S_ISFIFO",&arg))
     return NULL;
#ifdef S_ISFIFO
 return_bool(S_ISFIFO(arg));
#else
 return_bool((arg&STAT_IFMT) == STAT_IFIFO);
#endif
}
INTERN DREF DeeObject *DCALL
f_libfs_S_ISLNK(size_t argc, DeeObject **__restrict argv) {
 uint16_t arg;
 if (DeeArg_Unpack(argc,argv,"I16u:S_ISLNK",&arg))
     return NULL;
#ifdef S_ISLNK
 return_bool(S_ISLNK(arg));
#else
 return_bool((arg&STAT_IFMT) == STAT_IFLNK);
#endif
}
INTERN DREF DeeObject *DCALL
f_libfs_S_ISSOCK(size_t argc, DeeObject **__restrict argv) {
 uint16_t arg;
 if (DeeArg_Unpack(argc,argv,"I16u:S_ISSOCK",&arg))
     return NULL;
#ifdef S_ISSOCK
 return_bool(S_ISSOCK(arg));
#else
 return_bool((arg&STAT_IFMT) == STAT_IFSOCK);
#endif
}

PRIVATE DEFINE_CMETHOD(libfs_S_ISDIR,&f_libfs_S_ISDIR);
PRIVATE DEFINE_CMETHOD(libfs_S_ISCHR,&f_libfs_S_ISCHR);
PRIVATE DEFINE_CMETHOD(libfs_S_ISBLK,&f_libfs_S_ISBLK);
PRIVATE DEFINE_CMETHOD(libfs_S_ISREG,&f_libfs_S_ISREG);
PRIVATE DEFINE_CMETHOD(libfs_S_ISFIFO,&f_libfs_S_ISFIFO);
PRIVATE DEFINE_CMETHOD(libfs_S_ISLNK,&f_libfs_S_ISLNK);
PRIVATE DEFINE_CMETHOD(libfs_S_ISSOCK,&f_libfs_S_ISSOCK);

#define INIT_FS_ERROR(name,doc,parent,members) \
{ \
    OBJECT_HEAD_INIT(&DeeType_Type), \
    /* .tp_name     = */name, \
    /* .tp_doc      = */DOC(doc), \
    /* .tp_flags    = */TP_FNORMAL|TP_FINHERITCTOR, \
    /* .tp_weakrefs = */0, \
    /* .tp_features = */TF_NONE, \
    /* .tp_base     = */parent, \
    /* .tp_init = */{ \
        { \
            /* .tp_alloc = */{ \
                /* .tp_ctor      = */NULL, \
                /* .tp_copy_ctor = */NULL, \
                /* .tp_deep_ctor = */NULL, \
                /* .tp_any_ctor  = */NULL, \
                TYPE_FIXED_ALLOCATOR(DeeSystemErrorObject) \
            } \
        }, \
        /* .tp_dtor        = */NULL, \
        /* .tp_assign      = */NULL, \
        /* .tp_move_assign = */NULL \
    }, \
    /* .tp_cast = */{ \
        /* .tp_str  = */NULL, \
        /* .tp_repr = */NULL, \
        /* .tp_bool = */NULL \
    }, \
    /* .tp_call          = */NULL, \
    /* .tp_visit         = */NULL, \
    /* .tp_gc            = */NULL, \
    /* .tp_math          = */NULL, \
    /* .tp_cmp           = */NULL, \
    /* .tp_seq           = */NULL, \
    /* .tp_iter_next     = */NULL, \
    /* .tp_attr          = */NULL, \
    /* .tp_with          = */NULL, \
    /* .tp_buffer        = */NULL, \
    /* .tp_methods       = */NULL, \
    /* .tp_getsets       = */NULL, \
    /* .tp_members       = */NULL, \
    /* .tp_class_methods = */NULL, \
    /* .tp_class_getsets = */NULL, \
    /* .tp_class_members = */members \
}
INTERN DeeTypeObject DeeError_NoDirectory =
INIT_FS_ERROR("NoDirectory",
              "An error derived from :FileNotFound that is thrown when a "
              "directory was expected, but something different was found",
             &DeeError_FileNotFound,NULL);
INTERN DeeTypeObject DeeError_IsDirectory =
INIT_FS_ERROR("IsDirectory",
              "An error derived from :FileExists that is thrown when something "
              "other than a directory was expected, but one was found none-the-less",
             &DeeError_FileExists,NULL);
INTERN DeeTypeObject DeeError_CrossDevice =
INIT_FS_ERROR("CrossDevice",
              "An error derived from :FSError that is thrown when attempting "
              "to move a file between different devices or partitions",
             &DeeError_FSError,NULL);
INTERN DeeTypeObject DeeError_NotEmpty =
INIT_FS_ERROR("NotEmpty",
              "An error derived from :FSError that is thrown when "
              "attempting to remove a directory that isn't empty",
             &DeeError_FSError,NULL);
INTERN DeeTypeObject DeeError_BusyFile =
INIT_FS_ERROR("BusyFile",
              "An error derived from :FSError that is thrown when "
              "attempting to remove a file or directory that is being "
              "used by another process",
             &DeeError_FSError,NULL);
INTERN DeeTypeObject DeeError_NoLink =
INIT_FS_ERROR("NoLink",
              "An error derived from :FileNotFound that is thrown when attempting "
              "to invoke :readlink on a file that isn't a symbolic link",
             &DeeError_FileNotFound,NULL);


PRIVATE struct dex_symbol symbols[] = {
    { "stat", (DeeObject *)&DeeStat_Type, MODSYM_FNORMAL },
    { "lstat", (DeeObject *)&DeeLStat_Type, MODSYM_FNORMAL },
    { "user", (DeeObject *)&DeeUser_Type, MODSYM_FNORMAL },
    { "group", (DeeObject *)&DeeGroup_Type, MODSYM_FNORMAL },
    { "NoDirectory", (DeeObject *)&DeeError_NoDirectory, MODSYM_FNORMAL },
    { "IsDirectory", (DeeObject *)&DeeError_IsDirectory, MODSYM_FNORMAL },
    { "CrossDevice", (DeeObject *)&DeeError_CrossDevice, MODSYM_FNORMAL },
    { "NotEmpty", (DeeObject *)&DeeError_NotEmpty, MODSYM_FNORMAL },
    { "BusyFile", (DeeObject *)&DeeError_BusyFile, MODSYM_FNORMAL },
    { "NoLink", (DeeObject *)&DeeError_NoLink, MODSYM_FNORMAL },
    { "dir", (DeeObject *)&DeeDir_Type, MODSYM_FNORMAL },
    { "query", (DeeObject *)&DeeQuery_Type, MODSYM_FNORMAL },
    { "environ", &DeeEnv_Singleton, MODSYM_FNORMAL,
      DOC("->?S?T2?Dstring?Dstring\n"
          "A :mapping-style singleton instance that can be used to "
          "access and enumerate environment variables by name:\n"
          ">print environ[\"PATH\"]; /* \"/bin:/usr/bin:...\" */\n"
          "Other mapping operations known from :dict can be used "
          "to delete (${del environ[...]}), set (${environ[...] = ...}) and "
          "check for the existance of (${... in environ}) environment variables, "
          "as well as enumerating all variables (${for (key,item: environ) ...})") },
    { "gettmp", (DeeObject *)&libfs_gettmp, MODSYM_FNORMAL,
      DOC("->?Dstring\n"
          "@interrupt\n"
          "@throw SystemError Failed to retrieve a temporary path name for some reason\n"
          "Return the path to a folder that can be used as "
          "temporary storage of files and directories") },
    { "getcwd", (DeeObject *)&libfs_getcwd, MODSYM_FNORMAL,
      DOC("->?Dstring\n"
          "@interrupt\n"
          "@throw FileAccessError Permission to read a part of the current working directory's path was denied\n"
          "@throw FileNotFound The current working directory has been unlinked\n"
          "@throw SystemError Failed to retrieve the current working directory for some reason\n"
          "Return the absolute path of the current working directory") },
    { "gethostname", (DeeObject *)&libfs_gethostname, MODSYM_FNORMAL,
      DOC("->?Dstring\n"
          "@interrupt\n"
          "@throw SystemError Failed to retrieve the name of the hosting machine for some reason\n"
          "Returns the user-assigned name of the hosting machine") },
    { "chdir", (DeeObject *)&libfs_chdir, MODSYM_FNORMAL,
      DOC("(path:?Dstring)\n"
          "(fp:?Dfile)\n"
          "(fd:?Dint)\n"
          "@interrupt\n"
          "@throw FileNotFound The given @path could not be found\n"
          "@throw NoDirectory The given @path is not a directory\n"
          "@throw FileClosed The given @fp or @fd has been closed or is invalid\n"
          "@throw FileAccessError The current user does not have permissions to enter @path\n"
          "@throw SystemError Failed to change the current working directory for some reason\n"
          "Change the current working directory to @path, which may be a path "
          "relative to the old current working directory") },
    { "chtime", (DeeObject *)&libfs_chtime, MODSYM_FNORMAL,
      DOC("(path:?Dstring,atime:?Etime:Time=!N,mtime:?Etime:Time=!N,ctime:?Etime:Time=!N)\n"
          "(fp:?Dfile,atime:?Etime:Time=!N,mtime:?Etime:Time=!N,ctime:?Etime:Time=!N)\n"
          "(fd:?Dint,atime:?Etime:Time=!N,mtime:?Etime:Time=!N,ctime:?Etime:Time=!N)\n"
          "@interrupt\n"
          "@throw FileNotFound The given @path could not be found\n"
          "@throw NoDirectory A part of the given @path is not a directory\n"
          "@throw FileClosed The given file @fp was closed, or the given handle @fd is invalid\n"
          "@throw FileAccessError The current user does not have permissions to "
                             "change at least one of the given timestamps. "
                             "Note that some hosts (linux) don't allow "
                             "modification of the creation time, in which "
                             "case any attempt to change it will result "
                             "in this error being thrown\n"
          "@throw ReadOnlyFile The filesystem or device hosting the file found under "
                          "@path is in read-only operations mode, preventing the "
                          "file's timestamps from being changed\n"
          "@throw SystemError Failed to change time for some reason\n"
          "Change the timestamps associated with the given @path") },
    { "chmod", (DeeObject *)&libfs_chmod, MODSYM_FNORMAL,
      DOC("(path:?Dstring,mode:?X2?Dstring?Dint)\n"
          "(fp:?Dfile,mode:?X2?Dstring?Dint)\n"
          "(fd:?Dint,mode:?X2?Dstring?Dint)\n"
          "@interrupt\n"
          "@throw FileNotFound The given @path could not be found\n"
          "@throw NoDirectory A part of the given @path is not a directory\n"
          "@throw FileClosed The given file @fp was closed, or the given handle @fd is invalid\n"
          "@throw FileAccessError The current user does not have permissions "
                             "to change the mode of the given file @path\n"
          "@throw ReadOnlyFile The filesystem or device hosting the file found under "
                          "@path is in read-only operations mode, preventing the "
                          "file's mode from being changed\n"
          "@throw SystemError Failed to change permission for some reason\n"
          "@throw ValueError The given @mode is malformed or not recognized\n"
          "Change the permissions associated with a given @path") },
    { "lchmod", (DeeObject *)&libfs_lchmod, MODSYM_FNORMAL,
      DOC("(path:?Dstring,mode:?X2?Dstring?Dint)\n"
          "@interrupt\n"
          "@throw FileNotFound The given @path could not be found\n"
          "@throw NoDirectory A part of the given @path is not a directory\n"
          "@throw FileAccessError The current user does not have permissions "
                             "to change the mode of the given file @path\n"
          "@throw ReadOnlyFile The filesystem or device hosting the file found under "
                          "@path is in read-only operations mode, preventing the "
                          "file's mode from being changed\n"
          "@throw SystemError Failed to change permission for some reason\n"
          "@throw ValueError The given @mode is malformed or not recognized\n"
          "Change the permissions associated with a given @path\n"
          "If @path referrs to a symbolic link, change the permissions "
          "of that link, rather than those of the pointed-to file") },
    { "chown", (DeeObject *)&libfs_chown, MODSYM_FNORMAL,
      DOC("(path:?Dstring,user:?X3?Guser?Dstring?Dint,group:?X3?Ggroup?Dstring?Dint)\n"
          "(fp:?Dfile,user:?X3?Guser?Dstring?Dint,group:?X3?Ggroup?Dstring?Dint)\n"
          "(fd:?Dint,user:?X3?Guser?Dstring?Dint,group:?X3?Ggroup?Dstring?Dint)\n"
          "@interrupt\n"
          "@throw FileNotFound The given @path could not be found\n"
          "@throw NoDirectory A part of the given @path is not a directory\n"
          "@throw FileClosed The given file @fp was closed, or the given handle @fd is invalid\n"
          "@throw FileAccessError The current user does not have permissions "
                             "to change the ownership of the given file @path\n"
          "@throw ReadOnlyFile The filesystem or device hosting the file found under "
                          "@path is in read-only operations mode, preventing the "
                          "file's ownership from being changed\n"
          "@throw ValueError The given @user or @group could not be found\n"
          "@throw SystemError Failed to change ownership for some reason\n"
          "Change the ownership of a given @path") },
    { "lchown", (DeeObject *)&libfs_lchown, MODSYM_FNORMAL,
      DOC("(path:?Dstring,user:?X3?Guser?Dstring?Dint,group:?X3?Ggroup?Dstring?Dint)\n"
          "@interrupt\n"
          "@throw FileNotFound The given @path could not be found\n"
          "@throw NoDirectory A part of the given @path is not a directory\n"
          "@throw FileAccessError The current user does not have permissions "
                             "to change the ownership of the given file @path\n"
          "@throw ReadOnlyFile The filesystem or device hosting the file found under "
                          "@path is in read-only operations mode, preventing the "
                          "file's ownership from being changed\n"
          "@throw ValueError The given @user or @group could not be found\n"
          "@throw SystemError Failed to change ownership for some reason\n"
          "Change the ownership of a given @path\n"
          "If @path referrs to a symbolic link, change the ownership "
          "of that link, rather than those of the pointed-to file") },
    { "mkdir", (DeeObject *)&libfs_mkdir, MODSYM_FNORMAL,
      DOC("(path:?Dstring,permissions:?X2?Dstring?Dint=!N)\n"
          "@interrupt\n"
          "@throw FileNotFound One or more of @path's parents do not exist\n"
          "@throw NoDirectory A part of the given @path is not a directory\n"
          "@throw FileExists The given @path already exists\n"
          "@throw ValueError The given @permissions are malformed or not recognized\n"
          "@throw FileAccessError The current user does not have permissions to "
                             "create a new directory within the folder of @path\n"
          "@throw ReadOnlyFile The filesystem or device hosting the directory of "
                          "@path is in read-only operations mode, preventing the "
                          "creation of new directories\n"
          "@throw SystemError Failed to create a directory for some reason\n"
          "Create a new directory named @path") },
    { "rmdir", (DeeObject *)&libfs_rmdir, MODSYM_FNORMAL,
      DOC("(path:?Dstring)\n"
          "@interrupt\n"
          "@throw FileNotFound The given @path does not exist\n"
          "@throw NoDirectory A part of the given @path is not a directory\n"
          "@throw NoDirectory The given @path isn't a directory and either :unlink or :remove must be used to "
                               "delete it, or it is a mounting point if such functionality is supported by the host\n"
          "@throw NotEmpty The given directory @path isn't empty empty\n"
          "@throw BusyFile The given @path is currently being used and cannot be deleted\n"
          "@throw FileAccessError The current user does not have permissions to "
                             "remove the directory described by @path\n"
          "@throw ReadOnlyFile The filesystem or device hosting the directory of "
                          "@path is in read-only operations mode, preventing the "
                          "deletion of existing directories\n"
          "@throw SystemError Failed to delete the directory @path for some reason\n"
          "Remove a directory named @path") },
    { "unlink", (DeeObject *)&libfs_unlink, MODSYM_FNORMAL,
      DOC("(path:?Dstring)\n"
          "@interrupt\n"
          "@throw FileNotFound The given @path does not exist\n"
          "@throw NoDirectory A part of the given @path is not a directory\n"
          "@throw IsDirectory The given @path is a directory and :rmdir or :remove must be used to delete it\n"
          "@throw BusyFile The given @path is currently being used and cannot be deleted\n"
          "@throw FileAccessError The current user does not have permissions to "
                             "remove the file described by @path\n"
          "@throw ReadOnlyFile The filesystem or device hosting the directory of "
                          "@path is in read-only operations mode, preventing the "
                          "deletion of existing files\n"
          "@throw SystemError Failed to unlink the given file @path for some reason\n"
          "Remove a non-directory filesystem object named @path") },
    { "remove", (DeeObject *)&libfs_remove, MODSYM_FNORMAL,
      DOC("(path:?Dstring)\n"
          "@interrupt\n"
          "@throw FileNotFound The given @path does not exist\n"
          "@throw NoDirectory A part of the given @path is not a directory\n"
          "@throw NotEmpty The given @path is a directory that isn't empty empty\n"
          "@throw BusyFile The given @path is currently being used and cannot be deleted\n"
          "@throw FileAccessError The current user does not have permissions to "
                             "remove the file or directory described by @path\n"
          "@throw ReadOnlyFile The filesystem or device hosting the directory of "
                          "@path is in read-only operations mode, preventing the "
                          "deletion of existing files or directories\n"
          "@throw SystemError Failed to remove the given file @path for some reason\n"
          "Remove a file or an empty directory name @path") },
    { "rename", (DeeObject *)&libfs_rename, MODSYM_FNORMAL,
      DOC("(existing_path:?Dstring,new_path:?Dstring)\n"
          "@interrupt\n"
          "@throw FileNotFound The given @existing_path could not be found, or a parent directory of @new_path does not exist\n"
          "@throw NoDirectory A part of the given @path is not a directory\n"
          "@throw CrossDevice The given @existing_path and @new_path are not located on the same device\n"
          "@throw ValueError Attempted to move a directory into itself\n"
          "@throw FileAccessError The current user does not have permissions to "
                             "access the file or directory @existing_path, or the "
                             "directory containing the non-existant file @new_path\n"
          "@throw ReadOnlyFile The filesystem or device hosting the given file or directory "
                          "@existing_path or the directory containing the non-existant "
                          "file @new_path is in read-only operations mode, preventing "
                          "the modification of existing files or directories\n"
          "@throw SystemError Failed to rename the given @existing_path for some reason\n"
          "Renames or moves a given @existing_path to be referred to as @new_path from then on") },
    { "copyfile", (DeeObject *)&libfs_copyfile, MODSYM_FNORMAL,
      DOC("(existing_file:?X3?Dstring?Dfile?Dint,new_file:?X3?Dstring?Dfile?Dint,progress:?Dcallable=!N)\n"
          "@interrupt\n"
          "@throw FileExists The given @new_file already exists\n"
          "@throw NoDirectory A part of the given @existing_file or @new_file is not a directory\n"
          "@throw FileNotFound The given @existing_file could not be found, or a parent directory of @new_file does not exist\n"
          "@throw IsDirectory The given @existing_file is a directory\n"
          "@throw FileAccessError The current user does not have permissions to access "
                             "the file @existing_file for reading, or the directory "
                             "containing the non-existant file @new_file for writing\n"
          "@throw ReadOnlyFile The filesystem or device hosting the directory "
                          "containing the non-existant file @new_file is "
                          "in read-only operations mode, preventing the "
                          "creation of new files\n"
          "@throw SystemError Failed to copy the given @existing_file for some reason\n"
          "Copies a given @existing_file to be create a duplicate named @new_file\n"
          "When given, @progress is invoked periodically during the copy operation using "
          "${operator()} while assing an internal progress decriptor object implementing "
          "the following functions:\n"
          "%{table Function name|Behavior\n"
          "${operator float}|Returns the overall progress as a floating-point value between ${0.0} and ${1.0}\n"
          "${member siz -> int}|The total number of bytes that must be copied\n"
          "${member cur -> int}|The total number of bytes already copied\n"
          "${member rem -> int}|The total number of bytes that must still be copied}\n"
          "The callback may throw an error to abort the copy operation, which is then "
          "propagated after the partially copied file may have been deleted, based on "
          "the host operating system's preferrance") },
    { "link", (DeeObject *)&libfs_link, MODSYM_FNORMAL,
      DOC("(existing_path:?X3?Dstring?Dfile?Dint,new_path:?Dstring)\n"
          "@interrupt\n"
          "@throw FileNotFound The given @existing_path could not be found, or a parent directory of @new_path does not exist\n"
          "@throw NoDirectory A part of the given @existing_path or @new_path is not a directory\n"
          "@throw UnsupportedAPI The underlying filesystem hosting @existing_path and @new_pathdoes not support hard links\n"
          "@throw CrossDevice The given @existing_path and @new_path are not apart of the same drive\n"
          "@throw FileAccessError The current user does not have permissions to access "
                             "the file or directory @existing_path for reading, or the "
                             "directory containing the non-existant object @new_path for writing\n"
          "@throw ReadOnlyFile The filesystem or device hosting the directory "
                          "containing the non-existant object @new_path is "
                          "in read-only operations mode, preventing the "
                          "addition of file or directory links\n"
          "@throw SystemError Failed to create the link to @existing_path for some reason\n"
          "Create a new hard link pointing to @existing_path as a file named @new_path\n"
          "Hard links are similar to symbolic links, yet cannot be used across multiple "
          "devices and are unaffected by mount locations. A hard link simply create a new "
          "directory entry under @new_path that points to the data block of an existing "
          "file @existing_path") },
    { "symlink", (DeeObject *)&libfs_symlink, MODSYM_FNORMAL,
      DOC("(target_text:?Dstring,link_path:?Dstring,format_target=!t)\n"
          "@interrupt\n"
          "@throw FileExists A file or directory named @link_path already exists\n"
          "@throw FileNotFound A parent directory of @link_path does not exist\n"
          "@throw NoDirectory A part of the given @link_path is not a directory\n"
          "@throw UnsupportedAPI The underlying filesystem does not support (or allow *cough* windows...) symbolic links\n"
          "@throw FileAccessError The current user does not have permissions to access "
                             "the directory containing the non-existant file @link_path for writing\n"
          "@throw ReadOnlyFile The filesystem or device hosting the directory "
                          "containing the non-existant object @link_path is "
                          "in read-only operations mode, preventing the "
                          "creation of new symbolic links\n"
          "@throw SystemError Failed to create a symbolic link under @link_path for some reason\n"
          "Symbolic links are filesystem redirection points which you can think of as "
          "keyword-style macros that exist in directories. When addressed, simply imagine "
          "their name being replaced with @target_text, at which point the resulting path "
          "is then re-evaluated:\n"
          ">import symlink from fs;\n"
          ">import file from deemon;\n"
          ">symlink(\"../foo\",\"/path/to/link\");\n"
          ">/* \"/path/to/[link]/file.txt\" */\n"
          ">/* \"/path/to/[../foo]/file.txt\" */\n"
          ">/* \"/path/foo/file.txt\" */\n"
          ">file.open(\"/path/to/link/file.txt\");\n"
          "Because of the fact that some filesystem support alternative path separators, "
          "those seperators may not be allowed to appear in symbolic link texts. If this "
          "is the case and if @format_target is :true, the given @target_text will be "
          "normalized to fix inconsistencies that might otherwise prevent the link from "
          "functioning properly") },
    { "readlink", (DeeObject *)&libfs_readlink, MODSYM_FNORMAL,
      DOC("(path:?Dstring)->?Dstring\n"
          "(fp:?Dfile)->?Dstring\n"
          "(fd:?Dint)->?Dstring\n"
          "@interrupt\n"
          "@throw FileNotFound The given @path does not exist\n"
          "@throw NoDirectory A part of the given @path is not a directory\n"
          "@throw NoLink The given @path does not refer to a symbolic link\n"
          "@throw ValueError The file described by @path is not a symlink\n"
          "@throw UnsupportedAPI The underlying filesystem does not support reading of symbolic links\n"
          "@throw FileAccessError The current user does not have permissions to access "
                             "@path or one of the containing directories for reading\n"
          "@throw SystemError Failed to read the symbolic link under @path for some reason\n"
          "Read and return the target_text used to create a symbolic link (see :symlink)") },
    { "headof", (DeeObject *)&libfs_headof, MODSYM_FNORMAL,
      DOC("(path:?Dstring)->?Dstring\n"
          "@return The head of a path, that is the directory without the filename\n"
          ">import headof from fs;\n"
          ">print headof(\"bar.txt\");        /* \"\" */\n"
          ">print headof(\"/foo/bar.txt\");   /* \"/foo/\" */\n"
          ">print headof(\"C:/foo/bar.txt\"); /* \"C:/foo/\" */") },
    { "tailof", (DeeObject *)&libfs_tailof, MODSYM_FNORMAL,
      DOC("(path:?Dstring)->?Dstring\n"
          "@return The tail of a path, that is the filename + extension\n"
          ">import tailof from fs;\n"
          ">print tailof(\"bar.txt\");        /* \"bar.txt\" */\n"
          ">print tailof(\"/foo/bar.txt\");   /* \"bar.txt\" */\n"
          ">print tailof(\"C:/foo/bar.txt\"); /* \"bar.txt\" */") },
    { "fileof", (DeeObject *)&libfs_fileof, MODSYM_FNORMAL,
      DOC("(path:?Dstring)->?Dstring\n"
          "@return The file portion of a path, excluding the file extension\n"
          ">import fileof from fs;\n"
          ">print fileof(\"bar.txt\");        /* \"bar\" */\n"
          ">print fileof(\"/foo/bar.txt\");   /* \"bar\" */\n"
          ">print fileof(\"C:/foo/bar.txt\"); /* \"bar\" */") },
    { "extof", (DeeObject *)&libfs_extof, MODSYM_FNORMAL,
      DOC("(path:?Dstring)->?Dstring\n"
          "@return The extension of a path, including the leading $\".\" character\n"
          ">import extof from fs;\n"
          ">print extof(\"bar.txt\");        /* \".txt\" */\n"
          ">print extof(\"/foo/bar.txt\");   /* \".txt\" */\n"
          ">print extof(\"C:/foo/bar.txt\"); /* \".txt\" */") },
    { "driveof", (DeeObject *)&libfs_driveof, MODSYM_FNORMAL,
      DOC("(path:?Dstring)->?Dstring\n"
          "@return The drive portion of an absolute path on windows, or $\"/\" on other platforms\n"
          ">import driveof from fs;\n"
          ">print driveof(\"bar.txt\");        /* \"\" or \"/\" */\n"
          ">print driveof(\"/foo/bar.txt\");   /* \"\" or \"/\" */\n"
          ">print driveof(\"C:/foo/bar.txt\"); /* \"C:/\" or \"/\" */") },
    { "inctrail", (DeeObject *)&libfs_inctrail, MODSYM_FNORMAL,
      DOC("(path:?Dstring)->?Dstring\n"
          "@return The path with a trailing slash included\n"
          ">import inctrail from fs;\n"
          ">print inctrail(\"/foo/bar/\"); /* \"/foo/bar/\" */\n"
          ">print inctrail(\"/foo/bar\");  /* \"/foo/bar/\" */") },
    { "exctrail", (DeeObject *)&libfs_exctrail, MODSYM_FNORMAL,
      DOC("(path:?Dstring)->?Dstring\n"
          "@return The path with a trailing slash excluded\n"
          ">import exctrail from fs;\n"
          ">print exctrail(\"/foo/bar/\"); /* \"/foo/bar\" */\n"
          ">print exctrail(\"/foo/bar\");  /* \"/foo/bar\" */") },
    { "abspath", (DeeObject *)&libfs_abspath, MODSYM_FNORMAL,
      DOC("(path:?Dstring,cwd=!P{.})->?Dstring\n"
          "@interrupt\n"
          "Makes @path an absolute path, using @cwd as the base point for the relative disposition\n"
          "If @path was already relative to begin with, it is forced to become relative "
          "as the result of calling #relpath with it and the return value of #getcwd\n"
          "If @cwd is relative, if will be forced to become absolute as the result of "
          "calling #abspath with @cwd as first and the return value of #getcwd as second argument\n"
          ">import abspath from fs;\n"
          ">print abspath(\"../user/bar\",\"/home/foobar\"); /* \"/home/user/bar\" */") },
    { "relpath", (DeeObject *)&libfs_relpath, MODSYM_FNORMAL,
      DOC("(path:?Dstring,cwd=!P{.})->?Dstring\n"
          "@interrupt\n"
          "Creates a relative path leading to @path and originating from @cwd\n"
          "If @path was already relative to begin with, it is forced to become absolute "
          "as the result of calling #abspath with it and the return value of #getcwd\n"
          "If @cwd is relative, if will be forced into an absolute path as the "
          "result of calling #abspath with it and the return value of #getcwd\n"
          "When running on a windows host, in the event that @path is located on a "
          "different #driveof than @cwd, @path will be re-returned as is")},
    { "isabs", (DeeObject *)&libfs_isabs, MODSYM_FNORMAL,
      DOC("(path:?Dstring)->?Dbool\n"
          "Returns :true if the given @path is considered to be absolute")  },
    { "isrel", (DeeObject *)&libfs_isrel, MODSYM_FNORMAL,
      DOC("(path:?Dstring)->?Dbool\n"
          "Returns the inverse of #isabs")  },
    { "issep", (DeeObject *)&libfs_issep, MODSYM_FNORMAL,
      DOC("(str:?Dstring)->?Dbool\n"
          "Returns :true if the given @str is recognized as a path "
          "seperator (Usually $\"/\" and/or $\"\\\")\n"
          "The host's primary and secondary seperator "
          "values can be read from #SEP and #ALTSEP")  },
    { "joinpath", (DeeObject *)&libfs_joinpath, MODSYM_FNORMAL,
      DOC("(paths!:?Dstring)->?Dstring\n"
          "Joins all @paths passed through varargs to generate a full path. "
          "For this purpose, all path elements are joined with #SEP, "
          "after removal of additional slashes and spaces surrounding the given @paths")  },
    { "expand", (DeeObject *)&libfs_expand, MODSYM_FNORMAL,
      DOC("(path:?Dstring,env:?Dmapping=!Genviron)->?Dstring\n"
          "(path:?Dstring,options:?Dstring=!Phvpf,env:?Dmapping=!Genviron)->?Dstring\n"
          "(path:?Dstring,options:?Dint,env:?Dmapping=!Genviron)->?Dstring\n"
          "@interrupt\n"
          "@param env A dict-style mapping used to resolve variable names. Defaults to :environ\n"
          "@throw ValueError The given @options string contains unrecognized options\n"
          "@throw ValueError An unknown environment variable was accessed and $\"f\" isn't part of @options\n"
          "Expand parts of the given @path, according to @options which is either an "
          "implementation-specific bitset, or a sequence of the following option characters:\n"
          "%{table Name|Behavior\n"
          "$\"h\"|Expand $\"~\" and $\"~<nam>\" to the return value of ${user([<nam>]).home()}\n"
          "$\"v\"|Expand $\"$<nam>\" and $\"${<nam>}\" to ${env[nam]}\n"
          "$\"V\"|Expand $\"%<nam>%\" to ${env[nam]}\n"
          "$\"p\"|Expand $\".\" and $\"..\" folders names while also deleting multiple consecutive "
                 "slashes, as well as all whitespace surrounding them. On hosts with an #ALTSEP differing "
                 "from #SEP, all occurrances of #ALTSEP are also replaced with #SEP. "
                 "This option, alongside $\"c\" and $\"a\" should be used before a path-string can be considered "
                 "uniform and suitable to be used as key in a hash-table used for mapping files to objects. "
                 "Note that the deemon core uses an option similar to this to implement the mapping "
                 "between modules and their associated files\n"
          "$\"a\"|Force the returned path to be absolute in relation to #getcwd\n"
          "$\"r\"|Force the returned path to be relative to #getcwd (Overrules $\"a\")\n"
          "$\"c\"|When the host filesystem is case-insensitive, force all case-sensitive parts into a "
                 "uniform casing. If the host's filesystem isn't case-sensitive, this flag is ignored\n"
          "$\"f\"|When used with $\"h\", $\"v\" or $\"V\", handle errors "
                 "for unknown environment variables or :{KeyError}s when accessing "
                 "@env, or a failure to determine the user's home directory "
                 "by not expanding that part of the path}\n"
          "Passing the same option more than once is allowed and simply ignored")  },
    { "SEP", (DeeObject *)&libfs_sep, MODSYM_FNORMAL,
      DOC("->?Dstring\n"
          "The host's primary path seperator. On windows that is "
          "$\"\\\" while on most other hosts it is $\"/\"\n"
          "If supported by the host, an alternative seperator can be read from #ALTSEP\n"
          "Additionally, a string can be testing for being a seperator by calling #issep") },
    { "ALTSEP", (DeeObject *)&libfs_sep, MODSYM_FNORMAL,
      DOC("->?Dstring\n"
          "The alternative path seperator or an alias for #SEP "
          "if the host only supports a single type of seperator") },
    { "DELIM", (DeeObject *)&libfs_delim, MODSYM_FNORMAL,
      DOC("->?Dstring\n"
          "A string used to delimit individual paths in path-listings often "
          "found in environment variables, most notably ${environ[\"PATH\"]}") },

    /* stat.st_mode bits. */
    LIBFS_S_IFMT_DEF 
    LIBFS_S_IFDIR_DEF
    LIBFS_S_IFCHR_DEF
    LIBFS_S_IFBLK_DEF
    LIBFS_S_IFREG_DEF
    LIBFS_S_IFIFO_DEF
    LIBFS_S_IFLNK_DEF
    LIBFS_S_IFSOCK_DEF
    LIBFS_S_ISUID_DEF
    LIBFS_S_ISGID_DEF
    LIBFS_S_ISVTX_DEF
    LIBFS_S_IRUSR_DEF
    LIBFS_S_IWUSR_DEF
    LIBFS_S_IXUSR_DEF
    LIBFS_S_IRGRP_DEF
    LIBFS_S_IWGRP_DEF
    LIBFS_S_IXGRP_DEF
    LIBFS_S_IROTH_DEF
    LIBFS_S_IWOTH_DEF
    LIBFS_S_IXOTH_DEF

    /* stat.st_mode helper functions. */
    { "S_ISDIR",  (DeeObject *)&libfs_S_ISDIR,  MODSYM_FREADONLY|MODSYM_FCONSTEXPR, DOC("(mode:?Dint)->?Dbool") },
    { "S_ISCHR",  (DeeObject *)&libfs_S_ISCHR,  MODSYM_FREADONLY|MODSYM_FCONSTEXPR, DOC("(mode:?Dint)->?Dbool") },
    { "S_ISBLK",  (DeeObject *)&libfs_S_ISBLK,  MODSYM_FREADONLY|MODSYM_FCONSTEXPR, DOC("(mode:?Dint)->?Dbool") },
    { "S_ISREG",  (DeeObject *)&libfs_S_ISREG,  MODSYM_FREADONLY|MODSYM_FCONSTEXPR, DOC("(mode:?Dint)->?Dbool") },
    { "S_ISFIFO", (DeeObject *)&libfs_S_ISFIFO, MODSYM_FREADONLY|MODSYM_FCONSTEXPR, DOC("(mode:?Dint)->?Dbool") },
    { "S_ISLNK",  (DeeObject *)&libfs_S_ISLNK,  MODSYM_FREADONLY|MODSYM_FCONSTEXPR, DOC("(mode:?Dint)->?Dbool") },
    { "S_ISSOCK", (DeeObject *)&libfs_S_ISSOCK, MODSYM_FREADONLY|MODSYM_FCONSTEXPR, DOC("(mode:?Dint)->?Dbool") },

#ifdef CONFIG_HOST_WINDOWS
    { "fixunc_np", (DeeObject *)&libfs_fixunc_np, MODSYM_FHIDDEN,
      DOC("(path:?Dstring)->?Dstring\n"
          "@interrupt\n"
          "Non-portable extension for fixing a given path to "
          "become a valid UNC long-path usable under windows\n"
          "The main purpose of doing this is to break the 260-character "
          "limit of regular paths under windows and extend it to a "
          "maximum of around ${2**16}")  },
    { "chattr_np", (DeeObject *)&libfs_chattr_np, MODSYM_FHIDDEN,
      DOC("(path:?Dstring,mode:?Dint)\n"
          "(fp:?Dfile,mode:?Dint)\n"
          "(fd:?Dint,mode:?Dint)\n"
          "@interrupt\n"
          "@throw FileNotFound The file specified by @path could not be found\n"
          "@throw FileAccessError You don't have permissions to change the attributes\n"
          "@throw ReadOnlyFile The filesystem hosting @path is read-only\n"
          "@throw SystemError Failed to change attributes for some reason\n"
          "Non-portable extension for changing the file attributes used by windows") },
#endif /* CONFIG_HOST_WINDOWS */
    { NULL }
};

PRIVATE char const *import_table[] = {
    /* NOTE: Indices in this table must match those used by `*_MODULE' macros! */
    "time",  /* #define TIME_MODULE   DEX.d_imports[0] */
    NULL
};


PUBLIC struct dex DEX = {
    /* .d_symbols      = */symbols,
    /* .d_init         = */NULL,
    /* .d_fini         = */NULL,
    /* .d_import_names = */{ import_table }
};

DECL_END


#endif /* !GUARD_DEX_FS_LIBFS_C */
