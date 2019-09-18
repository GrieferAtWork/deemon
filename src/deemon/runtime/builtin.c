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
#ifndef GUARD_DEEMON_RUNTIME_BUILTIN_C
#define GUARD_DEEMON_RUNTIME_BUILTIN_C 1

#include <deemon/api.h>
#include <deemon/code.h>
#include <deemon/module.h>
#include <deemon/object.h>
#include <deemon/string.h>

#ifndef CONFIG_NO_THREADS
#include <hybrid/sched/yield.h>
#endif /* !CONFIG_NO_THREADS */

#include "builtin.h"
#include "strings.h"

DECL_BEGIN

#define BUILTIN(name, object, flags) \
	PRIVATE DEFINE_STRING(str_##name,#name);

#define BUILTIN_REUSE(name, object, flags) /* nothing */
#include "builtins.def"

enum{
#define BUILTIN(name, object, flags) _id_##name,
#include "builtins.def"
	num_builtins_sym
};

PRIVATE DREF DeeObject *builtin_object_vector[num_builtins_obj] = {
#define BUILTIN(name, object, flags) (DeeObject *)object,
#define BUILTIN_ALIAS(name, alt, flags) /* nothing */
#include "builtins.def"
};

enum {
	num_builtins_spc  = num_builtins_sym + (num_builtins_sym/3),
	builtins_hashmask = (num_builtins_spc <= 0x0001 ? 0x0000 :
	                     num_builtins_spc <= 0x0002 ? 0x0001 :
	                     num_builtins_spc <= 0x0004 ? 0x0003 :
	                     num_builtins_spc <= 0x0008 ? 0x0007 :
	                     num_builtins_spc <= 0x0010 ? 0x000f :
	                     num_builtins_spc <= 0x0020 ? 0x001f :
	                     num_builtins_spc <= 0x0040 ? 0x003f :
	                     num_builtins_spc <= 0x0080 ? 0x007f :
	                     num_builtins_spc <= 0x0100 ? 0x00ff :
	                     num_builtins_spc <= 0x0200 ? 0x01ff :
	                     num_builtins_spc <= 0x0400 ? 0x03ff :
	                     num_builtins_spc <= 0x0800 ? 0x07ff :
	                     num_builtins_spc <= 0x1000 ? 0x0fff :
	                     num_builtins_spc <= 0x2000 ? 0x1fff :
	                     num_builtins_spc <= 0x4000 ? 0x3fff :
	                     num_builtins_spc <= 0x8000 ? 0x7fff :
	                     0xffff)
};

PRIVATE struct module_symbol deemon_symbols[builtins_hashmask + 1];
PRIVATE struct module_symbol deemon_builtins[num_builtins_sym] = {
#define BUILTIN(name, object, flags) { DeeString_STR(&str_##name), NULL, 0, flags | MODSYM_FNAMEOBJ, { id_##name } },
#ifndef CONFIG_NO_DOC
#define BUILTIN_DOC(name, object, flags, doc) { DeeString_STR(&str_##name), doc, 0, flags | MODSYM_FNAMEOBJ, { id_##name } },
#define BUILTIN_DOC_REUSE(name, object, flags, doc) BUILTIN_DOC(name, object, flags, doc)
#endif /* !CONFIG_NO_DOC */
#define BUILTIN_ALIAS(name, alt, flags) { DeeString_STR(&str_##name), NULL, 0, flags | MODSYM_FALIAS | MODSYM_FNAMEOBJ, { id_##alt } },
#include "builtins.def"
};

INTDEF struct module_symbol empty_module_buckets[];
INTERN struct static_module_struct deemon_module_head = {
	{
		/* ... */
		NULL,
		NULL
	}, {
		OBJECT_HEAD_INIT(&DeeModule_Type),
		/* .mo_name      = */ (DREF DeeStringObject *)&str_deemon,
		/* .mo_pself     = */ NULL,
		/* .mo_next      = */ NULL,
		/* .mo_path      = */ NULL,
#ifdef CONFIG_HOST_WINDOWS
		/* .mo_pathhash  = */ 0,
#endif /* CONFIG_HOST_WINDOWS */
		/* .mo_globpself = */ NULL,
		/* .mo_globnext  = */ NULL,
		/* .mo_importc   = */ 0,
		/* .mo_globalc   = */num_builtins_obj,
		/* .mo_flags     = */MODULE_FLOADING | MODULE_FDIDLOAD | MODULE_FINITIALIZING | MODULE_FDIDINIT,
		/* .mo_bucketm   = */builtins_hashmask,
		/* .mo_bucketv   = */deemon_symbols,
		/* .mo_importv   = */ NULL,
		/* .mo_globalv   = */builtin_object_vector,
		/* .mo_root      = */ &empty_code,
#ifndef CONFIG_NO_THREADS
		/* .mo_lock      = */ RWLOCK_INIT,
		/* .mo_loader    = */ NULL,
#endif /* !CONFIG_NO_THREADS */
#ifndef CONFIG_NO_DEC
		/* .mo_ctime     = */ 0,
#endif /* !CONFIG_NO_DEC */
		WEAKREF_SUPPORT_INIT
	}
};


PRIVATE ATTR_NOINLINE void DCALL init_builtins(void) {
	struct module_symbol *iter;
	for (iter = deemon_builtins; iter < COMPILER_ENDOF(deemon_builtins); ++iter) {
		dhash_t hash, i, perturb;
		DeeStringObject *name;
		name = COMPILER_CONTAINER_OF(MODULE_SYMBOL_GETNAMESTR(iter), DeeStringObject, s_str);
		hash = Dee_HashPtr(name->s_str, name->s_len * sizeof(char));
		iter->ss_hash = name->s_hash = hash;
		perturb = i = hash & builtins_hashmask;
		for (;; i = MODULE_HASHNX(i, perturb), MODULE_HASHPT(perturb)) {
			struct module_symbol *item = &deemon_symbols[i & builtins_hashmask];
			if (item->ss_name)
				continue;
			*item = *iter;
			break;
		}
	}
}


#ifdef CONFIG_NO_THREADS
#define INIT_PENDING 0
#define INIT_COMPLET 1
#else /* CONFIG_NO_THREADS */
#define INIT_PENDING 0
#define INIT_PROGRES 1
#define INIT_COMPLET 2
#endif /* !CONFIG_NO_THREADS */
PRIVATE int init_state = INIT_PENDING;

PUBLIC ATTR_RETNONNULL DeeModuleObject *
DCALL DeeModule_GetDeemon(void) {
	/* Lazily calculate hashes of exported objects upon first access. */
	if 		unlikely(init_state != INIT_COMPLET) {
#ifdef CONFIG_NO_THREADS
		init_builtins();
		init_state = INIT_COMPLET;
#else /* CONFIG_NO_THREADS */
		COMPILER_READ_BARRIER();
		if (ATOMIC_CMPXCH(init_state, INIT_PENDING, INIT_PROGRES)) {
			init_builtins();
			ATOMIC_WRITE(init_state, INIT_COMPLET);
		} else {
			while (ATOMIC_READ(init_state) != INIT_COMPLET)
				SCHED_YIELD();
		}
#endif /* !CONFIG_NO_THREADS */
	}
	return &deemon_module;
}


DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_BUILTIN_C */
