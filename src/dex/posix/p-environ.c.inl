/* Copyright (c) 2018-2025 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2025 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEX_POSIX_P_ENVIRON_C_INL
#define GUARD_DEX_POSIX_P_ENVIRON_C_INL 1
#define CONFIG_BUILDING_LIBPOSIX
#define DEE_SOURCE

#include "libposix.h"
/**/

#include <deemon/seq.h>
#include <deemon/util/atomic.h>
#include <deemon/util/lock.h>
/**/

#include <hybrid/debug-alignment.h>
/**/

#include <stddef.h> /* size_t */

DECL_BEGIN

INTDEF DeeTypeObject DeeEnviron_Type;
INTDEF DeeTypeObject DeeEnvironIterator_Type;



/* Figure out how to implement `getenv()' */
#undef posix_getenv_USE_GetEnvironmentVariableW
#undef posix_getenv_USE_wgetenv
#undef posix_getenv_USE_getenv
#undef posix_getenv_USE_wenviron
#undef posix_getenv_USE_environ
#undef posix_getenv_USE_STUB
#ifdef CONFIG_HOST_WINDOWS
#define posix_getenv_USE_GetEnvironmentVariableW
#elif defined(CONFIG_HAVE_wgetenv) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define posix_getenv_USE_wgetenv
#elif defined(CONFIG_HAVE_getenv)
#define posix_getenv_USE_getenv
#elif defined(CONFIG_HAVE_wgetenv)
#define posix_getenv_USE_wgetenv
#elif defined(CONFIG_HAVE_wenviron) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define posix_getenv_USE_wenviron
#elif defined(CONFIG_HAVE_environ)
#define posix_getenv_USE_environ
#elif defined(CONFIG_HAVE_wenviron)
#define posix_getenv_USE_wenviron
#else /* ... */
#define posix_getenv_USE_STUB
#endif /* !... */



/* Figure out how to implement `setenv()' */
#undef posix_setenv_USE_SetEnvironmentVariableW
#undef posix_setenv_USE_wsetenv
#undef posix_setenv_USE_setenv
#undef posix_setenv_USE_wputenv_s
#undef posix_setenv_USE_putenv_s
#undef posix_setenv_USE_wenviron
#undef posix_setenv_USE_environ
#undef posix_setenv_USE_STUB
#ifdef CONFIG_HOST_WINDOWS
#define posix_setenv_USE_SetEnvironmentVariableW
#elif defined(CONFIG_HAVE_wsetenv) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define posix_setenv_USE_wsetenv
#elif defined(CONFIG_HAVE_setenv)
#define posix_setenv_USE_setenv
#elif defined(CONFIG_HAVE_wsetenv)
#define posix_setenv_USE_wsetenv
#elif defined(CONFIG_HAVE_wputenv_s) && (defined(CONFIG_HAVE_wgetenv) || defined(CONFIG_HAVE_wenviron)) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define posix_setenv_USE_wputenv_s
#elif defined(CONFIG_HAVE_putenv_s) && (defined(CONFIG_HAVE_getenv) || defined(CONFIG_HAVE_environ))
#define posix_setenv_USE_putenv_s
#elif defined(CONFIG_HAVE_wputenv_s) && (defined(CONFIG_HAVE_wgetenv) || defined(CONFIG_HAVE_wenviron))
#define posix_setenv_USE_wputenv_s
#elif defined(CONFIG_HAVE_wenviron) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define posix_setenv_USE_wenviron
#elif defined(CONFIG_HAVE_environ)
#define posix_setenv_USE_environ
#elif defined(CONFIG_HAVE_wenviron)
#define posix_setenv_USE_wenviron
#else /* ... */
#define posix_setenv_USE_STUB
#endif /* !... */



/* Figure out how to implement `unsetenv()' */
#undef posix_unsetenv_USE_SetEnvironmentVariableW
#undef posix_unsetenv_USE_wunsetenv
#undef posix_unsetenv_USE_unsetenv
#undef posix_unsetenv_USE_wputenv
#undef posix_unsetenv_USE_putenv
#undef posix_unsetenv_USE_wenviron
#undef posix_unsetenv_USE_environ
#undef posix_unsetenv_USE_STUB
#ifdef posix_setenv_USE_environ
#define posix_unsetenv_USE_environ
#elif defined(posix_setenv_USE_wenviron)
#define posix_unsetenv_USE_wenviron
#elif defined(CONFIG_HOST_WINDOWS)
#define posix_unsetenv_USE_SetEnvironmentVariableW
#elif defined(CONFIG_HAVE_wunsetenv) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define posix_unsetenv_USE_wunsetenv
#elif defined(CONFIG_HAVE_unsetenv)
#define posix_unsetenv_USE_unsetenv
#elif defined(CONFIG_HAVE_wunsetenv)
#define posix_unsetenv_USE_wunsetenv
#elif defined(CONFIG_HAVE_wputenv) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define posix_unsetenv_USE_wputenv
#elif defined(CONFIG_HAVE_putenv)
#define posix_unsetenv_USE_putenv
#elif defined(CONFIG_HAVE_wputenv)
#define posix_unsetenv_USE_wputenv
#else /* ... */
#define posix_unsetenv_USE_STUB
#endif /* !... */



/* Figure out how to implement `clearenv()' */
#undef posix_clearenv_USE_environ_setempty
#undef posix_clearenv_USE_wenviron_setempty
#undef posix_clearenv_USE_SetEnvironmentStringsW
#undef posix_clearenv_USE_wclearenv
#undef posix_clearenv_USE_clearenv
#undef posix_clearenv_USE_wunsetenv
#undef posix_clearenv_USE_unsetenv
#undef posix_clearenv_USE_wputenv
#undef posix_clearenv_USE_putenv
#undef posix_clearenv_USE_STUB
#ifdef posix_setenv_USE_environ
#define posix_clearenv_USE_environ_setempty
#elif defined(posix_setenv_USE_wenviron)
#define posix_clearenv_USE_wenviron_setempty
#elif defined(CONFIG_HOST_WINDOWS)
#define posix_clearenv_USE_SetEnvironmentStringsW
#elif defined(CONFIG_HAVE_wclearenv) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define posix_clearenv_USE_wclearenv
#elif defined(CONFIG_HAVE_clearenv)
#define posix_clearenv_USE_clearenv
#elif defined(CONFIG_HAVE_wclearenv)
#define posix_clearenv_USE_wclearenv
#elif defined(CONFIG_HAVE_wunsetenv) && defined(CONFIG_HAVE_wenviron) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define posix_clearenv_USE_wunsetenv
#elif defined(CONFIG_HAVE_unsetenv) && defined(CONFIG_HAVE_environ)
#define posix_clearenv_USE_unsetenv
#elif defined(CONFIG_HAVE_wunsetenv) && defined(CONFIG_HAVE_wenviron)
#define posix_clearenv_USE_wunsetenv
#elif defined(CONFIG_HAVE_wputenv) && defined(CONFIG_HAVE_wenviron) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define posix_clearenv_USE_wputenv
#elif defined(CONFIG_HAVE_putenv) && defined(CONFIG_HAVE_environ)
#define posix_clearenv_USE_putenv
#elif defined(CONFIG_HAVE_wputenv) && defined(CONFIG_HAVE_wenviron)
#define posix_clearenv_USE_wputenv
#elif defined(CONFIG_HAVE_environ)
#define posix_clearenv_USE_environ_setempty
#elif defined(CONFIG_HAVE_wenviron)
#define posix_clearenv_USE_wenviron_setempty
#else /* ... */
#define posix_clearenv_USE_STUB
#endif /* !... */



/* Figure out how to implement `environ.operator iter()' */
#undef posix_enumenv_USE_GetEnvironmentStringsW
#undef posix_enumenv_USE_environ
#undef posix_enumenv_USE_wenviron
#undef posix_enumenv_USE_STUB
#ifdef CONFIG_HOST_WINDOWS
#define posix_enumenv_USE_GetEnvironmentStringsW
#elif defined(CONFIG_HAVE_wenviron) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define posix_enumenv_USE_wenviron
#elif defined(CONFIG_HAVE_environ)
#define posix_enumenv_USE_environ
#elif defined(CONFIG_HAVE_wenviron)
#define posix_enumenv_USE_wenviron
#else /* ... */
#define posix_enumenv_USE_STUB
#endif /* !... */



/* If we ever use `environ' for anything, we have to use a lock to access it. */
#if (defined(posix_getenv_USE_environ) || defined(posix_getenv_USE_wenviron) ||                       \
     defined(posix_setenv_USE_environ) || defined(posix_setenv_USE_wenviron) ||                       \
     defined(posix_unsetenv_USE_environ) || defined(posix_unsetenv_USE_wenviron) ||                   \
     defined(posix_clearenv_USE_environ_setempty) || defined(posix_clearenv_USE_wenviron_setempty) || \
     defined(posix_clearenv_USE_wunsetenv) || defined(posix_clearenv_USE_unsetenv) ||                 \
     defined(posix_clearenv_USE_wputenv) || defined(posix_clearenv_USE_putenv) ||                     \
     defined(posix_enumenv_USE_environ) || defined(posix_enumenv_USE_wenviron))
#if defined(CONFIG_HAVE_ENV_LOCK) && defined(CONFIG_HAVE_ENV_UNLOCK)
#ifdef CONFIG_HAVE_ENVLOCK_H
#include <envlock.h>
#endif /* CONFIG_HAVE_ENVLOCK_H */
#define environ_lock_read()     ENV_LOCK
#define environ_lock_write()    ENV_LOCK
#define environ_lock_endread()  ENV_UNLOCK
#define environ_lock_endwrite() ENV_UNLOCK
#else /* CONFIG_HAVE_ENV_LOCK && CONFIG_HAVE_ENV_UNLOCK */
PRIVATE Dee_atomic_rwlock_t dee_environ_lock = DEE_ATOMIC_RWLOCK_INIT;
#define environ_lock_read()     Dee_atomic_rwlock_read(&dee_environ_lock)
#define environ_lock_write()    Dee_atomic_rwlock_write(&dee_environ_lock)
#define environ_lock_endread()  Dee_atomic_rwlock_endread(&dee_environ_lock)
#define environ_lock_endwrite() Dee_atomic_rwlock_endwrite(&dee_environ_lock)
#endif /* !CONFIG_HAVE_ENV_LOCK || !CONFIG_HAVE_ENV_UNLOCK */
#endif /* ... */

#ifndef environ_lock_read
#define environ_lock_read()     (void)0
#define environ_lock_endread()  (void)0
#define environ_lock_write()    (void)0
#define environ_lock_endwrite() (void)0
#endif /* !environ_lock_read */

#if defined(posix_enumenv_USE_environ) || defined(posix_enumenv_USE_wenviron)
PRIVATE size_t dee_environ_version = 0;
#define environ_changed() (void)++dee_environ_version
#endif /* posix_enumenv_USE_environ || posix_enumenv_USE_wenviron */

#ifndef environ_changed
#define environ_changed() (void)0
#endif /* !environ_changed */


#if (defined(posix_getenv_USE_wgetenv) || defined(posix_getenv_USE_wenviron) ||                 \
     (defined(posix_setenv_USE_wputenv_s) && !defined(CONFIG_HAVE_wgetenv)) ||                  \
     defined(posix_setenv_USE_wenviron) || defined(posix_enumenv_USE_GetEnvironmentStringsW) || \
     defined(posix_enumenv_USE_wenviron))
#ifndef CONFIG_HAVE_wcslen
#define CONFIG_HAVE_wcslen
#undef wcslen
#define wcslen dee_wcslen
DeeSystem_DEFINE_wcslen(dee_wcslen)
#endif /* !CONFIG_HAVE_wcslen */
#ifndef CONFIG_HAVE_wcsend
#undef wcsend
#define wcsend(str) ((str) + wcslen(str))
#endif /* !CONFIG_HAVE_wcsend */
#endif /* ... */


#if defined(posix_setenv_USE_wenviron) || defined(posix_unsetenv_USE_wenviron)
#define HAVE_system_wenviron_is_dee_heap_allocated
PRIVATE bool system_wenviron_is_dee_heap_allocated = false;
PRIVATE bool DCALL try_set_system_wenviron_is_dee_heap_allocated(void) {
	Dee_wchar_t **new_wenviron;
	size_t i, count;
	if (system_wenviron_is_dee_heap_allocated)
		return true;
	count = 0;
	if (wenviron) {
		while (wenviron[count])
			++count;
	}
	new_wenviron = (Dee_wchar_t **)Dee_TryMallocc(count + 1, sizeof(Dee_wchar_t *));
	if unlikely(!new_wenviron)
		goto err_new_wenviron;
	for (i = 0; i < count; ++i) {
		Dee_wchar_t *old_str, *new_str;
		size_t old_len;
		old_str = (Dee_wchar_t *)wenviron[i];
		old_len = wcslen((wchar_t *)old_str);
		new_str = (Dee_wchar_t *)Dee_TryMallocc(old_len + 1, sizeof(Dee_wchar_t));
		if unlikely(!new_str)
			goto err_new_wenviron_i;
		new_str = (Dee_wchar_t *)memcpyc(new_str, old_str, old_len, sizeof(Dee_wchar_t));
		new_wenviron[i] = new_str;
	}
	new_wenviron[count] = NULL;
	wenviron = (wchar_t **)new_wenviron;
	system_wenviron_is_dee_heap_allocated = true;
	return true;
err_new_wenviron_i:
	while (i--)
		Dee_Free(new_wenviron[i]);
err_new_wenviron:
	Dee_Free(new_wenviron);
err:
	return false;
}
#endif /* posix_setenv_USE_wenviron || posix_unsetenv_USE_wenviron */

#if defined(posix_setenv_USE_environ) || defined(posix_unsetenv_USE_environ)
#define HAVE_system_environ_is_dee_heap_allocated
PRIVATE bool system_environ_is_dee_heap_allocated = false;
PRIVATE bool DCALL try_set_system_environ_is_dee_heap_allocated(void) {
	char **new_environ;
	size_t i, count;
	if (system_environ_is_dee_heap_allocated)
		return true;
	count = 0;
	if (environ) {
		while (environ[count])
			++count;
	}
	new_environ = (char **)Dee_TryMallocc(count + 1, sizeof(char *));
	if unlikely(!new_environ)
		goto err_new_environ;
	for (i = 0; i < count; ++i) {
		char *old_str, *new_str;
		size_t old_len;
		old_str = environ[i];
		old_len = strlen(old_str);
		new_str = (char *)Dee_TryMallocc(old_len + 1, sizeof(char));
		if unlikely(!new_str)
			goto err_new_environ_i;
		new_str = (char *)memcpyc(new_str, old_str, old_len, sizeof(char));
		new_environ[i] = new_str;
	}
	new_environ[count] = NULL;
	environ = new_environ;
	system_environ_is_dee_heap_allocated = true;
	return true;
err_new_environ_i:
	while (i--)
		Dee_Free(new_environ[i]);
err_new_environ:
	Dee_Free(new_environ);
err:
	return false;
}
#endif /* posix_setenv_USE_environ || posix_unsetenv_USE_environ */

#if (defined(posix_unsetenv_USE_wputenv) ||               \
     defined(posix_enumenv_USE_GetEnvironmentStringsW) || \
     defined(posix_enumenv_USE_wenviron))
#ifndef CONFIG_HAVE_wcschr
#define CONFIG_HAVE_wcschr
DECL_BEGIN
#undef wcschr
#define wcschr dee_wcschr
LOCAL ATTR_PURE WUNUSED NONNULL((1)) Dee_wchar_t *
dee_wcschr(Dee_wchar_t const *haystack, Dee_wchar_t needle) {
	for (;; ++haystack) {
		Dee_wchar_t ch = *haystack;
		if (ch == needle)
			return (Dee_wchar_t *)haystack;
		if (!ch)
			break;
	}
	return NULL;
}
DECL_END
#endif /* !CONFIG_HAVE_wcschr */
#endif /* ... */



/************************************************************************/
/* Platform-specific functions                                          */
/************************************************************************/

/* @return:  1: Exists
 * @return:  0: Doesn't exist
 * @return: -1: Error */
PRIVATE WUNUSED NONNULL((1)) int DCALL
posix_environ_hasenv(DeeStringObject *__restrict name) {
#ifdef posix_getenv_USE_GetEnvironmentVariableW
	bool result;
	LPCWSTR wname = (LPCWSTR)DeeString_AsWide((DeeObject *)name);
	if unlikely(!wname)
		return -1;
	environ_lock_read();
	DBG_ALIGNMENT_DISABLE();
	result = GetEnvironmentVariableW(wname, NULL, 0) != 0;
	DBG_ALIGNMENT_ENABLE();
	environ_lock_endread();
	return result ? 1 : 0;
#endif /* posix_getenv_USE_GetEnvironmentVariableW */

#ifdef posix_getenv_USE_wgetenv
	bool result;
	Dee_wchar_t const *wname = DeeString_AsWide((DeeObject *)name);
	if unlikely(!wname)
		return -1;
	environ_lock_read();
	DBG_ALIGNMENT_DISABLE();
	result = wgetenv((wchar_t *)wname) != NULL;
	DBG_ALIGNMENT_ENABLE();
	environ_lock_endread();
	return result ? 1 : 0;
#endif /* posix_getenv_USE_wgetenv */

#ifdef posix_getenv_USE_getenv
	bool result;
	char const *utf8 = DeeString_AsUtf8((DeeObject *)name);
	if unlikely(!utf8)
		return -1;
	environ_lock_read();
	DBG_ALIGNMENT_DISABLE();
	result = getenv((char *)utf8) != NULL;
	DBG_ALIGNMENT_ENABLE();
	environ_lock_endread();
	return result ? 1 : 0;
#endif /* posix_getenv_USE_getenv */

#ifdef posix_getenv_USE_wenviron
	int result;
	Dee_wchar_t const *wenvstr;
	Dee_wchar_t const *wname;
	size_t i, wname_len;
	wname = DeeString_AsWide((DeeObject *)name);
	if unlikely(!wname)
		return -1;
	wname_len = wcslen((wchar_t *)wname);
	result = 0;
	environ_lock_read();
	if (wenviron) {
		for (i = 0; (wenvstr = (Dee_wchar_t const *)wenviron[i]) != NULL; ++i) {
			if (bcmpc(wenvstr, wname, wname_len, sizeof(Dee_wchar_t)) == 0 &&
				wenvstr[wname_len] == (Dee_wchar_t)'=') {
				result = 1;
				break;
			}
		}
	}
	environ_lock_endread();
	return result;
#endif /* posix_getenv_USE_wenviron */

#ifdef posix_getenv_USE_environ
	int result;
	char const *envstr, *utf8_name;
	size_t i, utf8_name_len;
	utf8_name = DeeString_AsUtf8((DeeObject *)name);
	if unlikely(!utf8_name)
		return -1;
	utf8_name_len = strlen(utf8_name);
	result = 0;
	environ_lock_read();
	if (environ) {
		for (i = 0; (envstr = (char *)environ[i]) != NULL; ++i) {
			if (bcmpc(envstr, utf8_name, wname_len, sizeof(char)) == 0 &&
				envstr[wname_len] == '=') {
				result = 1;
				break;
			}
		}
	}
	environ_lock_endread();
	return result;
#endif /* posix_getenv_USE_environ */

#ifdef posix_getenv_USE_STUB
	(void)name;
	return 0;
#endif /* posix_getenv_USE_STUB */
}

PRIVATE ATTR_COLD NONNULL((1)) int DCALL
err_unknown_env_var(DeeObject *__restrict name) {
	return DeeError_Throwf(&DeeError_KeyError,
	                       "Unknown environment variable `%k'",
	                       name);
}

/* Caller must call: `err_unknown_env_var((DeeObject *)name);'  */
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
posix_environ_trygetenv(DeeStringObject *name) {
#ifdef posix_getenv_USE_GetEnvironmentVariableW
	LPWSTR buffer, new_buffer;
	DWORD bufsize = 256, error;
	LPWSTR wname;
	wname = (LPWSTR)DeeString_AsWide((DeeObject *)name);
	if unlikely(!wname)
		goto err;
	buffer = DeeString_NewWideBuffer(bufsize);
	if unlikely(!buffer)
		goto err;
	for (;;) {
		environ_lock_read();
		DBG_ALIGNMENT_DISABLE();
		error = GetEnvironmentVariableW(wname, buffer, bufsize + 1);
		DBG_ALIGNMENT_ENABLE();
		environ_lock_endread();
		if (!error) {
			/* Error. */
			DeeString_FreeWideBuffer(buffer);
			return ITER_DONE;
		}
		if (error <= bufsize)
			break;
		/* Resize to fit. */
		new_buffer = DeeString_ResizeWideBuffer(buffer, error);
		if unlikely(!new_buffer)
			goto err_buffer;
		buffer  = new_buffer;
		bufsize = error - 1;
	}
	buffer = DeeString_TruncateWideBuffer(buffer, error);
	return DeeString_PackWideBuffer(buffer, STRING_ERROR_FREPLAC);
err_buffer:
	DeeString_FreeWideBuffer(buffer);
err:
	return NULL;
#endif /* posix_getenv_USE_GetEnvironmentVariableW */

#if (defined(posix_getenv_USE_wgetenv) || defined(posix_getenv_USE_wenviron))
	size_t reqlen;
	Dee_wchar_t *new_buffer, *buffer = NULL;
	Dee_wchar_t const *wenvstr;
	Dee_wchar_t const *wname = DeeString_AsWide((DeeObject *)name);
#ifdef posix_getenv_USE_wenviron
	size_t wname_len;
#endif /* posix_getenv_USE_wenviron */
	if unlikely(!wname)
		goto err;
#ifdef posix_getenv_USE_environ
	wname_len = wcslen((wchar_t *)wname);
#endif /* posix_getenv_USE_environ */
again:
	environ_lock_read();
	DBG_ALIGNMENT_DISABLE();
#ifdef posix_getenv_USE_wgetenv
	wenvstr = (Dee_wchar_t const *)wgetenv((wchar_t *)wname);
#endif /* posix_getenv_USE_wgetenv */
#ifdef posix_getenv_USE_wenviron
	wenvstr = NULL;
	if (wenviron) {
		size_t i;
		for (i = 0; (wenvstr = (Dee_wchar_t *)wenviron[i]) != NULL; ++i) {
			if (bcmpc(wenvstr, wname, wname_len, sizeof(char)) == 0 &&
			    wenvstr[wname_len] == (Dee_wchar_t)'=')
				break;
		}
	}
#endif /* posix_getenv_USE_wenviron */
	if (wenvstr == NULL) {
		DBG_ALIGNMENT_ENABLE();
		environ_lock_endread();
		DeeString_FreeWideBuffer(buffer);
		return ITER_DONE;
	}
	reqlen     = wcslen((wchar_t *)wenvstr);
	new_buffer = DeeString_TryResizeWideBuffer(buffer, reqlen);
	if unlikely(!new_buffer) {
		DBG_ALIGNMENT_ENABLE();
		environ_lock_endread();
		new_buffer = DeeString_ResizeWideBuffer(buffer, reqlen);
		if unlikely(!new_buffer)
			goto err_buffer;
		buffer = new_buffer;
		goto again;
	}
	buffer = (Dee_wchar_t *)memcpyc(new_buffer, wenvstr, reqlen, sizeof(Dee_wchar_t));
	DBG_ALIGNMENT_ENABLE();
	environ_lock_endread();
	buffer = DeeString_TruncateWideBuffer(buffer, reqlen);
	return DeeString_PackWideBuffer(buffer, STRING_ERROR_FREPLAC);
err_buffer:
	DeeString_FreeWideBuffer(buffer);
err:
	return NULL;
#endif /* posix_getenv_USE_wgetenv || posix_getenv_USE_wenviron */

#if (defined(posix_getenv_USE_getenv) || defined(posix_getenv_USE_environ))
	size_t reqlen;
	char *new_buffer, *buffer = NULL;
	char const *envstr, *utf8_name = DeeString_AsUtf8((DeeObject *)name);
#ifdef posix_getenv_USE_environ
	size_t utf8_name_len;
#endif /* posix_getenv_USE_environ */
	if unlikely(!utf8_name)
		goto err;
#ifdef posix_getenv_USE_environ
	utf8_name_len = strlen(utf8_name);
#endif /* posix_getenv_USE_environ */
again:
	environ_lock_read();
	DBG_ALIGNMENT_DISABLE();
#ifdef posix_getenv_USE_getenv
	envstr = (char const *)getenv((char *)utf8_name);
#endif /* posix_getenv_USE_getenv */
#ifdef posix_getenv_USE_environ
	envstr = NULL;
	if (environ) {
		size_t i;
		for (i = 0; (envstr = (char *)environ[i]) != NULL; ++i) {
			if (bcmpc(envstr, utf8_name, utf8_name_len, sizeof(char)) == 0 &&
			    envstr[utf8_name_len] == (char)'=')
				break;
		}
	}
#endif /* posix_getenv_USE_environ */
	if (envstr == NULL) {
		DBG_ALIGNMENT_ENABLE();
		environ_lock_endread();
		DeeString_Free1ByteBuffer((uint8_t *)buffer);
		return ITER_DONE;
	}
	reqlen     = strlen(envstr);
	new_buffer = (char *)DeeString_TryResize1ByteBuffer((uint8_t *)buffer, reqlen);
	if unlikely(!new_buffer) {
		DBG_ALIGNMENT_ENABLE();
		environ_lock_endread();
		new_buffer = (char *)DeeString_Resize1ByteBuffer((uint8_t *)buffer, reqlen);
		if unlikely(!new_buffer)
			goto err_buffer;
		buffer = new_buffer;
		goto again;
	}
	buffer = (char *)memcpyc(new_buffer, envstr, reqlen, sizeof(char));
	DBG_ALIGNMENT_ENABLE();
	environ_lock_endread();
	buffer = (char *)DeeString_Truncate1ByteBuffer((uint8_t *)buffer, reqlen);
	return DeeString_PackUtf8Buffer((uint8_t *)buffer, STRING_ERROR_FREPLAC);
err_buffer:
	DeeString_Free1ByteBuffer((uint8_t *)buffer);
err:
	return NULL;
#endif /* posix_getenv_USE_getenv || posix_getenv_USE_environ */

#ifdef posix_getenv_USE_STUB
	(void)name;
	return ITER_DONE;
#endif /* posix_getenv_USE_STUB */
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
posix_environ_setenv(DeeStringObject *name, DeeStringObject *value, bool replace) {
#ifdef posix_setenv_USE_SetEnvironmentVariableW
	LPWSTR wname, wvalue;
	wname = (LPWSTR)DeeString_AsWide((DeeObject *)name);
	if unlikely(!wname)
		goto err;
	wvalue = (LPWSTR)DeeString_AsWide((DeeObject *)value);
	if unlikely(!wvalue)
		goto err;
again_setenv:
	environ_lock_write();
	DBG_ALIGNMENT_DISABLE();
	if (!replace) {
		if (GetEnvironmentVariableW(wname, NULL, 0) != 0) {
			/* Variable already exists, and we're not supposed to replace it. */
			DBG_ALIGNMENT_ENABLE();
			environ_lock_endwrite();
			return 0;
		}
	}
	if (!SetEnvironmentVariableW(wname, wvalue)) {
		DWORD dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		environ_lock_endwrite();
		if (DeeNTSystem_IsBadAllocError(dwError)) {
			if (Dee_CollectMemory(1))
				goto again_setenv;
		} else {
			DeeNTSystem_ThrowErrorf(&DeeError_SystemError, dwError,
			                        "Failed to set environment variable `%k' to `%k'",
			                        name, value);
		}
		goto err;
	}
	DBG_ALIGNMENT_ENABLE();
	environ_changed();
	environ_lock_endwrite();
	/* Broadcast an environ-changed notification. */
	return DeeNotify_Broadcast(Dee_NOTIFICATION_CLASS_ENVIRON, (DeeObject *)name);
err:
	return -1;
#endif /* posix_setenv_USE_SetEnvironmentVariableW */

#ifdef posix_setenv_USE_wsetenv
	Dee_wchar_t const *wname, *wvalue;
	wname = DeeString_AsWide((DeeObject *)name);
	if unlikely(!wname)
		goto err;
	wvalue = DeeString_AsWide((DeeObject *)value);
	if unlikely(!wvalue)
		goto err;
again_setenv:
	environ_lock_write();
	DBG_ALIGNMENT_DISABLE();
	if (wsetenv((wchar_t *)wname, (wchar_t *)wvalue, replace ? 1 : 0) != 0) {
		int error = DeeSystem_GetErrno();
		DBG_ALIGNMENT_ENABLE();
		environ_lock_endwrite();
		DeeSystem_IF_E1(error, ENOMEM, {
			if (Dee_CollectMemory(1))
				goto again_setenv;
			goto err;
		});
		DeeUnixSystem_ThrowErrorf(&DeeError_SystemError, error,
		                          "Failed to set environment variable `%k' to `%k'",
		                          name, value);
		goto err;
	}
	DBG_ALIGNMENT_ENABLE();
	environ_changed();
	environ_lock_endwrite();
	/* Broadcast an environ-changed notification. */
	return DeeNotify_Broadcast(Dee_NOTIFICATION_CLASS_ENVIRON, (DeeObject *)name);
err:
	return -1;
#endif /* posix_setenv_USE_wsetenv */

#ifdef posix_setenv_USE_setenv
	char const *utf8_name, *utf8_value;
	utf8_name = DeeString_AsUtf8((DeeObject *)name);
	if unlikely(!utf8_name)
		goto err;
	utf8_value = DeeString_AsUtf8((DeeObject *)value);
	if unlikely(!utf8_value)
		goto err;
again_setenv:
	environ_lock_write();
	DBG_ALIGNMENT_DISABLE();
	if (setenv((char *)utf8_name, (char *)utf8_value, replace ? 1 : 0) != 0) {
		int error = DeeSystem_GetErrno();
		DBG_ALIGNMENT_ENABLE();
		environ_lock_endwrite();
		DeeSystem_IF_E1(error, ENOMEM, {
			if (Dee_CollectMemory(1))
				goto again_setenv;
			goto err;
		});
		DeeUnixSystem_ThrowErrorf(&DeeError_SystemError, error,
		                          "Failed to set environment variable `%k' to `%k'",
		                          name, value);
		goto err;
	}
	DBG_ALIGNMENT_ENABLE();
	environ_changed();
	environ_lock_endwrite();
	/* Broadcast an environ-changed notification. */
	return DeeNotify_Broadcast(Dee_NOTIFICATION_CLASS_ENVIRON, (DeeObject *)name);
err:
	return -1;
#endif /* posix_setenv_USE_setenv */

#ifdef posix_setenv_USE_wputenv_s
	int error;
	Dee_wchar_t const *wname, *wvalue;
	wname = DeeString_AsWide((DeeObject *)name);
	if unlikely(!wname)
		goto err;
	wvalue = DeeString_AsWide((DeeObject *)value);
	if unlikely(!wvalue)
		goto err;
again_setenv:
	environ_lock_write();
	DBG_ALIGNMENT_DISABLE();
	if (!replace) {
#ifdef CONFIG_HAVE_wgetenv
		if (wgetenv((wchar_t *)wname) != NULL) {
			/* Variable already exists, and we're not supposed to replace it. */
			DBG_ALIGNMENT_ENABLE();
			environ_lock_endwrite();
			return 0;
		}
#else /* CONFIG_HAVE_wgetenv */
		Dee_wchar_t const *wenvstr;
		size_t i, wname_len;
		wname_len = wcslen((wchar_t *)wname);
		environ_lock_write();
		if (wenviron) {
			for (i = 0; (wenvstr = (Dee_wchar_t *)wenviron[i]) != NULL; ++i) {
				if (bcmpc(wenvstr, wname, wname_len, sizeof(Dee_wchar_t)) == 0 &&
				    wenvstr[wname_len] == (Dee_wchar_t)'=') {
					/* Variable already exists, and we're not supposed to replace it. */
					DBG_ALIGNMENT_ENABLE();
					environ_lock_endwrite();
					return 0;
				}
			}
		}
#endif /* !CONFIG_HAVE_wgetenv */
	}
	if ((error = wputenv_s((wchar_t *)wname, (wchar_t *)wvalue)) != 0) {
		DBG_ALIGNMENT_ENABLE();
		environ_lock_endwrite();
		DeeSystem_IF_E1(error, ENOMEM, {
			if (Dee_CollectMemory(1))
				goto again_setenv;
			goto err;
		});
		DeeUnixSystem_ThrowErrorf(&DeeError_SystemError, error,
		                          "Failed to set environment variable `%k' to `%k'",
		                          name, value);
		goto err;
	}
	DBG_ALIGNMENT_ENABLE();
	environ_changed();
	environ_lock_endwrite();
	/* Broadcast an environ-changed notification. */
	return DeeNotify_Broadcast(Dee_NOTIFICATION_CLASS_ENVIRON, (DeeObject *)name);
err:
	return -1;
#endif /* posix_setenv_USE_wputenv_s */

#ifdef posix_setenv_USE_putenv_s
	int error;
	char const *utf8_name, *utf8_value;
	utf8_name = DeeString_AsUtf8((DeeObject *)name);
	if unlikely(!utf8_name)
		goto err;
	utf8_value = DeeString_AsUtf8((DeeObject *)value);
	if unlikely(!utf8_value)
		goto err;
again_setenv:
	environ_lock_write();
	DBG_ALIGNMENT_DISABLE();
	if (!replace) {
#ifdef CONFIG_HAVE_getenv
		if (getenv((char *)utf8_name) != NULL) {
			/* Variable already exists, and we're not supposed to replace it. */
			DBG_ALIGNMENT_ENABLE();
			environ_lock_endwrite();
			return 0;
		}
#else /* CONFIG_HAVE_getenv */
		char const *utf8_envstr;
		size_t i, utf8_name_len;
		utf8_name_len = strlen(utf8_name);
		environ_lock_write();
		if (environ) {
			for (i = 0; (utf8_envstr = environ[i]) != NULL; ++i) {
				if (bcmpc(utf8_envstr, utf8_name, utf8_name_len, sizeof(char)) == 0 &&
				    utf8_envstr[utf8_name_len] == '=') {
					/* Variable already exists, and we're not supposed to replace it. */
					DBG_ALIGNMENT_ENABLE();
					environ_lock_endwrite();
					return 0;
				}
			}
		}
#endif /* !CONFIG_HAVE_getenv */
	}
	if ((error = putenv_s((char *)utf8_name, (char *)utf8_value)) != 0) {
		DBG_ALIGNMENT_ENABLE();
		environ_lock_endwrite();
		DeeSystem_IF_E1(error, ENOMEM, {
			if (Dee_CollectMemory(1))
				goto again_setenv;
			goto err;
		});
		DeeUnixSystem_ThrowErrorf(&DeeError_SystemError, error,
		                          "Failed to set environment variable `%k' to `%k'",
		                          name, value);
		goto err;
	}
	DBG_ALIGNMENT_ENABLE();
	environ_changed();
	environ_lock_endwrite();
	/* Broadcast an environ-changed notification. */
	return DeeNotify_Broadcast(Dee_NOTIFICATION_CLASS_ENVIRON, (DeeObject *)name);
err:
	return -1;
#endif /* posix_setenv_USE_putenv_s */

#ifdef posix_setenv_USE_wenviron
	size_t i;
	Dee_wchar_t const *wide_name, *wide_value;
	Dee_wchar_t *wide_envline, *wide_oldline;
	size_t wide_name_len, wide_value_len;
	wide_name = DeeString_AsWide((DeeObject *)name);
	if unlikely(!wide_name)
		goto err;
	wide_value = DeeString_AsWide((DeeObject *)value);
	if unlikely(!wide_value)
		goto err;
	wide_name_len  = wcslen((wchar_t *)wide_name);
	wide_value_len = wcslen((wchar_t *)wide_value);
	wide_envline   = (Dee_wchar_t *)Dee_Mallocc(wide_name_len + 1 + wide_value_len + 1, sizeof(Dee_wchar_t));
	if unlikely(!wide_envline)
		goto err;
	{
		Dee_wchar_t *iter;
		iter = wide_envline;
		iter = (Dee_wchar_t *)mempcpyc(iter, wide_name, wide_name_len, sizeof(Dee_wchar_t));
		*iter++ = (Dee_wchar_t)'=';
		iter = (Dee_wchar_t *)mempcpyc(iter, wide_value, wide_value_len, sizeof(Dee_wchar_t));
		*iter = (Dee_wchar_t)'\0';
	}
again_insert_env:
	environ_lock_write();
	if (wenviron) {
		for (i = 0; (wide_oldline = (Dee_wchar_t *)wenviron[i]) != NULL; ++i) {
			if (bcmpc(wide_oldline, wide_name, wide_name_len, sizeof(Dee_wchar_t)) == 0 &&
			    wide_oldline[wide_name_len] == (Dee_wchar_t)'=') {
				if (replace) {
					if (!try_set_system_wenviron_is_dee_heap_allocated())
						goto unlock_and_try_collect_memory;
					wide_oldline = (Dee_wchar_t *)wenviron[i];
					wenviron[i]  = (wchar_t *)wide_envline;
				} else {
					wide_oldline = NULL;
				}
				/* Variable already exists, and we're not supposed to replace it. */
				DBG_ALIGNMENT_ENABLE();
				if (replace)
					environ_changed();
				environ_lock_endwrite();
				if (!replace)
					Dee_Free(wide_envline);
				Dee_Free(wide_oldline);
				if (replace)
					return DeeNotify_Broadcast(Dee_NOTIFICATION_CLASS_ENVIRON, (DeeObject *)name);
				return 0;
			}
		}
	}
	if (!try_set_system_wenviron_is_dee_heap_allocated())
		goto unlock_and_try_collect_memory;
	/* Append a new line to `wenviron' */
	{
		Dee_wchar_t **new_wenviron;
		size_t old_wenviron_count;
		for (old_wenviron_count = 0; wenviron[old_wenviron_count]; ++old_wenviron_count)
			;
		new_wenviron = (Dee_wchar_t **)Dee_TryReallocc(wenviron, old_wenviron_count + 2, sizeof(Dee_wchar_t *));
		if unlikely(!new_wenviron)
			goto unlock_and_try_collect_memory;
		new_wenviron[old_wenviron_count + 0] = wide_envline;
		new_wenviron[old_wenviron_count + 1] = NULL;
		wenviron = (wchar_t **)new_wenviron;
	}
	environ_changed();
	environ_lock_endwrite();
	return DeeNotify_Broadcast(Dee_NOTIFICATION_CLASS_ENVIRON, (DeeObject *)name);
unlock_and_try_collect_memory:
	environ_lock_endwrite();
	if (Dee_CollectMemory(1))
		goto again_insert_env;
	/*goto err_wide_envline;*/
err_wide_envline:
	Dee_Free(wide_envline);
err:
	return -1;
#endif /* posix_setenv_USE_wenviron */

#ifdef posix_setenv_USE_environ
	size_t i;
	char const *utf8_name, *utf8_value;
	char *utf8_envline, *utf8_oldline;
	size_t utf8_name_len, utf8_value_len;
	utf8_name = DeeString_AsUtf8((DeeObject *)name);
	if unlikely(!utf8_name)
		goto err;
	utf8_value = DeeString_AsUtf8((DeeObject *)value);
	if unlikely(!utf8_value)
		goto err;
	utf8_name_len  = strlen(utf8_name);
	utf8_value_len = strlen(utf8_value);
	utf8_envline   = (char *)Dee_Mallocc(utf8_name_len + 1 + utf8_value_len + 1, sizeof(char));
	if unlikely(!utf8_envline)
		goto err;
	{
		char *iter;
		iter = utf8_envline;
		iter = (char *)mempcpyc(iter, utf8_name, utf8_name_len, sizeof(char));
		*iter++ = '=';
		iter = (char *)mempcpyc(iter, utf8_value, utf8_value_len, sizeof(char));
		*iter = '\0';
	}
again_insert_env:
	environ_lock_write();
	if (environ) {
		for (i = 0; (utf8_oldline = environ[i]) != NULL; ++i) {
			if (bcmpc(utf8_oldline, utf8_name, utf8_name_len, sizeof(char)) == 0 &&
			    utf8_oldline[utf8_name_len] == '=') {
				if (replace) {
					if (!try_set_system_environ_is_dee_heap_allocated())
						goto unlock_and_try_collect_memory;
					utf8_oldline = environ[i];
					environ[i]   = utf8_envline;
				} else {
					utf8_oldline = NULL;
				}
				/* Variable already exists, and we're not supposed to replace it. */
				DBG_ALIGNMENT_ENABLE();
				if (replace)
					environ_changed();
				environ_lock_endwrite();
				if (!replace)
					Dee_Free(utf8_envline);
				Dee_Free(utf8_oldline);
				if (replace)
					return DeeNotify_Broadcast(Dee_NOTIFICATION_CLASS_ENVIRON, (DeeObject *)name);
				return 0;
			}
		}
	}
	if (!try_set_system_environ_is_dee_heap_allocated())
		goto unlock_and_try_collect_memory;
	/* Append a new line to `environ' */
	{
		char **new_environ;
		size_t old_environ_count;
		for (old_environ_count = 0; environ[old_environ_count]; ++old_environ_count)
			;
		new_environ = (char **)Dee_TryReallocc(environ, old_environ_count + 2, sizeof(char *));
		if unlikely(!new_environ)
			goto unlock_and_try_collect_memory;
		new_environ[old_environ_count + 0] = utf8_envline;
		new_environ[old_environ_count + 1] = NULL;
		environ = new_environ;
	}
	environ_changed();
	environ_lock_endwrite();
	return DeeNotify_Broadcast(Dee_NOTIFICATION_CLASS_ENVIRON, (DeeObject *)name);
unlock_and_try_collect_memory:
	environ_lock_endwrite();
	if (Dee_CollectMemory(1))
		goto again_insert_env;
	/*goto err_utf8_envline;*/
err_utf8_envline:
	Dee_Free(utf8_envline);
err:
	return -1;
#endif /* posix_setenv_USE_environ */

#ifdef posix_setenv_USE_STUB
	(void)name;
	(void)value;
	(void)replace;
	return DeeError_Throwf(&DeeError_UnsupportedAPI,
	                       "No way to set environment variable `%k' to `%k'",
	                       name, value);
#endif /* posix_setenv_USE_STUB */
}

/* @return:  0: Success
 * @return:  1: Not deleted because never defined
 * @return: -1: Error */
PRIVATE WUNUSED NONNULL((1)) int DCALL
posix_environ_unsetenv(DeeStringObject *__restrict name) {
#ifdef posix_unsetenv_USE_SetEnvironmentVariableW
	LPWSTR wname;
	wname = (LPWSTR)DeeString_AsWide((DeeObject *)name);
	if unlikely(!wname)
		goto err;
	environ_lock_write();
	DBG_ALIGNMENT_DISABLE();
	if (!SetEnvironmentVariableW(wname, NULL)) {
		DBG_ALIGNMENT_ENABLE();
		environ_lock_endwrite();
		return 1;
	}
	DBG_ALIGNMENT_ENABLE();
	environ_changed();
	environ_lock_endwrite();
	return DeeNotify_Broadcast(Dee_NOTIFICATION_CLASS_ENVIRON, (DeeObject *)name);
err:
	return -1;
#endif /* posix_unsetenv_USE_SetEnvironmentVariableW */

#ifdef posix_unsetenv_USE_wunsetenv
	Dee_wchar_t const *wname;
	wname = DeeString_AsWide((DeeObject *)name);
	if unlikely(!wname)
		goto err;
	environ_lock_write();
	DBG_ALIGNMENT_DISABLE();
	if (wunsetenv((wchar_t *)wname) != 0) {
		DBG_ALIGNMENT_ENABLE();
		environ_lock_endwrite();
		return 1;
	}
	DBG_ALIGNMENT_ENABLE();
	environ_changed();
	environ_lock_endwrite();
	return DeeNotify_Broadcast(Dee_NOTIFICATION_CLASS_ENVIRON, (DeeObject *)name);
err:
	return -1;
#endif /* posix_unsetenv_USE_wunsetenv */

#ifdef posix_unsetenv_USE_unsetenv
	char const *utf8_name;
	utf8_name = DeeString_AsUtf8((DeeObject *)name);
	if unlikely(!utf8_name)
		goto err;
	environ_lock_write();
	DBG_ALIGNMENT_DISABLE();
	if (unsetenv((char *)utf8_name) != 0) {
		DBG_ALIGNMENT_ENABLE();
		environ_lock_endwrite();
		return 1;
	}
	DBG_ALIGNMENT_ENABLE();
	environ_changed();
	environ_lock_endwrite();
	return DeeNotify_Broadcast(Dee_NOTIFICATION_CLASS_ENVIRON, (DeeObject *)name);
err:
	return -1;
#endif /* posix_unsetenv_USE_unsetenv */

#ifdef posix_unsetenv_USE_wputenv
	Dee_wchar_t const *wname;
	wname = DeeString_AsWide((DeeObject *)name);
	if unlikely(!wname)
		goto err;
	environ_lock_write();
	DBG_ALIGNMENT_DISABLE();
	if (wcschr((wchar_t *)wname, (wchar_t)'=') ||
	    wputenv((wchar_t *)wname) != 0) {
		DBG_ALIGNMENT_ENABLE();
		environ_lock_endwrite();
		return 1;
	}
	DBG_ALIGNMENT_ENABLE();
	environ_changed();
	environ_lock_endwrite();
	return DeeNotify_Broadcast(Dee_NOTIFICATION_CLASS_ENVIRON, (DeeObject *)name);
err:
	return -1;
#endif /* posix_unsetenv_USE_wputenv */

#ifdef posix_unsetenv_USE_putenv
	char const *utf8_name;
	utf8_name = DeeString_AsUtf8((DeeObject *)name);
	if unlikely(!utf8_name)
		goto err;
	environ_lock_write();
	DBG_ALIGNMENT_DISABLE();
	if (strchr(utf8_name, '=') || putenv((char *)utf8_name) != 0) {
		DBG_ALIGNMENT_ENABLE();
		environ_lock_endwrite();
		return 1;
	}
	DBG_ALIGNMENT_ENABLE();
	environ_changed();
	environ_lock_endwrite();
	return DeeNotify_Broadcast(Dee_NOTIFICATION_CLASS_ENVIRON, (DeeObject *)name);
err:
	return -1;
#endif /* posix_unsetenv_USE_putenv */

#ifdef posix_unsetenv_USE_wenviron
	size_t i;
	Dee_wchar_t const *wide_name;
	Dee_wchar_t *wide_oldline;
	size_t wide_name_len;
	wide_name = DeeString_AsWide((DeeObject *)name);
	if unlikely(!wide_name)
		goto err;
	wide_name_len = wcslen((wchar_t *)wide_name);
again_search_env:
	environ_lock_write();
	if (wenviron) {
		for (i = 0; (wide_oldline = (Dee_wchar_t *)wenviron[i]) != NULL; ++i) {
			if (bcmpc(wide_oldline, wide_name, wide_name_len, sizeof(Dee_wchar_t)) == 0 &&
			    wide_oldline[wide_name_len] == '=') {
				Dee_wchar_t **new_wenviron;
				size_t wenviron_after;
				if (!try_set_system_wenviron_is_dee_heap_allocated())
					goto unlock_and_try_collect_memory;
				for (wenviron_after = 0; wenviron[i + 1 + wenviron_after]; ++wenviron_after)
					;
				wide_oldline = (Dee_wchar_t *)wenviron[i];
				memmovedownc(&wenviron[i], &wenviron[i + 1],
				             wenviron_after, sizeof(Dee_wchar_t *));
				i += wenviron_after;
				wenviron[i] = NULL;
				new_wenviron = (Dee_wchar_t **)Dee_TryReallocc(wenviron, i + 1, sizeof(Dee_wchar_t *));
				if likely(new_wenviron)
					wenviron = (wchar_t **)new_wenviron;
				DBG_ALIGNMENT_ENABLE();
				environ_changed();
				environ_lock_endwrite();
				Dee_Free(wide_oldline);
				return DeeNotify_Broadcast(Dee_NOTIFICATION_CLASS_ENVIRON, (DeeObject *)name);
			}
		}
	}
	environ_lock_endwrite();
	return 1;
unlock_and_try_collect_memory:
	environ_lock_endwrite();
	if (Dee_CollectMemory(1))
		goto again_search_env;
err:
	return -1;
#endif /* posix_unsetenv_USE_wenviron */

#ifdef posix_unsetenv_USE_environ
	size_t i;
	char const *utf8_name;
	char *utf8_oldline;
	size_t utf8_name_len;
	utf8_name = DeeString_AsUtf8((DeeObject *)name);
	if unlikely(!utf8_name)
		goto err;
	utf8_name_len = strlen(utf8_name);
again_search_env:
	environ_lock_write();
	if (environ) {
		for (i = 0; (utf8_oldline = environ[i]) != NULL; ++i) {
			if (bcmpc(utf8_oldline, utf8_name, utf8_name_len, sizeof(char)) == 0 &&
			    utf8_oldline[utf8_name_len] == '=') {
				char **new_environ;
				size_t environ_after;
				if (!try_set_system_environ_is_dee_heap_allocated())
					goto unlock_and_try_collect_memory;
				for (environ_after = 0; environ[i + 1 + environ_after]; ++environ_after)
					;
				utf8_oldline = environ[i];
				memmovedownc(&environ[i], &environ[i + 1],
				             environ_after, sizeof(char *));
				i += environ_after;
				environ[i] = NULL;
				new_environ = (char **)Dee_TryReallocc(environ, i + 1, sizeof(char *));
				if likely(new_environ)
					environ = new_environ;
				DBG_ALIGNMENT_ENABLE();
				environ_changed();
				environ_lock_endwrite();
				Dee_Free(utf8_oldline);
				return DeeNotify_Broadcast(Dee_NOTIFICATION_CLASS_ENVIRON, (DeeObject *)name);
			}
		}
	}
	environ_lock_endwrite();
	return 1;
unlock_and_try_collect_memory:
	environ_lock_endwrite();
	if (Dee_CollectMemory(1))
		goto again_search_env;
err:
	return -1;
#endif /* posix_unsetenv_USE_environ */

#ifdef posix_unsetenv_USE_STUB
	(void)name;
	return DeeError_Throwf(&DeeError_UnsupportedAPI,
	                       "No way to delete environment variable `%k'",
	                       name);
#endif /* posix_unsetenv_USE_STUB */
}

PRIVATE WUNUSED int DCALL
posix_environ_clearenv(void) {
#ifdef posix_clearenv_USE_SetEnvironmentStringsW
	static WCHAR empty_wenviron[] = { '\0', '\0' };
	environ_lock_write();
	DBG_ALIGNMENT_DISABLE();
	if (!SetEnvironmentStringsW(empty_wenviron)) {
		DWORD dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		environ_lock_endwrite();
		return DeeNTSystem_ThrowErrorf(&DeeError_SystemError, dwError,
		                               "Failed clear environment variables");
	}
	DBG_ALIGNMENT_ENABLE();
	environ_changed();
	environ_lock_endwrite();
	return 0;
#endif /* posix_clearenv_USE_SetEnvironmentStringsW */

#if defined(posix_clearenv_USE_wclearenv) || defined(posix_clearenv_USE_clearenv)
	environ_lock_write();
	DBG_ALIGNMENT_DISABLE();
#ifdef posix_clearenv_USE_wclearenv
	if (wclearenv() != 0)
#else /* posix_clearenv_USE_wclearenv */
	if (clearenv() != 0)
#endif /* !posix_clearenv_USE_wclearenv */
	{
		int error = DeeSystem_GetErrno();
		DBG_ALIGNMENT_ENABLE();
		environ_lock_endwrite();
		return DeeUnixSystem_ThrowErrorf(&DeeError_SystemError, error,
		                                 "Failed clear environment variables");
	}
	DBG_ALIGNMENT_ENABLE();
	environ_changed();
	environ_lock_endwrite();
	return 0;
#endif /* posix_clearenv_USE_wclearenv */

#ifdef posix_clearenv_USE_wunsetenv
	environ_lock_write();
	DBG_ALIGNMENT_DISABLE();
#ifdef posix_clearenv_USE_wclearenv
	if (wclearenv() != 0)
#else /* posix_clearenv_USE_wclearenv */
	if (clearenv() != 0)
#endif /* !posix_clearenv_USE_wclearenv */
	{
		int error = DeeSystem_GetErrno();
		DBG_ALIGNMENT_ENABLE();
		environ_lock_endwrite();
		return DeeUnixSystem_ThrowErrorf(&DeeError_SystemError, error,
		                                 "Failed clear environment variables");
	}
	DBG_ALIGNMENT_ENABLE();
	environ_lock_endwrite();
	return 0;
#endif /* posix_clearenv_USE_wunsetenv */

#if defined(posix_clearenv_USE_wunsetenv) || defined(posix_clearenv_USE_wputenv)
	Dee_wchar_t *namebuf = NULL;
	size_t namebuflen = 0;
again:
	environ_lock_write();
	while (wenviron && *wenviron) {
		size_t namelen;
		Dee_wchar_t const *envline = (Dee_wchar_t const *)*wenviron;
		for (namelen = 0; envline[namelen] && envline[namelen] != (Dee_wchar_t)'='; ++namelen)
			;
		++namelen; /* Trailing NUL */
		if (namelen > namebuflen) {
			Dee_wchar_t *new_namebuf;
			new_namebuf = (Dee_wchar_t *)Dee_TryReallocc(namebuf, namelen, sizeof(Dee_wchar_t));
			if unlikely(!new_namebuf) {
				environ_lock_endwrite();
				if (Dee_CollectMemory(1))
					goto again;
				return -1;
			}
		}
		*(Dee_wchar_t *)mempcpyc(namebuf, envline, namelen, sizeof(Dee_wchar_t)) = (Dee_wchar_t)'\0';
#ifdef posix_clearenv_USE_unsetenv
		if (wunsetenv(namebuf))
#else /* posix_clearenv_USE_unsetenv */
		if (wputenv(namebuf))
#endif /* !posix_clearenv_USE_unsetenv */
		{
			int error = DeeSystem_GetErrno();
			environ_lock_endwrite();
			DeeUnixSystem_ThrowErrorf(&DeeError_SystemError, error,
			                          "Unable to delete environment variable %q",
			                          namebuf);
			Dee_Free(namebuf);
			return -1;
		}
	}
	environ_changed();
	environ_lock_endwrite();
	Dee_Free(namebuf);
	return 0;
#endif /* posix_clearenv_USE_unsetenv || posix_clearenv_USE_putenv */

#if defined(posix_clearenv_USE_unsetenv) || defined(posix_clearenv_USE_putenv)
	char *namebuf = NULL;
	size_t namebuflen = 0;
again:
	environ_lock_write();
	while (environ && *environ) {
		size_t namelen;
		char const *envline = *environ;
		for (namelen = 0; envline[namelen] && envline[namelen] != '='; ++namelen)
			;
		++namelen; /* Trailing NUL */
		if (namelen > namebuflen) {
			char *new_namebuf;
			new_namebuf = (char *)Dee_TryReallocc(namebuf, namelen, sizeof(char));
			if unlikely(!new_namebuf) {
				environ_lock_endwrite();
				if (Dee_CollectMemory(1))
					goto again;
				return -1;
			}
		}
		*(char *)mempcpyc(namebuf, envline, namelen, sizeof(char)) = '\0';
#ifdef posix_clearenv_USE_unsetenv
		if (unsetenv(namebuf))
#else /* posix_clearenv_USE_unsetenv */
		if (putenv(namebuf))
#endif /* !posix_clearenv_USE_unsetenv */
		{
			int error = DeeSystem_GetErrno();
			environ_lock_endwrite();
			DeeUnixSystem_ThrowErrorf(&DeeError_SystemError, error,
			                          "Unable to delete environment variable %q",
			                          namebuf);
			Dee_Free(namebuf);
			return -1;
		}
	}
	environ_changed();
	environ_lock_endwrite();
	Dee_Free(namebuf);
	return 0;
#endif /* posix_clearenv_USE_unsetenv || posix_clearenv_USE_putenv */

#ifdef posix_clearenv_USE_environ_setempty
	if (environ && *environ) {
#ifdef HAVE_system_environ_is_dee_heap_allocated
		char **old_environ;
#endif /* HAVE_system_environ_is_dee_heap_allocated */
		char **new_environ;
again_alloc_empty:
#ifdef HAVE_system_environ_is_dee_heap_allocated
		new_environ = (char **)Dee_TryCallocc(1, sizeof(char *));
#else /* HAVE_system_environ_is_dee_heap_allocated */
		new_environ = (char **)calloc(1, sizeof(char *));
#endif /* !HAVE_system_environ_is_dee_heap_allocated */
		if unlikely(!new_environ) {
			if (Dee_TryCollectMemory(1))
				goto again_alloc_empty;
			return -1;
		}
		environ_lock_write();
#ifdef HAVE_system_environ_is_dee_heap_allocated
		old_environ = environ;
#endif /* HAVE_system_environ_is_dee_heap_allocated */
		environ = new_environ;
#ifdef HAVE_system_environ_is_dee_heap_allocated
		if (!system_environ_is_dee_heap_allocated)
			old_environ = NULL;
		system_environ_is_dee_heap_allocated = true;
#endif /* HAVE_system_environ_is_dee_heap_allocated */
		environ_lock_endwrite();
#ifdef HAVE_system_environ_is_dee_heap_allocated
		if (old_environ) {
			size_t i;
			char *envstr;
			for (i = 0; (envstr = old_environ[i]) != NULL; ++i)
				Dee_Free(envstr);
			Dee_Free(old_environ);
		}
#endif /* HAVE_system_environ_is_dee_heap_allocated */
	}
#endif /* posix_clearenv_USE_environ_setempty */

#ifdef posix_clearenv_USE_wenviron_setempty
	if (wenviron && *wenviron) {
#ifdef HAVE_system_wenviron_is_dee_heap_allocated
		Dee_wchar_t **old_wenviron;
#endif /* HAVE_system_wenviron_is_dee_heap_allocated */
		Dee_wchar_t **new_wenviron;
again_alloc_empty:
#ifdef HAVE_system_wenviron_is_dee_heap_allocated
		new_wenviron = (Dee_wchar_t **)Dee_TryCallocc(1, sizeof(Dee_wchar_t *));
#else /* HAVE_system_wenviron_is_dee_heap_allocated */
		new_wenviron = (Dee_wchar_t **)calloc(1, sizeof(Dee_wchar_t *));
#endif /* !HAVE_system_wenviron_is_dee_heap_allocated */
		if unlikely(!new_wenviron) {
			if (Dee_TryCollectMemory(1))
				goto again_alloc_empty;
			return -1;
		}
		environ_lock_write();
#ifdef HAVE_system_wenviron_is_dee_heap_allocated
		old_wenviron = (Dee_wchar_t **)wenviron;
#endif /* HAVE_system_wenviron_is_dee_heap_allocated */
		wenviron = (wchar_t **)new_wenviron;
#ifdef HAVE_system_wenviron_is_dee_heap_allocated
		if (!system_wenviron_is_dee_heap_allocated)
			old_wenviron = NULL;
		system_wenviron_is_dee_heap_allocated = true;
#endif /* HAVE_system_wenviron_is_dee_heap_allocated */
		environ_changed();
		environ_lock_endwrite();
#ifdef HAVE_system_wenviron_is_dee_heap_allocated
		if (old_wenviron) {
			size_t i;
			Dee_wchar_t *envstr;
			for (i = 0; (envstr = old_wenviron[i]) != NULL; ++i)
				Dee_Free(envstr);
			Dee_Free(old_wenviron);
		}
#endif /* HAVE_system_wenviron_is_dee_heap_allocated */
	}
#endif /* posix_clearenv_USE_wenviron_setempty */

#ifdef posix_clearenv_USE_STUB
	return DeeError_Throwf(&DeeError_UnsupportedAPI,
	                       "No way to clear environment variables");
#endif /* posix_clearenv_USE_STUB */
}


#ifdef posix_enumenv_USE_STUB
PRIVATE ATTR_COLD int DCALL err_environ_enum_not_supported(void) {
	return DeeError_Throwf(&DeeError_UnsupportedAPI,
	                       "No way to enumerate environment variables");
}
#endif /* posix_enumenv_USE_STUB */

PRIVATE WUNUSED size_t DCALL
posix_environ_getcount(void) {
#ifdef posix_enumenv_USE_GetEnvironmentStringsW
	size_t result = 0;
	LPWCH strings = GetEnvironmentStringsW();
	if (strings != NULL) {
		Dee_wchar_t *iter;
		for (iter = strings; *iter != (Dee_wchar_t)'\0';
		     iter = wcsend(iter) + 1)
			++result;
		FreeEnvironmentStringsW(strings);
	}
	return result;
#endif /* posix_enumenv_USE_GetEnvironmentStringsW */

#ifdef posix_enumenv_USE_environ
	size_t result = 0;
	environ_lock_read();
	if (environ) {
		while (environ[result])
			++result;
	}
	environ_lock_endread();
	return result;
#endif /* posix_enumenv_USE_environ */

#ifdef posix_enumenv_USE_wenviron
	size_t result = 0;
	environ_lock_read();
	if (wenviron) {
		while (wenviron[result])
			++result;
	}
	environ_lock_endread();
	return result;
#endif /* posix_enumenv_USE_wenviron */

#ifdef posix_enumenv_USE_STUB
	return (size_t)err_environ_enum_not_supported();
#endif /* posix_enumenv_USE_STUB */
}




/************************************************************************/
/* ENVIRON ITERATOR OBJECT                                              */
/************************************************************************/
struct environ_iterator_object;
typedef struct environ_iterator_object EnvironIterator;
struct environ_iterator_object {
	OBJECT_HEAD
#ifdef posix_enumenv_USE_GetEnvironmentStringsW
	Dee_wchar_t  *ei_environment_iter;    /* [1..1][lock(ATOMIC)] Pointer to next environment string to yield */
	LPWCH         ei_environment_strings; /* [1..1][owned] Environment strings */
	DREF EnvironIterator *ei_owner;    /* [0..1][const] Owning iterator (for `operator copy') */
#define ENVIRON_ITERATOR_tchar                  WCHAR
#define ENVIRON_ITERATOR_strchr                 wcschr
#define ENVIRON_ITERATOR_strlen                 wcslen
#define ENVIRON_ITERATOR_strend                 wcsend
#define ENVIRON_ITERATOR_DeeString_TryNewBuffer DeeString_TryNewWideBuffer
#define ENVIRON_ITERATOR_DeeString_NewBuffer    DeeString_NewWideBuffer
#define ENVIRON_ITERATOR_DeeString_FreeBuffer   DeeString_FreeWideBuffer
#define ENVIRON_ITERATOR_DeeString_PackBuffer   DeeString_PackWideBuffer
#endif /* posix_enumenv_USE_GetEnvironmentStringsW */

#ifdef posix_enumenv_USE_environ
	size_t     ei_index;           /* [lock(ATOMIC)] Index of next string to yield from `ei_environ' */
	char     **ei_environ;         /* [0..1][lock(dee_environ_lock)][0..n][const] Environ table */
	size_t     ei_environ_version; /* [const] Environ version loaded into `ei_environ' */
#define ENVIRON_ITERATOR_tchar                  char
#define ENVIRON_ITERATOR_strchr                 strchr
#define ENVIRON_ITERATOR_strlen                 strlen
#define ENVIRON_ITERATOR_strend                 strend
#define ENVIRON_ITERATOR_DeeString_TryNewBuffer (char *)DeeString_TryNew1ByteBuffer
#define ENVIRON_ITERATOR_DeeString_NewBuffer    (char *)DeeString_New1ByteBuffer
#define ENVIRON_ITERATOR_DeeString_FreeBuffer(buffer)   DeeString_Free1ByteBuffer((uint8_t *)(buffer))
#define ENVIRON_ITERATOR_DeeString_PackBuffer(buffer, error_mode) DeeString_PackUtf8Buffer((uint8_t *)(buffer), error_mode)
#endif /* posix_enumenv_USE_environ */

#ifdef posix_enumenv_USE_wenviron
	size_t        ei_index;           /* [lock(ATOMIC)] Index of next string to yield from `ei_environ' */
	Dee_wchar_t **ei_environ;         /* [0..1][lock(dee_environ_lock)][0..n][const] Environ table */
	size_t        ei_environ_version; /* [const] Environ version loaded into `ei_environ' */
#define ENVIRON_ITERATOR_tchar                  Dee_wchar_t
#define ENVIRON_ITERATOR_strchr                 wcschr
#define ENVIRON_ITERATOR_strlen                 wcslen
#define ENVIRON_ITERATOR_strend                 wcsend
#define ENVIRON_ITERATOR_DeeString_TryNewBuffer DeeString_TryNewWideBuffer
#define ENVIRON_ITERATOR_DeeString_NewBuffer    DeeString_NewWideBuffer
#define ENVIRON_ITERATOR_DeeString_FreeBuffer   DeeString_FreeWideBuffer
#define ENVIRON_ITERATOR_DeeString_PackBuffer   DeeString_PackWideBuffer
#endif /* posix_enumenv_USE_wenviron */
};

PRIVATE int DCALL
environ_iterator_init(EnvironIterator *__restrict self) {
#ifdef posix_enumenv_USE_GetEnvironmentStringsW
again_GetEnvironmentStringsW:
	DBG_ALIGNMENT_DISABLE();
	self->ei_environment_strings = GetEnvironmentStringsW();
	if unlikely(!self->ei_environment_strings) {
		DWORD dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsBadAllocError(dwError)) {
			if (Dee_CollectMemory(1))
				goto again_GetEnvironmentStringsW;
		} else {
			DeeNTSystem_ThrowErrorf(&DeeError_SystemError, dwError,
			                        "Failed to enumerate environment variables");
		}
		return -1;
	}
	DBG_ALIGNMENT_ENABLE();
	self->ei_environment_iter = self->ei_environment_strings;
	self->ei_owner            = NULL;
	return 0;
#endif /* posix_enumenv_USE_GetEnvironmentStringsW */

#if defined(posix_enumenv_USE_environ) || defined(posix_enumenv_USE_wenviron)
	environ_lock_read();
#ifdef posix_enumenv_USE_environ
	self->ei_environ = environ;
#endif /* posix_enumenv_USE_environ */
#ifdef posix_enumenv_USE_wenviron
	self->ei_environ = (Dee_wchar_t **)wenviron;
#endif /* posix_enumenv_USE_wenviron */
	if (!self->ei_environ) {
		static ENVIRON_ITERATOR_tchar *const empty_environ[] = { NULL };
		self->ei_environ = (ENVIRON_ITERATOR_tchar **)empty_environ;
	}
	self->ei_index           = 0;
	self->ei_environ_version = dee_environ_version;
	environ_lock_endread();
	return 0;
#endif /* posix_enumenv_USE_environ || posix_enumenv_USE_wenviron */

#ifdef posix_enumenv_USE_STUB
	(void)self;
	return err_environ_enum_not_supported();
#endif /* posix_enumenv_USE_STUB */
}

PRIVATE int DCALL
environ_iterator_copy(EnvironIterator *__restrict self,
                      EnvironIterator *__restrict other) {
#ifdef posix_enumenv_USE_GetEnvironmentStringsW
	self->ei_environment_strings = other->ei_environment_strings;
	self->ei_environment_iter    = atomic_read(&other->ei_environment_iter);

	/* Set the "owner" field to prevent `self->ei_environment_strings'
	 * from being freed before we are finished with it! */
	self->ei_owner = other->ei_owner;
	if (self->ei_owner == NULL)
		self->ei_owner = other;
	Dee_Decref(self->ei_owner);
	return 0;
#endif /* posix_enumenv_USE_GetEnvironmentStringsW */

#if defined(posix_enumenv_USE_environ) || defined(posix_enumenv_USE_wenviron)
	self->ei_environ         = other->ei_environ;
	self->ei_environ_version = other->ei_environ_version;
	self->ei_index           = atomic_read(&other->ei_index);
	environ_lock_endread();
	return 0;
#endif /* posix_enumenv_USE_environ || posix_enumenv_USE_wenviron */

#ifdef posix_enumenv_USE_STUB
	(void)self;
	(void)other;
	return err_environ_enum_not_supported();
#endif /* posix_enumenv_USE_STUB */
}

#ifdef posix_enumenv_USE_GetEnvironmentStringsW
#define HAVE_environ_iterator_fini
PRIVATE void DCALL
environ_iterator_fini(EnvironIterator *__restrict self) {
	FreeEnvironmentStringsW(self->ei_environment_strings);
	Dee_XDecref(self->ei_owner);
}
#endif /* posix_enumenv_USE_GetEnvironmentStringsW */


#if defined(posix_enumenv_USE_environ) || defined(posix_enumenv_USE_wenviron)
#ifndef err_changed_sequence
#define err_changed_sequence err_changed_sequence
INTERN ATTR_COLD NONNULL((1)) int
(DCALL err_changed_sequence)(DeeObject *__restrict seq) {
	ASSERT_OBJECT(seq);
	return DeeError_Throwf(&DeeError_RuntimeError,
	                       "A sequence `%k' has changed while being iterated: `%k'",
	                       Dee_TYPE(seq), seq);
}
#endif /* !err_changed_sequence */
#endif /* posix_enumenv_USE_environ || posix_enumenv_USE_wenviron */


PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
environ_iterator_nextpair(EnvironIterator *__restrict self,
                          DREF DeeObject *key_and_value[2]) {
#if (defined(posix_enumenv_USE_GetEnvironmentStringsW) || \
     (defined(posix_enumenv_USE_environ) || defined(posix_enumenv_USE_wenviron)))
	DREF DeeObject *key, *value;
	size_t key_len, value_len;
	ENVIRON_ITERATOR_tchar *key_buf, *value_buf;
	ENVIRON_ITERATOR_tchar *envline, *eq;
#ifndef posix_enumenv_USE_GetEnvironmentStringsW
	size_t line_index;
#endif /* !posix_enumenv_USE_GetEnvironmentStringsW */
again:
	environ_lock_read();

	/* Check if environ was modified since the iterator was created */
#if defined(posix_enumenv_USE_environ) || defined(posix_enumenv_USE_wenviron)
	if (self->ei_environ_version != dee_environ_version) {
		environ_lock_endread();
		err_changed_sequence((DeeObject *)self);
		goto err;
	}
#endif /* posix_enumenv_USE_environ || posix_enumenv_USE_wenviron */

	/* Load next environ line and check if EOF has been reached */
#ifdef posix_enumenv_USE_GetEnvironmentStringsW
	envline = atomic_read(&self->ei_environment_iter);
	if (!*envline)
#else /* posix_enumenv_USE_GetEnvironmentStringsW */
	line_index = atomic_read(&self->ei_index);
	envline    = self->ei_environ[line_index];
	if (!envline)
#endif /* !posix_enumenv_USE_GetEnvironmentStringsW */
	{
		environ_lock_endread();
		return 1;
	}
	
	/* Split the environ line */
	eq = ENVIRON_ITERATOR_strchr(envline, (ENVIRON_ITERATOR_tchar)'=');
	if (eq == NULL) {
		key_len = ENVIRON_ITERATOR_strlen(envline);
		eq      = envline + key_len;
	} else {
		key_len = (size_t)(eq - envline);
		++eq;
	}

	/* Allocate buffers for the key/value parts */
	value_len = ENVIRON_ITERATOR_strlen(eq);
	key_buf   = ENVIRON_ITERATOR_DeeString_TryNewBuffer(key_len);
	if unlikely(!key_buf) {
		environ_lock_endread();
		if (Dee_CollectMemory(1))
			goto again;
		goto err;
	}
	value_buf = ENVIRON_ITERATOR_DeeString_TryNewBuffer(value_len);
	if unlikely(!value_buf) {
		environ_lock_endread();
		ENVIRON_ITERATOR_DeeString_FreeBuffer(key_buf);
		if (Dee_CollectMemory(1))
			goto again;
		goto err;
	}

	/* Advance iterator */
#ifdef posix_enumenv_USE_GetEnvironmentStringsW
	{
		ENVIRON_ITERATOR_tchar *new_iter;
		new_iter = eq + value_len + 1;
		if (!atomic_cmpxch_weak_or_write(&self->ei_environment_iter, envline, new_iter)) {
			environ_lock_endread();
			ENVIRON_ITERATOR_DeeString_FreeBuffer(key_buf);
			ENVIRON_ITERATOR_DeeString_FreeBuffer(value_buf);
			goto again;
		}
	}
#else /* posix_enumenv_USE_GetEnvironmentStringsW */
	if (!atomic_cmpxch_weak_or_write(&self->ei_index, line_index, line_index + 1)) {
		environ_lock_endread();
		ENVIRON_ITERATOR_DeeString_FreeBuffer(key_buf);
		ENVIRON_ITERATOR_DeeString_FreeBuffer(value_buf);
		goto again;
	}
#endif /* !posix_enumenv_USE_GetEnvironmentStringsW */

	/* Load environ strings */
	memcpyc(key_buf, envline, key_len, sizeof(ENVIRON_ITERATOR_tchar));
	memcpyc(value_buf, eq, value_len, sizeof(ENVIRON_ITERATOR_tchar));
	environ_lock_endread();

	/* Pack environ string buffers */
	key = ENVIRON_ITERATOR_DeeString_PackBuffer(key_buf, STRING_ERROR_FREPLAC);
	if unlikely(!key)
		goto err_value;
	value = ENVIRON_ITERATOR_DeeString_PackBuffer(value_buf, STRING_ERROR_FREPLAC);
	if unlikely(!key) {
		Dee_Decref(key);
		goto err;
	}

	/* Pack everything together */
	key_and_value[0] = key;
	key_and_value[1] = value;
	return 0;
err_value:
	ENVIRON_ITERATOR_DeeString_FreeBuffer(value_buf);
err:
	return -1;
#endif /* ... */

#ifdef posix_enumenv_USE_STUB
	(void)self;
	err_environ_enum_not_supported();
	return -1;
#endif /* posix_enumenv_USE_STUB */
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
environ_iterator_nextkey(EnvironIterator *__restrict self) {
#if (defined(posix_enumenv_USE_GetEnvironmentStringsW) || \
     (defined(posix_enumenv_USE_environ) || defined(posix_enumenv_USE_wenviron)))
	size_t key_len;
#ifdef posix_enumenv_USE_GetEnvironmentStringsW
	size_t value_len;
#endif /* posix_enumenv_USE_GetEnvironmentStringsW */
	ENVIRON_ITERATOR_tchar *key_buf;
	ENVIRON_ITERATOR_tchar *envline, *eq;
#ifndef posix_enumenv_USE_GetEnvironmentStringsW
	size_t line_index;
#endif /* !posix_enumenv_USE_GetEnvironmentStringsW */
again:
	environ_lock_read();

	/* Check if environ was modified since the iterator was created */
#if defined(posix_enumenv_USE_environ) || defined(posix_enumenv_USE_wenviron)
	if (self->ei_environ_version != dee_environ_version) {
		environ_lock_endread();
		err_changed_sequence((DeeObject *)self);
		goto err;
	}
#endif /* posix_enumenv_USE_environ || posix_enumenv_USE_wenviron */

	/* Load next environ line and check if EOF has been reached */
#ifdef posix_enumenv_USE_GetEnvironmentStringsW
	envline = atomic_read(&self->ei_environment_iter);
	if (!*envline)
#else /* posix_enumenv_USE_GetEnvironmentStringsW */
	line_index = atomic_read(&self->ei_index);
	envline    = self->ei_environ[line_index];
	if (!envline)
#endif /* !posix_enumenv_USE_GetEnvironmentStringsW */
	{
		environ_lock_endread();
		return ITER_DONE;
	}
	
	/* Split the environ line */
	eq = ENVIRON_ITERATOR_strchr(envline, (ENVIRON_ITERATOR_tchar)'=');
	if (eq == NULL) {
		key_len = ENVIRON_ITERATOR_strlen(envline);
		eq      = envline + key_len;
	} else {
		key_len = (size_t)(eq - envline);
		++eq;
	}

	/* Allocate buffer for the key-part */
#ifdef posix_enumenv_USE_GetEnvironmentStringsW
	value_len = ENVIRON_ITERATOR_strlen(eq);
#endif /* posix_enumenv_USE_GetEnvironmentStringsW */
	key_buf = ENVIRON_ITERATOR_DeeString_TryNewBuffer(key_len);
	if unlikely(!key_buf) {
		environ_lock_endread();
		if (Dee_CollectMemory(1))
			goto again;
		goto err;
	}

	/* Advance iterator */
#ifdef posix_enumenv_USE_GetEnvironmentStringsW
	{
		ENVIRON_ITERATOR_tchar *new_iter;
		new_iter = eq + value_len + 1;
		if (!atomic_cmpxch_weak_or_write(&self->ei_environment_iter, envline, new_iter)) {
			environ_lock_endread();
			ENVIRON_ITERATOR_DeeString_FreeBuffer(key_buf);
			goto again;
		}
	}
#else /* posix_enumenv_USE_GetEnvironmentStringsW */
	if (!atomic_cmpxch_weak_or_write(&self->ei_index, line_index, line_index + 1)) {
		environ_lock_endread();
		ENVIRON_ITERATOR_DeeString_FreeBuffer(key_buf);
		goto again;
	}
#endif /* !posix_enumenv_USE_GetEnvironmentStringsW */

	/* Load environ strings */
	memcpyc(key_buf, envline, key_len, sizeof(ENVIRON_ITERATOR_tchar));
	environ_lock_endread();

	/* Pack environ string buffers */
	return ENVIRON_ITERATOR_DeeString_PackBuffer(key_buf, STRING_ERROR_FREPLAC);
err:
	return NULL;
#endif /* ... */

#ifdef posix_enumenv_USE_STUB
	(void)self;
	err_environ_enum_not_supported();
	return NULL;
#endif /* posix_enumenv_USE_STUB */
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
environ_iterator_nextvalue(EnvironIterator *__restrict self) {
#if (defined(posix_enumenv_USE_GetEnvironmentStringsW) || \
     (defined(posix_enumenv_USE_environ) || defined(posix_enumenv_USE_wenviron)))
	size_t key_len, value_len;
	ENVIRON_ITERATOR_tchar *value_buf;
	ENVIRON_ITERATOR_tchar *envline, *eq;
#ifndef posix_enumenv_USE_GetEnvironmentStringsW
	size_t line_index;
#endif /* !posix_enumenv_USE_GetEnvironmentStringsW */
again:
	environ_lock_read();

	/* Check if environ was modified since the iterator was created */
#if defined(posix_enumenv_USE_environ) || defined(posix_enumenv_USE_wenviron)
	if (self->ei_environ_version != dee_environ_version) {
		environ_lock_endread();
		err_changed_sequence((DeeObject *)self);
		goto err;
	}
#endif /* posix_enumenv_USE_environ || posix_enumenv_USE_wenviron */

	/* Load next environ line and check if EOF has been reached */
#ifdef posix_enumenv_USE_GetEnvironmentStringsW
	envline = atomic_read(&self->ei_environment_iter);
	if (!*envline)
#else /* posix_enumenv_USE_GetEnvironmentStringsW */
	line_index = atomic_read(&self->ei_index);
	envline = self->ei_environ[line_index];
	if (!envline)
#endif /* !posix_enumenv_USE_GetEnvironmentStringsW */
	{
		environ_lock_endread();
		return ITER_DONE;
	}
	
	/* Split the environ line */
	eq = ENVIRON_ITERATOR_strchr(envline, (ENVIRON_ITERATOR_tchar)'=');
	if (eq == NULL) {
		key_len = ENVIRON_ITERATOR_strlen(envline);
		eq      = envline + key_len;
	} else {
		key_len = (size_t)(eq - envline);
		++eq;
	}

	/* Allocate buffer for the value-part */
	value_len = ENVIRON_ITERATOR_strlen(eq);
	value_buf = ENVIRON_ITERATOR_DeeString_TryNewBuffer(value_len);
	if unlikely(!value_buf) {
		environ_lock_endread();
		if (Dee_CollectMemory(1))
			goto again;
		goto err;
	}

	/* Advance iterator */
#ifdef posix_enumenv_USE_GetEnvironmentStringsW
	{
		ENVIRON_ITERATOR_tchar *new_iter;
		new_iter = eq + value_len + 1;
		if (!atomic_cmpxch_weak_or_write(&self->ei_environment_iter, envline, new_iter)) {
			environ_lock_endread();
			ENVIRON_ITERATOR_DeeString_FreeBuffer(value_buf);
			goto again;
		}
	}
#else /* posix_enumenv_USE_GetEnvironmentStringsW */
	if (!atomic_cmpxch_weak_or_write(&self->ei_index, line_index, line_index + 1)) {
		environ_lock_endread();
		ENVIRON_ITERATOR_DeeString_FreeBuffer(value_buf);
		goto again;
	}
#endif /* !posix_enumenv_USE_GetEnvironmentStringsW */

	/* Load environ strings */
	memcpyc(value_buf, eq, value_len, sizeof(ENVIRON_ITERATOR_tchar));
	environ_lock_endread();

	/* Pack environ string buffers */
	return ENVIRON_ITERATOR_DeeString_PackBuffer(value_buf, STRING_ERROR_FREPLAC);
err:
	return NULL;
#endif /* ... */

#ifdef posix_enumenv_USE_STUB
	(void)self;
	err_environ_enum_not_supported();
	return NULL;
#endif /* posix_enumenv_USE_STUB */
}

PRIVATE struct type_iterator environ_iterator_iterator = {
	/* .tp_nextpair  = */ (int (DCALL *)(DeeObject *__restrict, DREF DeeObject *[2]))&environ_iterator_nextpair,
	/* .tp_nextkey   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&environ_iterator_nextkey,
	/* .tp_nextvalue = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&environ_iterator_nextvalue,
	/* .tp_advance   = */ NULL,
};


PRIVATE WUNUSED NONNULL((1)) int DCALL
environ_iterator_bool(EnvironIterator *__restrict self) {
#if (defined(posix_enumenv_USE_GetEnvironmentStringsW) || \
     (defined(posix_enumenv_USE_environ) || defined(posix_enumenv_USE_wenviron)))
	ENVIRON_ITERATOR_tchar *envline;
	environ_lock_read();

	/* Check if environ was modified since the iterator was created */
#if defined(posix_enumenv_USE_environ) || defined(posix_enumenv_USE_wenviron)
	if (self->ei_environ_version != dee_environ_version) {
		environ_lock_endread();
		return err_changed_sequence((DeeObject *)self);
	}
#endif /* posix_enumenv_USE_environ || posix_enumenv_USE_wenviron */

	/* Load next environ line and check if EOF has been reached */
#ifdef posix_enumenv_USE_GetEnvironmentStringsW
	envline = atomic_read(&self->ei_environment_iter);
	if (!*envline)
#else /* posix_enumenv_USE_GetEnvironmentStringsW */
	{
		size_t line_index;
		line_index = atomic_read(&self->ei_index);
		envline    = self->ei_environ[line_index];
	}
	if (!envline)
#endif /* !posix_enumenv_USE_GetEnvironmentStringsW */
	{
		environ_lock_endread();
		return 0;
	}
	environ_lock_endread();
	return 1;
#endif /* ... */

#ifdef posix_enumenv_USE_STUB
	(void)self;
	return err_environ_enum_not_supported();
#endif /* posix_enumenv_USE_STUB */
}


PRIVATE DeeObject DeeEnviron_Singleton = { OBJECT_HEAD_INIT(&DeeEnviron_Type) };
PRIVATE struct type_member tpconst environ_iterator_members[] = {
	TYPE_MEMBER_CONST("seq", &DeeEnviron_Singleton),
#ifdef posix_enumenv_USE_GetEnvironmentStringsW
	TYPE_MEMBER_FIELD("__owner__", STRUCT_OBJECT_OPT, offsetof(EnvironIterator, ei_owner)),
#endif /* posix_enumenv_USE_GetEnvironmentStringsW */
#if defined(posix_enumenv_USE_environ) || defined(posix_enumenv_USE_wenviron)
	TYPE_MEMBER_FIELD("__index__", STRUCT_ATOMIC | STRUCT_SIZE_T, offsetof(EnvironIterator, ei_index)),
	TYPE_MEMBER_FIELD("__version__", STRUCT_CONST | STRUCT_SIZE_T, offsetof(EnvironIterator, ei_environ_version)),
#endif /* posix_enumenv_USE_environ || posix_enumenv_USE_wenviron */
	TYPE_MEMBER_END
};

INTERN DeeTypeObject DeeEnvironIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_EnvironIterator",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&environ_iterator_init,
				/* .tp_copy_ctor = */ (dfunptr_t)&environ_iterator_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&environ_iterator_copy,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(EnvironIterator)
			}
		},
#ifdef HAVE_environ_iterator_fini
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&environ_iterator_fini,
#else /* HAVE_environ_iterator_fini */
		/* .tp_dtor        = */ NULL,
#endif /* !HAVE_environ_iterator_fini */
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&environ_iterator_bool
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ &environ_iterator_iterator,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ environ_iterator_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};









/************************************************************************/
/* Portable deemon API                                                  */
/************************************************************************/
PRIVATE WUNUSED DREF DeeObject *DCALL environ_ctor(void) {
	return_reference_(&DeeEnviron_Singleton);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
environ_iter(DeeObject *__restrict UNUSED(self)) {
	return DeeObject_NewDefault(&DeeEnvironIterator_Type);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
environ_getitem(DeeObject *UNUSED(self), DeeObject *key) {
	DREF DeeObject *result;
	if (DeeObject_AssertTypeExact(key, &DeeString_Type))
		goto err;
	result = posix_environ_trygetenv((DeeStringObject *)key);
	if unlikely(result == ITER_DONE)
		goto err_not_found;
	return result;
err_not_found:
	err_unknown_env_var(key);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
environ_trygetitem(DeeObject *UNUSED(self), DeeObject *key) {
	if (DeeObject_AssertTypeExact(key, &DeeString_Type))
		goto err;
	return posix_environ_trygetenv((DeeStringObject *)key);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
environ_delitem(DeeObject *UNUSED(self), DeeObject *key) {
	int error;
	if (DeeObject_AssertTypeExact(key, &DeeString_Type))
		goto err;
	error = posix_environ_unsetenv((DeeStringObject *)key);
	if (error > 0)
		error = err_unknown_env_var(key);
	return error;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
environ_setitem(DeeObject *UNUSED(self),
                DeeObject *key, DeeObject *value) {
	if (DeeObject_AssertTypeExact(key, &DeeString_Type))
		goto err;
	if (DeeObject_AssertTypeExact(value, &DeeString_Type))
		goto err;
	return posix_environ_setenv((DeeStringObject *)key,
	                      (DeeStringObject *)value,
	                      true);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
environ_contains(DeeObject *UNUSED(self), DeeObject *key) {
	int exists;
	if (DeeObject_AssertTypeExact(key, &DeeString_Type))
		goto err;
	exists = posix_environ_hasenv((DeeStringObject *)key);
	if unlikely(exists < 0)
		goto err;
	return_bool(exists);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
environ_size(DeeObject *__restrict UNUSED(self)) {
	return posix_environ_getcount();
}


PRIVATE struct type_seq environ_seq = {
	/* .tp_iter                       = */ &environ_iter,
	/* .tp_sizeob                     = */ NULL,
	/* .tp_contains                   = */ &environ_contains,
	/* .tp_getitem                    = */ &environ_getitem,
	/* .tp_delitem                    = */ &environ_delitem,
	/* .tp_setitem                    = */ &environ_setitem,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ NULL,
	/* .tp_foreach_pair               = */ NULL, // TODO: (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&environ_foreach_pair,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&environ_size,
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ NULL,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ NULL,
	/* .tp_setitem_index              = */ NULL,
	/* .tp_bounditem_index            = */ NULL,
	/* .tp_hasitem_index              = */ NULL,
	/* .tp_getrange_index             = */ NULL,
	/* .tp_delrange_index             = */ NULL,
	/* .tp_setrange_index             = */ NULL,
	/* .tp_getrange_index_n           = */ NULL,
	/* .tp_delrange_index_n           = */ NULL,
	/* .tp_setrange_index_n           = */ NULL,
	/* .tp_trygetitem                 = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&environ_trygetitem,
#if 0
	/* .tp_trygetitem_index           = */ NULL,
	/* .tp_trygetitem_string_hash     = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&environ_trygetitem_string_hash,
	/* .tp_getitem_string_hash        = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&environ_getitem_string_hash,
	/* .tp_delitem_string_hash        = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&environ_delitem_string_hash,
	/* .tp_setitem_string_hash        = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t, DeeObject *))&environ_setitem_string_hash,
	/* .tp_bounditem_string_hash      = */ NULL,
	/* .tp_hasitem_string_hash        = */ NULL,
	/* .tp_trygetitem_string_len_hash = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&environ_trygetitem_string_len_hash,
	/* .tp_getitem_string_len_hash    = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&environ_getitem_string_len_hash,
	/* .tp_delitem_string_len_hash    = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&environ_delitem_string_len_hash,
	/* .tp_setitem_string_len_hash    = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t, DeeObject *))&environ_setitem_string_len_hash,
	/* .tp_bounditem_string_len_hash  = */ NULL,
	/* .tp_hasitem_string_len_hash    = */ NULL,
#endif
};

PRIVATE struct type_member tpconst environ_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &DeeEnvironIterator_Type),
	TYPE_MEMBER_END
};


INTERN DeeTypeObject DeeEnviron_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "environ",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FVARIABLE | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeMapping_Type,
	/* .tp_init = */ {
		{
			/* .tp_var = */ {
				/* .tp_ctor      = */ (dfunptr_t)&environ_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
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
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &environ_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ environ_class_members
};


/************************************************************************/
/* High-level environ API                                               */
/************************************************************************/

/*[[[deemon import("rt.gen.dexutils").gw("getenv", "varname:?Dstring,defl:?Dstring=NULL->?Dstring", libname: "posix", ispure: true); ]]]*/
FORCELOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL posix_getenv_f_impl(DeeObject *varname, DeeObject *defl);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_getenv_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_GETENV_DEF { "getenv", (DeeObject *)&posix_getenv, MODSYM_FREADONLY, DOC("(varname:?Dstring,defl?:?Dstring)->?Dstring") },
#define POSIX_GETENV_DEF_DOC(doc) { "getenv", (DeeObject *)&posix_getenv, MODSYM_FREADONLY, DOC("(varname:?Dstring,defl?:?Dstring)->?Dstring\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_getenv, &posix_getenv_f, METHOD_FPURECALL);
#ifndef DEFINED_kwlist__varname_defl
#define DEFINED_kwlist__varname_defl
PRIVATE DEFINE_KWLIST(kwlist__varname_defl, { KEX("varname", 0x4d384ee, 0x68ff72c018adb0c4), KEX("defl", 0x4353f18, 0x655c26b85fe0c07b), KEND });
#endif /* !DEFINED_kwlist__varname_defl */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_getenv_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *varname;
		DeeObject *defl;
	} args;
	args.defl = NULL;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__varname_defl, "o|o:getenv", &args))
		goto err;
	return posix_getenv_f_impl(args.varname, args.defl);
err:
	return NULL;
}
FORCELOCAL WUNUSED NONNULL((1))DREF DeeObject *DCALL posix_getenv_f_impl(DeeObject *varname, DeeObject *defl)
/*[[[end]]]*/
{
	DREF DeeObject *result;
	if (DeeObject_AssertTypeExact(varname, &DeeString_Type))
		goto err;
	result = posix_environ_trygetenv((DeeStringObject *)varname);
	if (result == ITER_DONE) {
		result = defl;
		if (result) {
			Dee_Incref(result);
		} else {
			err_unknown_env_var(varname);
		}
	}
	return result;
err:
	return NULL;
}


/*[[[deemon import("rt.gen.dexutils").gw("setenv", "varname:?Dstring,value:?Dstring,replace:c:bool=true", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL posix_setenv_f_impl(DeeObject *varname, DeeObject *value, bool replace);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_setenv_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_SETENV_DEF { "setenv", (DeeObject *)&posix_setenv, MODSYM_FREADONLY, DOC("(varname:?Dstring,value:?Dstring,replace=!t)") },
#define POSIX_SETENV_DEF_DOC(doc) { "setenv", (DeeObject *)&posix_setenv, MODSYM_FREADONLY, DOC("(varname:?Dstring,value:?Dstring,replace=!t)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_setenv, &posix_setenv_f, METHOD_FNORMAL);
#ifndef DEFINED_kwlist__varname_value_replace
#define DEFINED_kwlist__varname_value_replace
PRIVATE DEFINE_KWLIST(kwlist__varname_value_replace, { KEX("varname", 0x4d384ee, 0x68ff72c018adb0c4), KEX("value", 0xd9093f6e, 0x69e7413ae0c88471), KEX("replace", 0x54b94882, 0x2d4ba4f8cfd63bc6), KEND });
#endif /* !DEFINED_kwlist__varname_value_replace */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_setenv_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *varname;
		DeeObject *value;
		bool replace;
	} args;
	args.replace = true;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__varname_value_replace, "oo|b:setenv", &args))
		goto err;
	return posix_setenv_f_impl(args.varname, args.value, args.replace);
err:
	return NULL;
}
FORCELOCAL WUNUSED NONNULL((1, 2))DREF DeeObject *DCALL posix_setenv_f_impl(DeeObject *varname, DeeObject *value, bool replace)
/*[[[end]]]*/
{
	if (DeeObject_AssertTypeExact(varname, &DeeString_Type))
		goto err;
	if (DeeObject_AssertTypeExact(value, &DeeString_Type))
		goto err;
	if unlikely(posix_environ_setenv((DeeStringObject *)varname,
	                                 (DeeStringObject *)value,
	                                 replace))
		goto err;
	return_none;
err:
	return NULL;
}


/*[[[deemon import("rt.gen.dexutils").gw("putenv", "envline:?Dstring", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL posix_putenv_f_impl(DeeObject *envline);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_putenv_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_PUTENV_DEF { "putenv", (DeeObject *)&posix_putenv, MODSYM_FREADONLY, DOC("(envline:?Dstring)") },
#define POSIX_PUTENV_DEF_DOC(doc) { "putenv", (DeeObject *)&posix_putenv, MODSYM_FREADONLY, DOC("(envline:?Dstring)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_putenv, &posix_putenv_f, METHOD_FNORMAL);
#ifndef DEFINED_kwlist__envline
#define DEFINED_kwlist__envline
PRIVATE DEFINE_KWLIST(kwlist__envline, { KEX("envline", 0x86f5873f, 0x891a6b77bb84dc09), KEND });
#endif /* !DEFINED_kwlist__envline */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_putenv_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *envline;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__envline, "o:putenv", &args))
		goto err;
	return posix_putenv_f_impl(args.envline);
err:
	return NULL;
}
FORCELOCAL WUNUSED NONNULL((1))DREF DeeObject *DCALL posix_putenv_f_impl(DeeObject *envline)
/*[[[end]]]*/
{
	int error;
	char const *eq, *utf8_envline;
	if (DeeObject_AssertTypeExact(envline, &DeeString_Type))
		goto err;
	utf8_envline = DeeString_AsUtf8(envline);
	if unlikely(!utf8_envline)
		goto err;
	eq = strchr(utf8_envline, '=');
	if (eq) {
		/* Add to environ */
		DREF DeeStringObject *name, *value;
		name = (DREF DeeStringObject *)DeeString_NewUtf8(utf8_envline,
		                                                 (size_t)(eq - utf8_envline),
		                                                 STRING_ERROR_FSTRICT);
		if unlikely(!name)
			goto err;
		value = (DREF DeeStringObject *)DeeString_NewUtf8(eq + 1,
		                                                  strlen(eq + 1),
		                                                  STRING_ERROR_FSTRICT);
		if (!value) {
			Dee_Decref_likely(name);
			goto err;
		}
		error = posix_environ_setenv(name, value, true);
		Dee_Decref_likely(value);
		Dee_Decref_likely(name);
	} else {
		/* Remove from environ */
		error = posix_environ_unsetenv((DeeStringObject *)envline);
	}
	if unlikely(error)
		goto err;
	return_none;
err:
	return NULL;
}


/*[[[deemon import("rt.gen.dexutils").gw("unsetenv", "varname:?Dstring->?Dbool", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL posix_unsetenv_f_impl(DeeObject *varname);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_unsetenv_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_UNSETENV_DEF { "unsetenv", (DeeObject *)&posix_unsetenv, MODSYM_FREADONLY, DOC("(varname:?Dstring)->?Dbool") },
#define POSIX_UNSETENV_DEF_DOC(doc) { "unsetenv", (DeeObject *)&posix_unsetenv, MODSYM_FREADONLY, DOC("(varname:?Dstring)->?Dbool\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_unsetenv, &posix_unsetenv_f, METHOD_FNORMAL);
#ifndef DEFINED_kwlist__varname
#define DEFINED_kwlist__varname
PRIVATE DEFINE_KWLIST(kwlist__varname, { KEX("varname", 0x4d384ee, 0x68ff72c018adb0c4), KEND });
#endif /* !DEFINED_kwlist__varname */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_unsetenv_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *varname;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__varname, "o:unsetenv", &args))
		goto err;
	return posix_unsetenv_f_impl(args.varname);
err:
	return NULL;
}
FORCELOCAL WUNUSED NONNULL((1))DREF DeeObject *DCALL posix_unsetenv_f_impl(DeeObject *varname)
/*[[[end]]]*/
{
	int error;
	if (DeeObject_AssertTypeExact(varname, &DeeString_Type))
		goto err;
	error = posix_environ_unsetenv((DeeStringObject *)varname);
	if unlikely(error < 0)
		goto err;
	return_bool(error == 0);
err:
	return NULL;
}


/*[[[deemon import("rt.gen.dexutils").gw("clearenv", "", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_clearenv_f_impl(void);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_clearenv_f(size_t argc, DeeObject *const *argv);
#define POSIX_CLEARENV_DEF { "clearenv", (DeeObject *)&posix_clearenv, MODSYM_FREADONLY, DOC("()") },
#define POSIX_CLEARENV_DEF_DOC(doc) { "clearenv", (DeeObject *)&posix_clearenv, MODSYM_FREADONLY, DOC("()\n" doc) },
PRIVATE DEFINE_CMETHOD(posix_clearenv, &posix_clearenv_f, METHOD_FNORMAL);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_clearenv_f(size_t argc, DeeObject *const *argv) {
	_DeeArg_Unpack0(err, argc, argv, "clearenv");
	return posix_clearenv_f_impl();
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_clearenv_f_impl(void)
/*[[[end]]]*/
{
	if unlikely(posix_environ_clearenv())
		goto err;
	return_none;
err:
	return NULL;
}


DECL_END

#endif /* !GUARD_DEX_POSIX_P_ENVIRON_C_INL */
