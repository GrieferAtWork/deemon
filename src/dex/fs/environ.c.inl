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
#ifndef GUARD_DEX_FS_ENVIRON_C_INL
#define GUARD_DEX_FS_ENVIRON_C_INL 1
#ifndef _KOS_SOURCE
#define _KOS_SOURCE     1
#endif
#ifndef _BSD_SOURCE
#define _BSD_SOURCE     1
#endif
#ifndef _GNU_SOURCE
#define _GNU_SOURCE     1
#endif
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif
#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE   500
#endif

#include <deemon/api.h>

#include "libfs.h"

#include <deemon/exec.h>
#include <deemon/string.h>
#include <deemon/error.h>
#include <deemon/tuple.h>
#include <deemon/seq.h>
#ifndef CONFIG_NO_THREADS
#include <deemon/util/rwlock.h>
#endif

#include <stdlib.h>
#include <string.h>

#ifndef __USE_KOS
#ifndef strend
#define strend(str) ((str)+strlen(str))
#endif
#endif

DECL_BEGIN

typedef struct {
    OBJECT_HEAD
    char       **e_iter;    /* [0..1][1..1] Next environment string. */
    unsigned int e_version; /* The environment version when iteration started. */
} Env;

PRIVATE DEFINE_RWLOCK(env_lock);
PRIVATE unsigned int env_version = 0;

PRIVATE char *empty_env[] = { NULL };

PRIVATE int DCALL
env_init(Env *__restrict self) {
again:
 rwlock_read(&env_lock);
 self->e_iter    = environ;
 self->e_version = env_version;
 rwlock_endread(&env_lock);
 if unlikely(!self->e_iter) {
  self->e_iter = empty_env;
  return -1;
 }
 return 0;
}

PRIVATE int DCALL
env_bool(Env *__restrict self) {
 return self->e_iter[0] != 0;
}

PRIVATE DREF DeeObject *DCALL
DeeString_TryNewSized(char const *__restrict str, size_t len) {
 DREF DeeStringObject *result;
 result = (DREF DeeStringObject *)DeeObject_TryMalloc(offsetof(DeeStringObject,s_str)+
                                                     (len+1)*sizeof(char));
 if unlikely(!result) goto done;
 DeeObject_Init(result,&DeeString_Type);
 result->s_data = NULL;
 result->s_hash = (dhash_t)-1;
 result->s_len  = len;
 memcpy(result->s_str,str,len*sizeof(char));
 result->s_str[len] = '\0';
done:
 return (DREF DeeObject *)result;
}

PRIVATE DREF DeeObject *DCALL
env_next(Env *__restrict self) {
 unsigned int my_version;
 DREF DeeObject *result_tuple;
 DREF DeeObject *name,*value;
 char **presult,*result,*valstart;
 rwlock_read(&env_lock);
 /* Check environment version number. */
 if ((my_version = self->e_version) != env_version) {
iter_done:
  rwlock_endread(&env_lock);
  return ITER_DONE;
 }
 do {
  presult = self->e_iter;
  if (!*presult) goto iter_done;
 } while (ATOMIC_CMPXCH(self->e_iter,presult,presult+1));
 result   = *presult;
 valstart = strrchr(result,'=');
 if (!valstart) valstart = strend(result);
allocate_value:
 value = DeeString_TryNewSized(valstart,strlen(valstart));
 if unlikely(!value) {
  /* Collect memory and try again. */
  rwlock_endread(&env_lock);
  if (!Dee_CollectMemory(offsetof(DeeStringObject,s_str)+
                        (strlen(valstart)+1)*sizeof(char)))
       goto err;
  rwlock_read(&env_lock);
  if (my_version != env_version)
      goto iter_done;
  goto allocate_value;
 }
 if (*valstart) --valstart;
allocate_name:
 name = DeeString_TryNewSized(result,(size_t)(valstart-result));
 if unlikely(!name) {
  /* Collect memory and try again. */
  rwlock_endread(&env_lock);
  if (!Dee_CollectMemory(offsetof(DeeStringObject,s_str)+
                        ((size_t)(valstart-result)+1)*sizeof(char)))
       goto err_value;
  rwlock_read(&env_lock);
  if (my_version != env_version) {
   Dee_Decref(value);
   goto iter_done;
  }
  goto allocate_name;
 }
 rwlock_endread(&env_lock);
 /* All right! we've managed to read the name + value! */
 result_tuple = DeeTuple_PackSymbolic(2,name,value); /* Inherit name+value on success. */
 if unlikely(!result_tuple) goto err_name;
 return result_tuple;
err_name:
 Dee_Decref(name);
err_value:
 Dee_Decref(value);
err:
 return NULL;
}

PRIVATE struct type_member env_members[] = {
    TYPE_MEMBER_CONST("seq",&DeeEnv_Singleton),
    TYPE_MEMBER_END
};

INTERN DeeTypeObject DeeEnvIterator_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"environ.iterator",
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
        /* .tp_dtor        = */NULL,
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
 DeeError_Throwf(&DeeError_ValueError,
                 "Unknown environment variable `%k'",
                 name);
}
PRIVATE void DCALL
err_unknown_env_var_s(char const *__restrict name) {
 DeeError_Throwf(&DeeError_ValueError,
                 "Unknown environment variable `%s'",
                 name);
}

INTERN bool DCALL
fs_hasenv(/*String*/DeeObject *__restrict name) {
 bool result;
 rwlock_read(&env_lock);
 DBG_ALIGNMENT_DISABLE();
 result = getenv(DeeString_STR(name)) != NULL;
 DBG_ALIGNMENT_ENABLE();
 rwlock_endread(&env_lock);
 return result;
}
INTERN DREF DeeObject *DCALL
fs_getenv(DeeObject *__restrict name, bool try_get) {
 DREF DeeObject *result;
 char *strval; size_t valsiz;
again:
 rwlock_read(&env_lock);
 DBG_ALIGNMENT_DISABLE();
 strval = getenv(DeeString_STR(name));
 DBG_ALIGNMENT_ENABLE();
 if (!strval) {
  rwlock_endread(&env_lock);
  if (!try_get) {
   DeeError_Throwf(&DeeError_ValueError,
                   "Unknown environment variable `%s'",
                   name);
  }
  return NULL;
 }
 valsiz = strlen(strval);
 result = DeeString_TryNewSized(strval,valsiz);
 if unlikely(!result) {
  rwlock_endread(&env_lock);
  /* Collect memory and try again. */
  if (!try_get) {
   if (Dee_CollectMemory(offsetof(DeeStringObject,s_str)+
                        (valsiz+1)*sizeof(char)))
       goto again;
  }
  return NULL;
 }
 rwlock_endread(&env_lock);
 return result;
}
INTERN int DCALL
fs_printenv(char const *__restrict name,
            struct ascii_printer *__restrict printer, bool try_get) {
 char *buf,*envval; int env_ver; size_t envlen;
again:
 rwlock_read(&env_lock);
 env_ver = env_version;
 DBG_ALIGNMENT_DISABLE();
 envval  = getenv(name);
 DBG_ALIGNMENT_ENABLE();
 if (!envval) {
  rwlock_endread(&env_lock);
  if (!try_get) { err_unknown_env_var_s(name); goto err; }
  return 1;
 }
 envlen = strlen(envval);
 rwlock_endread(&env_lock);
 buf = ascii_printer_alloc(printer,envlen);
 if unlikely(!buf) goto err;
 rwlock_read(&env_lock);
 /* Check if the environment changed in the mean time. */
 if unlikely(env_ver != env_version) {
  rwlock_endread(&env_lock);
  ascii_printer_release(printer,envlen);
  goto again;
 }
 /* Copy the environment variable string. */
 memcpy(buf,envval,envlen*sizeof(char));
 rwlock_endread(&env_lock);
 return 0;
err:
 return -1;
}

INTERN int DCALL
fs_delenv(DeeObject *__restrict name) {
 /* TODO: unsetenv() */
 /* TODO: putenv() */
 (void)name;
 DERROR_NOTIMPLEMENTED();
 return DeeNotify_Broadcast(NOTIFICATION_CLASS_ENVIRON,name);
}

INTERN int DCALL
fs_setenv(DeeObject *__restrict name,
          DeeObject *__restrict value) {
 /* TODO: setenv() */
 /* TODO: putenv() */
 (void)name;
 DERROR_NOTIMPLEMENTED();
 return DeeNotify_Broadcast(NOTIFICATION_CLASS_ENVIRON,name);
}

DECL_END

#endif /* !GUARD_DEX_FS_ENVIRON_C_INL */
