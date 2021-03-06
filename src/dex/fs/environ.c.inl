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
#ifndef GUARD_DEX_FS_ENVIRON_C_INL
#define GUARD_DEX_FS_ENVIRON_C_INL 1
#ifndef DEE_SOURCE
#define DEE_SOURCE 1
#endif /* !DEE_SOURCE */

#include "libfs.h"
/**/
#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/error.h>
#include <deemon/exec.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/system-features.h> /* strend(), memcpyc(), ... */
#include <deemon/tuple.h>

#include "_res.h"

#ifndef CONFIG_NO_THREADS
#include <deemon/util/rwlock.h>
#endif /* !CONFIG_NO_THREADS */

#include <hybrid/atomic.h>

DECL_BEGIN

#ifndef CONFIG_HAVE_strrchr
#define CONFIG_HAVE_strrchr 1
#undef strrchr
#define strrchr dee_strrchr
LOCAL WUNUSED NONNULL((1)) char *
dee_strrchr(char const *haystack, char needle) {
	char *result = NULL;
	for (;; ++haystack) {
		char ch = *haystack;
		if (ch == needle)
			result = (char *)haystack;
		if (!ch)
			break;
	}
	return result;
}
#endif /* !CONFIG_HAVE_strrchr */




typedef struct {
	OBJECT_HEAD
	char       **e_iter;    /* [0..1][1..1] Next environment string. */
	unsigned int e_version; /* The environment version when iteration started. */
} Env;

PRIVATE rwlock_t env_lock = RWLOCK_INIT;
PRIVATE unsigned int env_version = 0;

PRIVATE char *empty_env[] = { NULL };

PRIVATE WUNUSED NONNULL((1)) int DCALL
env_init(Env *__restrict self) {
/*again:*/
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

PRIVATE WUNUSED NONNULL((1)) int DCALL
env_bool(Env *__restrict self) {
	return self->e_iter[0] != 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeString_TryNewSized(char const *__restrict str, size_t len) {
	DREF DeeStringObject *result;
	result = (DREF DeeStringObject *)DeeObject_TryMalloc(offsetof(DeeStringObject, s_str) +
	                                                     (len + 1) * sizeof(char));
	if unlikely(!result)
		goto done;
	DeeObject_Init(result, &DeeString_Type);
	result->s_data = NULL;
	result->s_hash = (dhash_t)-1;
	result->s_len  = len;
	memcpyc(result->s_str, str, len, sizeof(char));
	result->s_str[len] = '\0';
done:
	return (DREF DeeObject *)result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
env_next(Env *__restrict self) {
	unsigned int my_version;
	DREF DeeObject *result_tuple;
	DREF DeeObject *name, *value;
	char **presult, *result, *valstart;
	rwlock_read(&env_lock);
	/* Check environment version number. */
	if ((my_version = self->e_version) != env_version) {
iter_done:
		rwlock_endread(&env_lock);
		return ITER_DONE;
	}
	do {
		presult = self->e_iter;
		if (!*presult)
			goto iter_done;
	} while (ATOMIC_CMPXCH(self->e_iter, presult, presult + 1));
	result   = *presult;
	valstart = strrchr(result, '=');
	if (!valstart)
		valstart = strend(result);
allocate_value:
	value = DeeString_TryNewSized(valstart, strlen(valstart));
	if unlikely(!value) {
		/* Collect memory and try again. */
		rwlock_endread(&env_lock);
		if (!Dee_CollectMemory(offsetof(DeeStringObject, s_str) +
		                       (strlen(valstart) + 1) * sizeof(char)))
			goto err;
		rwlock_read(&env_lock);
		if (my_version != env_version)
			goto iter_done;
		goto allocate_value;
	}
	if (*valstart)
		--valstart;
allocate_name:
	name = DeeString_TryNewSized(result, (size_t)(valstart - result));
	if unlikely(!name) {
		/* Collect memory and try again. */
		rwlock_endread(&env_lock);
		if (!Dee_CollectMemory(offsetof(DeeStringObject, s_str) +
		                       ((size_t)(valstart - result) + 1) * sizeof(char)))
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
	result_tuple = DeeTuple_PackSymbolic(2, name, value); /* Inherit name+value on success. */
	if unlikely(!result_tuple)
		goto err_name;
	return result_tuple;
err_name:
	Dee_Decref(name);
err_value:
	Dee_Decref(value);
err:
	return NULL;
}


INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
enviterator_next_key(DeeObject *__restrict self) {
	unsigned int my_version;
	DREF DeeObject *name;
	char **presult, *result, *valstart;
	Env *me = (Env *)self;
	rwlock_read(&env_lock);
	/* Check environment version number. */
	if ((my_version = me->e_version) != env_version) {
iter_done:
		rwlock_endread(&env_lock);
		return ITER_DONE;
	}
	do {
		presult = me->e_iter;
		if (!*presult)
			goto iter_done;
	} while (ATOMIC_CMPXCH(me->e_iter, presult, presult + 1));
	result   = *presult;
	valstart = strrchr(result, '=');
	if (!valstart)
		valstart = strend(result);
	else {
		--valstart;
	}
allocate_name:
	name = DeeString_TryNewSized(result, (size_t)(valstart - result));
	if unlikely(!name) {
		/* Collect memory and try again. */
		rwlock_endread(&env_lock);
		if (!Dee_CollectMemory(offsetof(DeeStringObject, s_str) +
		                       ((size_t)(valstart - result) + 1) * sizeof(char)))
			goto err;
		rwlock_read(&env_lock);
		if (my_version != env_version)
			goto iter_done;
		goto allocate_name;
	}
	rwlock_endread(&env_lock);
	return name;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
enviterator_next_value(DeeObject *__restrict self) {
	unsigned int my_version;
	DREF DeeObject *value;
	char **presult, *result, *valstart;
	Env *me = (Env *)self;
	rwlock_read(&env_lock);
	/* Check environment version number. */
	if ((my_version = me->e_version) != env_version) {
iter_done:
		rwlock_endread(&env_lock);
		return ITER_DONE;
	}
	do {
		presult = me->e_iter;
		if (!*presult)
			goto iter_done;
	} while (ATOMIC_CMPXCH(me->e_iter, presult, presult + 1));
	result   = *presult;
	valstart = strrchr(result, '=');
	if (!valstart) {
		value = Dee_EmptyString;
		Dee_Incref(value);
	} else {
allocate_value:
		value = DeeString_TryNewSized(valstart, strlen(valstart));
		if unlikely(!value) {
			/* Collect memory and try again. */
			rwlock_endread(&env_lock);
			if (!Dee_CollectMemory(offsetof(DeeStringObject, s_str) +
			                       (strlen(valstart) + 1) * sizeof(char)))
				goto err;
			rwlock_read(&env_lock);
			if (my_version != env_version)
				goto iter_done;
			goto allocate_value;
		}
	}
	rwlock_endread(&env_lock);
	return value;
err:
	return NULL;
}


PRIVATE struct type_member tpconst env_members[] = {
	TYPE_MEMBER_CONST("seq", &DeeEnv_Singleton),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject DeeEnvIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ S_EnvIterator_tp_name,
	/* .tp_doc      = */ S_EnvIterator_tp_doc,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&env_init,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(Env)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&env_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&env_next,
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



PRIVATE NONNULL((1)) void DCALL
err_unknown_env_var(DeeObject *__restrict name) {
	DeeError_Throwf(&DeeError_KeyError,
	                "Unknown environment variable `%k'",
	                name);
}

PRIVATE NONNULL((1)) void DCALL
err_unknown_env_var_s(char const *__restrict name) {
	DeeError_Throwf(&DeeError_KeyError,
	                "Unknown environment variable `%s'",
	                name);
}

INTERN WUNUSED NONNULL((1)) bool DCALL
fs_hasenv(/*String*/ DeeObject *__restrict name) {
	bool result;
	rwlock_read(&env_lock);
	DBG_ALIGNMENT_DISABLE();
	result = getenv(DeeString_STR(name)) != NULL;
	DBG_ALIGNMENT_ENABLE();
	rwlock_endread(&env_lock);
	return result;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
fs_getenv(DeeObject *__restrict name, bool try_get) {
	DREF DeeObject *result;
	char *strval;
	size_t valsiz;
again:
	rwlock_read(&env_lock);
	DBG_ALIGNMENT_DISABLE();
	strval = getenv(DeeString_STR(name));
	DBG_ALIGNMENT_ENABLE();
	if (!strval) {
		rwlock_endread(&env_lock);
		if (!try_get)
			err_unknown_env_var(name);
		return NULL;
	}
	valsiz = strlen(strval);
	result = DeeString_TryNewSized(strval, valsiz);
	if unlikely(!result) {
		rwlock_endread(&env_lock);
		/* Collect memory and try again. */
		if (!try_get) {
			if (Dee_CollectMemory(offsetof(DeeStringObject, s_str) +
			                      (valsiz + 1) * sizeof(char)))
				goto again;
		}
		return NULL;
	}
	rwlock_endread(&env_lock);
	return result;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
fs_printenv(char const *__restrict name,
            struct unicode_printer *__restrict printer,
            bool try_get) {
	char *buf, *envval;
	unsigned int env_ver;
	size_t envlen;
	dssize_t error;
again:
	rwlock_read(&env_lock);
	env_ver = env_version;
	DBG_ALIGNMENT_DISABLE();
	envval = getenv(name);
	DBG_ALIGNMENT_ENABLE();
	if (!envval) {
		rwlock_endread(&env_lock);
		if (!try_get) {
			err_unknown_env_var_s(name);
			goto err;
		}
		return 1;
	}
	envlen = strlen(envval);
	rwlock_endread(&env_lock);
	buf = unicode_printer_alloc_utf8(printer, envlen);
	if unlikely(!buf)
		goto err;
	rwlock_read(&env_lock);
	/* Check if the environment changed in the mean time. */
	if unlikely(env_ver != env_version) {
		rwlock_endread(&env_lock);
		unicode_printer_free_utf8(printer, buf);
		goto again;
	}
	/* Copy the environment variable string. */
	memcpyc(buf, envval, envlen, sizeof(char));
	rwlock_endread(&env_lock);
	error = unicode_printer_confirm_utf8(printer, buf, envlen);
	if unlikely(error < 0)
		goto err;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
fs_delenv(DeeObject *__restrict name) {
	/* TODO: unsetenv() */
	/* TODO: putenv() */
	(void)name;
	DERROR_NOTIMPLEMENTED();
	return DeeNotify_Broadcast(Dee_NOTIFICATION_CLASS_ENVIRON, name);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
fs_setenv(DeeObject *__restrict name,
          DeeObject *__restrict value) {
	/* TODO: setenv() */
	/* TODO: putenv() */
	(void)name;
	(void)value;
	DERROR_NOTIMPLEMENTED();
	return DeeNotify_Broadcast(Dee_NOTIFICATION_CLASS_ENVIRON, name);
}

DECL_END

#endif /* !GUARD_DEX_FS_ENVIRON_C_INL */
