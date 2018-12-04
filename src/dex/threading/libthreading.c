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
#ifndef GUARD_DEX_THREADING_LIBTHREADING_C
#define GUARD_DEX_THREADING_LIBTHREADING_C 1
#define CONFIG_BUILDING_LIBTHREADING 1
#define _KOS_SOURCE 1

#include "libthreading.h"

#include <deemon/api.h>
#include <deemon/dex.h>
#include <deemon/thread.h>

DECL_BEGIN


PRIVATE struct dex_symbol symbols[] = {
    { "Semaphore", (DeeObject *)&DeeSemaphore_Type },
    { "Mutex", (DeeObject *)&DeeMutex_Type },
    //{ "RwLock", (DeeObject *)&DeeRWLock_Type }, /* TODO */
    { "Tls", (DeeObject *)&DeeTls_Type },
    { NULL }
};



#ifndef CONFIG_NO_THREADS
PRIVATE struct tls_callback_hooks orig_hooks;
PRIVATE struct tls_callback_hooks thrd_hooks = {
    /* .tc_fini  = */(void(DCALL *)(void *__restrict))&thread_tls_fini,
    /* .tc_visit = */(void(DCALL *)(void *__restrict,dvisit_t,void*))&thread_tls_visit
};

PRIVATE int DCALL
libthreading_init(DeeDexObject *__restrict UNUSED(self)) {
 /* Install our custom TLS callback hooks. */
 memcpy(&orig_hooks,&_DeeThread_TlsCallbacks,
        sizeof(struct tls_callback_hooks));
 memcpy(&_DeeThread_TlsCallbacks,&thrd_hooks,
        sizeof(struct tls_callback_hooks));
 return 0;
}

PRIVATE void DCALL
libthreading_fini(DeeDexObject *__restrict UNUSED(self)) {
 /* Restore the original callback hooks. */
 memcpy(&_DeeThread_TlsCallbacks,&orig_hooks,
        sizeof(struct tls_callback_hooks));
}

#endif /* !CONFIG_NO_THREADS */


PUBLIC struct dex DEX = {
    /* .d_symbols      = */symbols,
#ifndef CONFIG_NO_THREADS
    /* .d_init         = */&libthreading_init,
    /* .d_fini         = */&libthreading_fini
#endif /* !CONFIG_NO_THREADS */
};

DECL_END


#endif /* !GUARD_DEX_THREADING_LIBTHREADING_C */
