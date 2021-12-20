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
#ifndef GUARD_DEEMON_DEX_H
#define GUARD_DEEMON_DEX_H 1

#include "api.h"

#include "object.h"

#ifndef CONFIG_NO_DEX
#include "module.h"
#ifndef CONFIG_NO_NOTIFICATIONS
#include "notify.h"
#endif /* !CONFIG_NO_NOTIFICATIONS */
#include <stdbool.h>
#include <stddef.h>

DECL_BEGIN

#ifdef DEE_SOURCE
#define Dee_dex_object       dex_object
#define Dee_dex_symbol       dex_symbol
#define Dee_dex              dex
#ifndef CONFIG_NO_NOTIFICATIONS
#define Dee_string_object    string_object
#define Dee_dex_notification dex_notification
#endif /* !CONFIG_NO_NOTIFICATIONS */
#endif /* DEE_SOURCE */

typedef struct Dee_dex_object DeeDexObject;

struct Dee_dex_symbol {
	char const           *ds_name;  /* [1..1][SENTINEL(NULL)] Name of this symbol. */
	DeeObject            *ds_obj;   /* [0..1] The initial value of this symbol. */
	uintptr_t             ds_flags; /* Set of `MODSYM_F*'. */
	/*utf-8*/ char const *ds_doc;   /* [0..1] An optional documentation string. */
};

#ifndef CONFIG_NO_NOTIFICATIONS
struct Dee_string_object;
struct Dee_dex_notification {
#define Dee_DEX_NOTIFICATION_FNORMAL 0x0000 /* Normal notification flags. */
#if __SIZEOF_POINTER__ >= 8
	uint32_t              dn_class;    /* [valid_if(dn_name)] Notification class (One of `NOTIFICATION_CLASS_*') */
	uint32_t              dn_flag;     /* [valid_if(dn_name)] Notification flags (Set of `DEX_NOTIFICATION_F*') */
#elif __SIZEOF_POINTER__ >= 4
	uint16_t              dn_class;    /* [valid_if(dn_name)] Notification class (One of `NOTIFICATION_CLASS_*') */
	uint16_t              dn_flag;     /* [valid_if(dn_name)] Notification flags (Set of `DEX_NOTIFICATION_F*') */
#else
#error FIXME
#endif
	struct Dee_string_object
	                     *dn_name;     /* [0..1] Notification name (`NULL' indicates sentinal). */
	Dee_notify_t          dn_callback; /* [1..1][valid_if(dn_name)] Notification callback. */
	DeeObject            *dn_arg;      /* [0..1][valid_if(dn_name)] Notification argument. */
};
#endif /* !CONFIG_NO_NOTIFICATIONS */

struct Dee_dex {
	/* The extension descriptor structure that must be
	 * exported by the extension module under the name `DEX'. */
	struct Dee_dex_symbol *d_symbols; /* [0..1] The vector of exported symbols.
	                                   * NOTE: Indices in this vector are re-used as global variable numbers. */
	/* Optional initializer/finalizer callbacks.
	 * When non-NULL, `d_init()' is invoked after globals have been.
	 * Extension modules are only unloaded before deemon itself terminates.
	 * NOTE: When executed, the `MODULE_FDIDINIT' hasn't been set yet,
	 *       but will be as soon as the function returns a value of ZERO(0).
	 *       Any other return value can be used to indicate an error having
	 *       been thrown, which in turn will cause the caller to propagate
	 *       said error. */
	WUNUSED NONNULL((1))
	int        (DCALL *d_init)(DeeDexObject *__restrict self);
	/* WARNING: `d_fini()' must not attempt to add more references to `self'.
	 *           When an extension module is supposed to get unloaded, it _has_
	 *           to be unloaded and there is no way around that! */
	NONNULL((1))
	void       (DCALL *d_fini)(DeeDexObject *__restrict self);
	union {
	    char const *const *d_import_names; /* [1..1|SENTINEL([0..0])][0..1]
	                                        * NULL-terminated vector of other imported modules. */
	    DeeObject        **d_imports;      /* [1..1][0..1] A shadow copy of the dex's `m_importv' vector.
	                                        * NOTE: Not modified when `d_import_names' was empty, or NULL. */
	}
#ifndef __COMPILER_HAVE_TRANSPARENT_UNION
	_dee_aunion
#define d_import_names _dee_aunion.d_import_names
#define d_imports      _dee_aunion.d_imports
#endif /* !__COMPILER_HAVE_TRANSPARENT_UNION */
	;
	/* Called during the GC-cleanup phase near the end of deemon's execution cycle.
	 * This function should be implemented to clear global caches or object hooks.
	 * @return: true:  Something was cleared.
	 * @return: false: Nothing was cleared. (Same as not implementing this callback) */
	NONNULL((1))
	bool       (DCALL *d_clear)(DeeDexObject *__restrict self);
#ifndef CONFIG_NO_NOTIFICATIONS
	struct Dee_dex_notification *d_notify;     /* [0..1] Dex notification hooks. */
#endif /* !CONFIG_NO_NOTIFICATIONS */
};

struct Dee_dex_object {
	DeeModuleObject    d_module;       /* The underlying module. */
	struct Dee_dex    *d_dex;          /* [1..1][const_if(MODULE_FDIDLOAD)] The dex definition table exported by this extension.
	                                    * NOTE: This pointer is apart of the extension's static address space. */
	void              *d_handle;       /* [?..?][const_if(MODULE_FDIDLOAD)] System-specific library handle. */
	DeeDexObject     **d_pself;        /* [1..1][== self][0..1][lock(INTERN(dex_lock))] Dex self-pointer. */
	DREF DeeDexObject *d_next;         /* [0..1][lock(INTERN(dex_lock))] Extension initialized before this one.
	                                    * During finalization, extensions are unloaded in reverse order. */
#ifdef __COMPILER_HAVE_TRANSPARENT_UNION
	char const *const *d_import_names; /* [1..1|SENTINEL([0..0])][0..1] NULL-terminated vector of other imported modules. */
#else /* __COMPILER_HAVE_TRANSPARENT_UNION */
	union {
#undef d_import_names
		char const *const *d_import_names; /* [1..1|SENTINEL([0..0])][0..1] NULL-terminated vector of other imported modules. */
#define d_import_names _dee_aunion.d_import_names
	} _dee_aunion;
#endif /* !__COMPILER_HAVE_TRANSPARENT_UNION */
};

DDATDEF DeeTypeObject DeeDex_Type;
#define DeeDex_Check(ob)      DeeObject_InstanceOf(ob, &DeeDex_Type)
#define DeeDex_CheckExact(ob) DeeObject_InstanceOfExact(ob, &DeeDex_Type)

#ifdef CONFIG_BUILDING_DEX
/* Implemented by the extension:
 * >> PUBLIC struct Dee_dex DEX = { ... }; */
EXPDEF struct Dee_dex DEX;
#endif /* CONFIG_BUILDING_DEX */

#ifdef CONFIG_BUILDING_DEEMON
/* Try to load an extension file.
 * NOTE: This isn't where the dex gets initialized!
 * @return:  0: The extension was successfully loaded.
 * @return: -1: An error occurred. */
INTDEF WUNUSED NONNULL((1)) int DCALL
dex_load_handle(DeeDexObject *__restrict self,
                /*inherited(always)*/ void *handle,
                DeeObject *__restrict input_file);

/* Initialize the given dex module. */
INTDEF WUNUSED NONNULL((1)) int DCALL dex_initialize(DeeDexObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeDex_New(DeeObject *__restrict name);

/* Clear global data caches of all loaded dex modules. */
INTDEF bool DCALL DeeDex_Cleanup(void);

/* Unload all loaded dex modules. */
INTDEF void DCALL DeeDex_Finalize(void);
#endif /* CONFIG_BUILDING_DEEMON */


DECL_END
#endif /* !CONFIG_NO_DEX */

#endif /* !GUARD_DEEMON_DEX_H */
