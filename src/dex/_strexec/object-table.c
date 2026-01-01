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
#ifndef GUARD_DEX_STREXEC_OBJECT_TABLE_C
#define GUARD_DEX_STREXEC_OBJECT_TABLE_C 1
#define DEE_SOURCE

#include "libjit.h"
/**/

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/module.h>
#include <deemon/object.h>
#include <deemon/system-features.h>

DECL_BEGIN

INTERN struct jit_object_entry jit_empty_object_list[1] = {
	{
		/* .oe_namestr = */ NULL,
		/* .oe_namelen = */ 0,
		/* .oe_namehsh = */ 0,
		/* .oe_type    = */ JIT_OBJECT_ENTRY_TYPE_LOCAL,
		/* .oe_value   = */ { NULL }
	}
};

/* Initialize `dst' as a copy of `src' */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
JITObjectTable_Copy(JITObjectTable *__restrict dst,
                    JITObjectTable const *__restrict src) {
	struct jit_object_entry *old_table;
	struct jit_object_entry *new_table;
	size_t i;
	dst->ot_mask = src->ot_mask;
	dst->ot_size = src->ot_size;
	dst->ot_used = src->ot_used;
	old_table    = src->ot_list;
	if (old_table == jit_empty_object_list) {
		dst->ot_list = jit_empty_object_list;
	} else {
		size_t size;
		size      = (dst->ot_mask + 1) * sizeof(struct jit_object_entry);
		new_table = (struct jit_object_entry *)Dee_Calloc(size);
		if unlikely(!new_table)
			goto err;
		dst->ot_list = new_table;
		memcpy(new_table, src->ot_list, size);
		for (i = 0; i <= dst->ot_mask; ++i) {
			if (!ITER_ISOK(new_table[i].oe_namestr))
				continue;
			Dee_XIncref(new_table[i].oe_value);
		}
	}
	return 0;
err:
	return -1;
}

INTERN NONNULL((1)) void DCALL
JITObjectTable_Fini(JITObjectTable *__restrict self) {
	if (self->ot_list != jit_empty_object_list) {
		size_t i;
		for (i = 0; i <= self->ot_mask; ++i) {
			if (!ITER_ISOK(self->ot_list[i].oe_namestr))
				continue;
			jit_object_entry_fini(&self->ot_list[i]);
		}
		Dee_Free(self->ot_list);
	}
	if unlikely(self->ot_star_importc) {
		Dee_Decrefv(self->ot_star_importv, self->ot_star_importc);
		Dee_Free(self->ot_star_importv);
	} else {
		ASSERT(!self->ot_star_importv);
	}
}

INTERN NONNULL((1, 2)) void DCALL
JITObjectTable_Visit(JITObjectTable *__restrict self, Dee_visit_t proc, void *arg) {
	if (self->ot_list != jit_empty_object_list) {
		size_t i;
		for (i = 0; i <= self->ot_mask; ++i) {
			if (!ITER_ISOK(self->ot_list[i].oe_namestr))
				continue;
			jit_object_entry_visit(&self->ot_list[i]);
		}
	}
	Dee_Visitv(self->ot_star_importv, self->ot_star_importc);
}


PRIVATE NONNULL((1)) bool DCALL
JITObjectTable_TryRehash(JITObjectTable *__restrict self,
                         size_t new_mask) {
	size_t i, j, perturb;
	struct jit_object_entry *new_table;
	ASSERT(new_mask >= self->ot_used);
	ASSERT(new_mask != 0);
	new_table = (struct jit_object_entry *)Dee_TryCallocc(new_mask + 1,
	                                                      sizeof(struct jit_object_entry));
	if unlikely(!new_table)
		return false;
	if (self->ot_list != jit_empty_object_list) {
		struct jit_object_entry *old_table;
		old_table = self->ot_list;
		for (i = 0; i <= self->ot_mask; ++i) {
			struct jit_object_entry *old_entry, *new_entry;
			old_entry = &old_table[i];
			if (!ITER_ISOK(old_entry->oe_namestr))
				continue; /* Unused or deleted. */
			perturb = j = old_entry->oe_namehsh & new_mask;
			for (;; JITObjectTable_NEXT(j, perturb)) {
				new_entry = &new_table[j & new_mask];
				if (!new_entry->oe_namestr)
					break;
			}
			/* Copy into the new entry. */
			memcpy(new_entry, old_entry, sizeof(struct jit_object_entry));
		}
		Dee_Free(old_table);
		/* Indicate that all deleted entries have been removed. */
		self->ot_used = self->ot_size;
	} else {
		ASSERT(self->ot_used == 0);
		ASSERT(self->ot_size == 0);
		ASSERT(self->ot_mask == 0);
	}
	self->ot_list = new_table;
	self->ot_mask = new_mask;
	return true;
}


/* Update an object within the given object table, potentially overwriting an
 * existing object, or creating a new entry if no existing object could be found.
 * @param: value: The value to assign to the entry.
 *                When `NULL', the entry is unbound.
 * @return: 1:  Successfully updated an existing entry when `override_existing' was `true'.
 * @return: 1:  An entry already existed for the given name when `override_existing' was `false'.
 * @return: 0:  Successfully created a new entry.
 * @return: -1: An error occurred (failed to increase the hash size of `self') */
INTERN WUNUSED ATTR_INS(2, 3) NONNULL((1)) int DCALL
JITObjectTable_Update(JITObjectTable *__restrict self,
                      /*utf-8*/ char const *namestr,
                      size_t namelen, Dee_hash_t namehsh,
                      /*[0..1]*/ DeeObject *value,
                      bool override_existing) {
	Dee_hash_t i, perturb;
	struct jit_object_entry *result_entry;
again:
	result_entry = NULL;
	perturb = i = namehsh & self->ot_mask;
	for (;; JITObjectTable_NEXT(i, perturb)) {
		struct jit_object_entry *entry;
		entry = &self->ot_list[i & self->ot_mask];
		if (entry->oe_namestr == (char *)ITER_DONE) {
			/* Re-use deleted entries. */
			if (!result_entry)
				result_entry = entry;
			continue;
		}
		if (!entry->oe_namestr) {
			if (!result_entry) {
				/* Check if we must re-hash the table. */
				if (self->ot_size + 1 >= (self->ot_mask * 2) / 3) {
					size_t new_mask;
					new_mask = (self->ot_mask << 1) | 1;
					if (self->ot_used < self->ot_size)
						new_mask = self->ot_mask; /* It's enough if we just rehash to get rid of deleted entries. */
					if (new_mask < 7)
						new_mask = 7;
					if likely(JITObjectTable_TryRehash(self, new_mask))
						goto again;
					if (self->ot_size == self->ot_mask) {
						new_mask = (self->ot_mask << 1) | 1;
						if (self->ot_used < self->ot_size)
							new_mask = self->ot_mask; /* It's enough if we just rehash to get rid of deleted entries. */
						for (;;) {
							if likely(JITObjectTable_TryRehash(self, new_mask))
								goto again;
							if unlikely(!Dee_CollectMemoryc(new_mask + 1, sizeof(struct jit_object_entry)))
								goto err;
						}
					}
				}
				++self->ot_size;
				result_entry = entry;
			}
			break;
		}
		if (!jit_object_entry_eqname(entry, namestr, namelen, namehsh))
			continue;
		/* Existing entry! */
		if (override_existing) {
			DREF DeeObject *oldval;
			oldval = entry->oe_value;
			Dee_XIncref(value);
			entry->oe_type  = JIT_OBJECT_ENTRY_TYPE_LOCAL;
			entry->oe_value = value;
			COMPILER_BARRIER();
			Dee_XDecref(oldval);
		}
		return 1;
	}
	++self->ot_used;
	result_entry->oe_namestr = namestr;
	result_entry->oe_namelen = namelen;
	result_entry->oe_namehsh = namehsh;
	result_entry->oe_type    = JIT_OBJECT_ENTRY_TYPE_LOCAL;
	result_entry->oe_value   = value;
	Dee_XIncref(value);
	return 0;
err:
	return -1;
}

/* Delete an existing entry for an object with the given name
 * @return: true:  Successfully deleted the entry, after potentially unbinding an associated object.
 * @return: false: The object table didn't include an entry matching the given name. */
INTERN WUNUSED ATTR_INS(2, 3) NONNULL((1)) bool DCALL
JITObjectTable_Delete(JITObjectTable *__restrict self,
                      /*utf-8*/ char const *namestr,
                      size_t namelen, Dee_hash_t namehsh) {
	Dee_hash_t i, perturb;
	perturb = i = namehsh & self->ot_mask;
	for (;; JITObjectTable_NEXT(i, perturb)) {
		struct jit_object_entry *entry;
		DREF DeeObject *value;
		entry = &self->ot_list[i & self->ot_mask];
		if (entry->oe_namestr == (char *)ITER_DONE)
			continue;
		if (!entry->oe_namestr)
			break;
		if (!jit_object_entry_eqname(entry, namestr, namelen, namehsh))
			continue;
		/* Found it! */
		value = entry->oe_value;
		entry->oe_namestr = (char *)ITER_DONE;
		ASSERT(self->ot_size);
		ASSERT(self->ot_used);
		--self->ot_used;
		COMPILER_BARRIER();
		if (self->ot_used < self->ot_mask / 3)
			JITObjectTable_TryRehash(self, self->ot_mask >> 1);
		COMPILER_BARRIER();
		Dee_XDecref(value);
		return true;
	}
	return false;
}


/* Lookup a given object within `self'
 * @return: * :   The entry associated with the given name.
 * @return: NULL: Could not find an object matching the specified name. (no error was thrown) */
INTERN WUNUSED ATTR_INS(2, 3) NONNULL((1)) struct jit_object_entry *DCALL
JITObjectTable_Lookup(JITObjectTable *__restrict self,
                      /*utf-8*/ char const *namestr,
                      size_t namelen, Dee_hash_t namehsh) {
	Dee_hash_t i, perturb;
	perturb = i = namehsh & self->ot_mask;
	for (;; JITObjectTable_NEXT(i, perturb)) {
		struct jit_object_entry *entry;
		entry = &self->ot_list[i & self->ot_mask];
		if (entry->oe_namestr == (char *)ITER_DONE)
			continue;
		if (!entry->oe_namestr)
			break;
		if (jit_object_entry_eqname(entry, namestr, namelen, namehsh))
			return entry;
	}
	return NULL;
}

/* Lookup or create an entry for a given name within `self'
 * @return: * :   The entry associated with the given name.
 * @return: NULL: Failed to create a new entry. (an error _WAS_ thrown) */
INTERN WUNUSED ATTR_INS(2, 3) NONNULL((1)) struct jit_object_entry *DCALL
JITObjectTable_Create(JITObjectTable *__restrict self,
                      /*utf-8*/ char const *namestr,
                      size_t namelen, Dee_hash_t namehsh) {
	Dee_hash_t i, perturb;
	struct jit_object_entry *result_entry;
again:
	result_entry = NULL;
	perturb = i = namehsh & self->ot_mask;
	for (;; JITObjectTable_NEXT(i, perturb)) {
		struct jit_object_entry *entry;
		entry = &self->ot_list[i & self->ot_mask];
		if (entry->oe_namestr == (char *)ITER_DONE) {
			/* Re-use deleted entries. */
			if (!result_entry)
				result_entry = entry;
			continue;
		}
		if (!entry->oe_namestr) {
			if (!result_entry) {
				/* Check if we must re-hash the table. */
				if (self->ot_size + 1 >= (self->ot_mask * 2) / 3) {
					size_t new_mask;
					new_mask = (self->ot_mask << 1) | 1;
					if (self->ot_used < self->ot_size)
						new_mask = self->ot_mask; /* It's enough if we just rehash to get rid of deleted entries. */
					if (new_mask < 7)
						new_mask = 7;
					if likely(JITObjectTable_TryRehash(self, new_mask))
						goto again;
					if (self->ot_size == self->ot_mask) {
						new_mask = (self->ot_mask << 1) | 1;
						if (self->ot_used < self->ot_size)
							new_mask = self->ot_mask; /* It's enough if we just rehash to get rid of deleted entries. */
						for (;;) {
							if likely(JITObjectTable_TryRehash(self, new_mask))
								goto again;
							if unlikely(!Dee_CollectMemoryc(new_mask + 1, sizeof(struct jit_object_entry)))
								goto err;
						}
					}
				}
				++self->ot_size;
				result_entry = entry;
			}
			break;
		}
		if (jit_object_entry_eqname(entry, namestr, namelen, namehsh))
			return entry; /* Existing entry! */
	}
	++self->ot_used;
	result_entry->oe_namestr = namestr;
	result_entry->oe_namelen = namelen;
	result_entry->oe_namehsh = namehsh;
	result_entry->oe_type    = JIT_OBJECT_ENTRY_TYPE_LOCAL;
	result_entry->oe_value   = NULL;
	return result_entry;
err:
	return NULL;
}


/* Add a *-import module or object to `self' (if not already present)
 * @return: 0 : Success
 * @return: -1: Success */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
JITObjectTable_AddImportStar(JITObjectTable *__restrict self,
                             DeeObject *module_or_object) {
	size_t i;
	DREF DeeObject **new_star_importv;
	ASSERT_OBJECT(module_or_object);
	for (i = 0; i < self->ot_star_importc; ++i) {
		if (self->ot_star_importv[i] == module_or_object)
			return 0; /* Already present. */
	}
	new_star_importv = (DREF DeeObject **)Dee_Reallocc(self->ot_star_importv,
	                                                   self->ot_star_importc + 1,
	                                                   sizeof(DREF DeeObject *));
	if unlikely(!new_star_importv)
		goto err;
	Dee_Incref(module_or_object);
	new_star_importv[self->ot_star_importc] = module_or_object;
	self->ot_star_importv = new_star_importv;
	++self->ot_star_importc;
	return 0;
err:
	return -1;
}

/* Search the list of *-imports of `self' for the one (if it exists)
 * that has an attribute matching the given `namestr'. If found,
 * return a reference to it, and if not found, return ITER_DONE.
 * NOTE: This function searches `self->ot_star_importv' in reverse
 *       order, meaning that modules from which an import happened
 *       more recently (as per `JITObjectTable_AddImportStar()')
 *       will be hit first. Also note that once a hit is found, the
 *       search ends (this behavior differs from the core compiler,
 *       where multiple *-imports of the same symbol-name result
 *       in a compiler error stating that the symbol is ambiguous).
 *       However, this difference is acceptable, since it only
 *       affects code that would otherwise be malformed.
 * @param: p_mod_symbol: when non-NULL, store the module-symbol (in
 *                       case the *-import was made for a module)
 * @return: * :        The module/object defining `namestr'
 * @return: ITER_DONE: The *-imported module defines `namestr'
 * @return: NULL:      An error was thrown. */
INTERN WUNUSED ATTR_INS(2, 3) NONNULL((1)) DREF DeeObject *DCALL
JITObjectTable_FindImportStar(JITObjectTable *__restrict self,
                              /*utf-8*/ char const *namestr,
                              size_t namelen, Dee_hash_t namehsh,
                              struct Dee_module_symbol **p_mod_symbol) {
	size_t i = self->ot_star_importc;
	while (i) {
		DeeObject *module_or_object;
		--i;
		module_or_object = self->ot_star_importv[i];

		/* Check if this star-import exposes the required attribute. */
		if (DeeModule_Check(module_or_object)) {
			struct Dee_module_symbol *symbol;
			symbol = DeeModule_GetSymbolStringLenHash((DeeModuleObject *)module_or_object,
			                                          namestr, namelen, namehsh);
			if (symbol) {
				/* Found it! */
				if (p_mod_symbol)
					*p_mod_symbol = symbol;
				return_reference_(module_or_object);
			}
		} else {
			/* Check if object has the required attribute (generic code-path) */
			int hasattr;
			hasattr = DeeObject_HasAttrStringLenHash(module_or_object,
			                                         namestr, namelen, namehsh);
			if (hasattr != 0) {
				if unlikely(Dee_HAS_ISERR(hasattr))
					goto err; /* Error */
				if (p_mod_symbol)
					*p_mod_symbol = NULL;
				return_reference_(module_or_object);
			}
		}
	}
	return ITER_DONE;
err:
	return NULL;
}

DECL_END

#endif /* !GUARD_DEX_STREXEC_OBJECT_TABLE_C */
