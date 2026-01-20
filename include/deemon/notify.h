/* Copyright (c) 2018-2026 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2026 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_NOTIFY_H
#define GUARD_DEEMON_NOTIFY_H 1

#include "api.h"

#include "object.h"

#include <stdbool.h> /* bool */
#include <stdint.h>  /* uint16_t */

#ifndef CONFIG_NO_NOTIFICATIONS
#include "int.h"
#include "string.h" /* DeeString_AsUtf8 */
#endif /* !CONFIG_NO_NOTIFICATIONS */

DECL_BEGIN


#ifndef CONFIG_NO_NOTIFICATIONS

/* Notification callback system for changes in environment variables.
 * Using this system, callbacks can be registered to-be executed after
 * specific environment variables have changed their value.
 *
 * The intended use for this is for libraries to invalidate caches that
 * might be associated with specific variables, such as the `$PATHEXT'
 * cache used by Window's implementation of the `ipc' module to quickly
 * map executable names to their full image paths.
 *
 * However, in order to ensure that this will remain the central hub
 * for custom notification callbacks, in addition to its name, a
 * notification class must be specified (which is not restricted
 * to those listed below). */
#define Dee_NOTIFICATION_CLASS_FNOCASE 0x8000 /* FLAG: Callback names are case-insensitive. */
#define Dee_NOTIFICATION_CLASS_FIDMASK 0x7fff /* MASK: The actual notification class ID. */
#ifdef CONFIG_HOST_WINDOWS
#define Dee_NOTIFICATION_CLASS_ENVIRON 0x8000 /* Environment variable. (The callback name is the variable name) */
#else /* CONFIG_HOST_WINDOWS */
#define Dee_NOTIFICATION_CLASS_ENVIRON 0x0000 /* Environment variable. (The callback name is the variable name) */
#endif /* !CONFIG_HOST_WINDOWS */
#define Dee_NOTIFICATION_CLASS_PWD     0x0001 /* Process working directory. (`name' is ignored) */


/* Returns the value of `(environ from posix).get(name, none)',
 * or `ITER_DONE' if that expression evaluated to `none'
 * @return: * :        The value of the environment variable `name'
 * @return: NULL:      An error occurred.
 * @return: ITER_DONE: No value assigned to the environment variable `name' */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL Dee_GetEnv(DeeObject *__restrict name);


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
DFUNDEF WUNUSED NONNULL((2, 3)) int (DCALL DeeNotify_StartListen)(uint16_t cls, DeeObject *__restrict name, Dee_notify_t callback, DeeObject *arg);
DFUNDEF NONNULL((2, 3)) int (DCALL DeeNotify_EndListen)(uint16_t cls, DeeObject *__restrict name, Dee_notify_t callback, DeeObject *arg);

/* Notify start/end functions for classes exclusively notified using `DeeNotify_BroadcastClass()'. */
#define DeeNotify_StartListenClass(cls, callback, arg) \
	DeeNotify_StartListen(cls, Dee_EmptyString, callback, arg)
#define DeeNotify_EndListenClass(cls, callback, arg) \
	DeeNotify_EndListen(cls, Dee_EmptyString, callback, arg)

/* Broadcast a change notification for the given class `cls' and `name'
 * NOTE: The caller is responsible for passing a string for `name'
 * @return:  0 : Success.
 * @return: -1 : Callback invocation was stopped after a callback indicated an error. */
DFUNDEF WUNUSED NONNULL((2)) int (DCALL DeeNotify_Broadcast)(uint16_t cls, DeeObject *__restrict name);
DFUNDEF WUNUSED NONNULL((2)) int (DCALL DeeNotify_BroadcastString)(uint16_t cls, char const *__restrict name);

/* Broadcast a change to all listeners of the given `cls' */
DFUNDEF WUNUSED int (DCALL DeeNotify_BroadcastClass)(uint16_t cls);

#ifndef __INTELLISENSE__
#ifndef __NO_builtin_expect
#define DeeNotify_Broadcast(cls, name)       __builtin_expect(DeeNotify_Broadcast(cls, name), 0)
#define DeeNotify_BroadcastString(cls, name) __builtin_expect(DeeNotify_BroadcastString(cls, name), 0)
#define DeeNotify_BroadcastClass(cls)        __builtin_expect(DeeNotify_BroadcastClass(cls), 0)
#endif /* !__NO_builtin_expect */
#endif /* !__INTELLISENSE__ */


#ifdef CONFIG_BUILDING_DEEMON
/* Delete all registered notification callbacks.
 * @return: true:  At least one registration was deleted.
 * @return: false: There was nothing to delete. */
INTDEF bool DCALL DeeNotify_Shutdown(void);
#endif /* CONFIG_BUILDING_DEEMON */

#endif /* !CONFIG_NO_NOTIFICATIONS */

DECL_END

#endif /* !GUARD_DEEMON_NOTIFY_H */
