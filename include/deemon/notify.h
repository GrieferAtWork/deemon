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
#ifndef GUARD_DEEMON_NOTIFY_H
#define GUARD_DEEMON_NOTIFY_H 1

#include "api.h"

#include "object.h"

#ifndef CONFIG_NO_NOTIFICATIONS
#include "int.h"
#include "string.h"
#endif /* !CONFIG_NO_NOTIFICATIONS */

DECL_BEGIN


#ifndef CONFIG_NO_NOTIFICATIONS

/* Notification callback system for changes in environment variables.
 * Using this system, callbacks can be registered to-be executed after
 * specific environment variables have changed their value.
 * The intended use for this is for libraries to invalidate caches that
 * might be associated with specific variables, such as the $PATHEXT
 * cache used by Window's implementation of the `process' module to
 * quickly map executable names to their full image paths.
 * However in order to ensure that this will remain the central hub
 * for custom notification callbacks, in addition to its name, a
 * notification class must be specified (which is not restricted
 * to those listed below). */
#define Dee_NOTIFICATION_CLASS_FNOCASE     0x8000 /* FLAG: Callback names are case-insensitive. */
#define Dee_NOTIFICATION_CLASS_FIDMASK     0x7fff /* MASK: The actual notification class ID. */
#ifdef CONFIG_HOST_WINDOWS
#define Dee_NOTIFICATION_CLASS_ENVIRON     0x8000 /* Environment variable. (The callback name is the variable name) */
#else /* CONFIG_HOST_WINDOWS */
#define Dee_NOTIFICATION_CLASS_ENVIRON     0x0000 /* Environment variable. (The callback name is the variable name) */
#endif /* !CONFIG_HOST_WINDOWS */


/* Returns the value of `(environ from fs).get(name, none)',
 * or `ITER_DONE' if that expression evaluated to `none'
 * @return: * :        The value of the environment variable `name'
 * @return: NULL:      An error occurred.
 * @return: ITER_DONE: No value assigned to the environment variable `name' */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL Dee_GetEnv(DeeObject *__restrict name);


#ifndef CONFIG_NO_DEX
#define DEX_NOTIFICATION_ENVIRON(name)         \
	{ Dee_NOTIFICATION_CLASS_ENVIRON,          \
	  Dee_DEX_NOTIFICATION_FNORMAL,            \
	  (struct string_object *)&name##_envname, \
	  &name##_notify, NULL }
#define DEX_NOTIFICATION_END \
	{ 0, 0, NULL, NULL, NULL }
#endif /* !CONFIG_NO_DEX */


#define DECLARE_NOTIFY_ENVIRON_INTEGER(T, name)             \
	INTDEF T name##_value;                                  \
	INTDEF bool name##_loaded;                              \
	INTDEF DeeObject name##_envname;                        \
	INTDEF int DCALL name##_notify(DeeObject *UNUSED(arg)); \
	INTDEF int DCALL name##_get(T *__restrict result);



/* >> DEFINE_NOTIFY_ENVIRON_INTEGER(int, my_envint, 7, "MY_ENVINT");
 * >> DREF DeeObject *DCALL
 * >> get_my_envint(size_t argc, DeeObject *const *argv) {
 * >>     int result;
 * >>     if (DeeArg_Unpack(argc, argv, ":get_my_envint"))
 * >>         goto err;
 * >>     if (my_envint_get(&result))
 * >>         goto err;
 * >>     return DeeInt_NewInt(result);
 * >> err:
 * >>     return NULL;
 * >> }
 */
#define DEFINE_NOTIFY_ENVIRON_INTEGER(T, name, default, environ_name)              \
	INTERN T name##_value     = (default);                                         \
	INTERN bool name##_loaded = false;                                             \
	INTERN DEFINE_STRING(name##_envname, environ_name);                            \
	INTERN int DCALL name##_notify(DeeObject *UNUSED(arg)) {                       \
		name##_loaded = false;                                                     \
		return 0;                                                                  \
	}                                                                              \
	INTERN int DCALL name##_get(T *__restrict result) {                            \
		DREF DeeObject *value;                                                     \
		if (!name##_loaded) {                                                      \
			value = Dee_GetEnv((DeeObject *)&name##_envname);                      \
			if (value == ITER_DONE) {                                              \
				name##_value = (default);                                          \
			} else {                                                               \
				char *strval;                                                      \
				if unlikely(!value)                                                \
					goto err;                                                      \
				if unlikely(DeeObject_AssertTypeExact(value, &DeeString_Type))     \
					goto err_value;                                                \
				strval = DeeString_AsUtf8(value);                                  \
				if unlikely(!strval)                                               \
					goto err_value;                                                \
				if (!*strval) {                                                    \
					name##_value = (default); /* Empty string --> default */       \
				} else {                                                           \
					if unlikely(Dee_TAtoi(T, strval, WSTR_LENGTH(strval),          \
					                      DEEINT_STRING(0, DEEINT_STRING_FNORMAL), \
					                      &name##_value))                          \
						goto err_value;                                            \
				}                                                                  \
				Dee_Decref(value);                                                 \
			}                                                                      \
			name##_loaded = true;                                                  \
		}                                                                          \
		*result = name##_value;                                                    \
		return 0;                                                                  \
	err_value:                                                                     \
		Dee_Decref(value);                                                         \
	err:                                                                           \
		return -1;                                                                 \
	}



/* @return:  0: Successfully handled the notification.
 * @return: -1: An error occurred and should be propagated. */
typedef int (DCALL *Dee_notify_t)(DeeObject *arg);

/* Add/remove a notification listener for a given class and name.
 * @param:  arg: When non-NULL, an object to which the internal
 *               notification registration will keep a reference.
 *               Additionally, this object is passed to `callback'
 *               whenever it is invoked.
 * @param:  cls: One of `NOTIFICATION_CLASS_*'
 * @param: name: The name of the notification to listen for. (Must be a string; e.g.: Name of an environment variable)
 * @return:   0: Successfully registered/removed the given `callback' and `arg' for `cls' and `name'.
 * @return:   1: The given `callback' has already / hasn't been registered for `cls'
 *               and `name' with the same `arg' and was not registered again / removed.
 * @return:  -1: An error occurred (Never returned by `DeeNotify_EndListen').
 * WARNING: Notifications may be invoked more than once if added from a notification callback. */
DFUNDEF WUNUSED NONNULL((2, 3)) int (DCALL DeeNotify_BeginListen)(uint16_t cls, DeeObject *__restrict name, Dee_notify_t callback, DeeObject *arg);
DFUNDEF NONNULL((2, 3)) int (DCALL DeeNotify_EndListen)(uint16_t cls, DeeObject *__restrict name, Dee_notify_t callback, DeeObject *arg);

/* Broadcast a change notification for the given class `cls' and `name'
 * NOTE: The caller is responsible for passing a string for `name'
 * @return:  * : The number of callbacks that were executed.
 * @return: -1 : Callback invocation was stopped after a callback indicated an error. */
DFUNDEF WUNUSED NONNULL((2)) int DCALL DeeNotify_Broadcast(uint16_t cls, DeeObject *__restrict name);
DFUNDEF WUNUSED NONNULL((2)) int DCALL DeeNotify_BroadcastString(uint16_t cls, char const *__restrict name);

#ifdef CONFIG_BUILDING_DEEMON
/* Delete all registered notification callbacks.
 * @return: true:  At least one registration was deleted.
 * @return: false: There was nothing to delete. */
INTDEF bool DCALL DeeNotify_Shutdown(void);
#endif /* CONFIG_BUILDING_DEEMON */

#endif /* !CONFIG_NO_NOTIFICATIONS */

DECL_END

#endif /* !GUARD_DEEMON_NOTIFY_H */
