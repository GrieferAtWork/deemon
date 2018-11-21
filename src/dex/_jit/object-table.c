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
#ifndef GUARD_DEX_JIT_OBJECT_TABLE_C
#define GUARD_DEX_JIT_OBJECT_TABLE_C 1

#include "libjit.h"
#include <deemon/object.h>
#include <deemon/module.h>
#include <deemon/string.h>
#include <deemon/stringutils.h>

DECL_BEGIN

INTERN struct jit_object_entry jit_empty_object_list[1] = {
    {
        /* .oe_nameobj = */NULL,
        /* .oe_namestr = */NULL,
        /* .oe_namelen = */0,
        /* .oe_namehsh = */0,
        /* .oe_value   = */NULL
    }
};

/* Initialize `dst' as a copy of `src' */
INTERN int DCALL
JITObjectTable_Copy(JITObjectTable *__restrict dst,
                    JITObjectTable const *__restrict src) {
 struct jit_object_entry *old_table;
 struct jit_object_entry *new_table;
 size_t i;
 dst->ot_mask = src->ot_mask;
 dst->ot_size = src->ot_size;
 dst->ot_used = src->ot_used;
 old_table = src->ot_list;
 if (old_table == jit_empty_object_list) {
  dst->ot_list = jit_empty_object_list;
 } else {
  size_t size = (dst->ot_mask + 1) * sizeof(struct jit_object_entry);
  new_table = (struct jit_object_entry *)Dee_Calloc(size);
  if unlikely(!new_table) return -1;
  dst->ot_list = new_table;
  memcpy(new_table,src->ot_list,size);
  for (i = 0; i <= dst->ot_mask; ++i) {
   if (!ITER_ISOK(new_table[i].oe_nameobj))
        continue;
   Dee_XIncref(new_table[i].oe_value);
   Dee_Incref(new_table[i].oe_nameobj);
  }
 }
 return 0;
}

/* Insert all elements from `src' into `dst'
 * Existing entires will not be overwritten. */
INTERN int DCALL
JITObjectTable_UpdateTable(JITObjectTable *__restrict dst,
                           JITObjectTable const *__restrict src) {
 size_t i;
 struct jit_object_entry *old_table;
 old_table = src->ot_list;
 for (i = 0; i <= src->ot_mask; ++i) {
  if (!ITER_ISOK(old_table[i].oe_nameobj))
       continue;
  if (JITObjectTable_Update(dst,
                            old_table[i].oe_namestr,
                            old_table[i].oe_namelen,
                            old_table[i].oe_namehsh,
                            old_table[i].oe_nameobj,
                            old_table[i].oe_value,
                            false))
      goto err;
 }
 return 0;
err:
 return -1;
}



INTERN void DCALL
JITObjectTable_Fini(JITObjectTable *__restrict self) {
 size_t i;
 if (self->ot_list == jit_empty_object_list)
     return;
 for (i = 0; i <= self->ot_mask; ++i) {
  DeeObject *nameobj;
  nameobj = self->ot_list[i].oe_nameobj;
  if (!ITER_ISOK(nameobj)) continue;
  Dee_XDecref(self->ot_list[i].oe_value);
  Dee_Decref_unlikely(nameobj);
 }
 Dee_Free(self->ot_list);
}

INTERN void DCALL
JITObjectTable_Fini_iterdone(JITObjectTable *__restrict self) {
 size_t i;
 if (self->ot_list == jit_empty_object_list)
     return;
 for (i = 0; i <= self->ot_mask; ++i) {
  DeeObject *nameobj;
  nameobj = self->ot_list[i].oe_nameobj;
  if (!ITER_ISOK(nameobj)) continue;
  if (ITER_ISOK(self->ot_list[i].oe_value))
      Dee_Decref(self->ot_list[i].oe_value);
  Dee_Decref_unlikely(nameobj);
 }
 Dee_Free(self->ot_list);
}

INTERN bool DCALL
JITObjectTable_TryRehash(JITObjectTable *__restrict self,
                         size_t new_mask) {
 size_t i,j,perturb;
 struct jit_object_entry *new_table;
 ASSERT(new_mask >= self->ot_used);
 ASSERT(new_mask != 0);
 new_table = (struct jit_object_entry *)Dee_TryCalloc((new_mask + 1) *
                                                       sizeof(struct jit_object_entry));
 if unlikely(!new_table) return false;
 if (self->ot_list != jit_empty_object_list) {
  struct jit_object_entry *old_table;
  old_table = self->ot_list;
  for (i = 0; i <= self->ot_mask; ++i) {
   struct jit_object_entry *old_entry,*new_entry;
   old_entry = &old_table[i];
   if (!ITER_ISOK(old_entry->oe_nameobj))
        continue; /* Unused or deleted. */
   perturb = j = old_entry->oe_namehsh & new_mask;
   for (;; JITObjectTable_NEXT(j,perturb)) {
    new_entry = &new_table[j & new_mask];
    if (!new_entry->oe_nameobj)
         break;
   }
   /* Copy into the new entry. */
   memcpy(new_entry,old_entry,sizeof(struct jit_object_entry));
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
INTERN int DCALL
JITObjectTable_Update(JITObjectTable *__restrict self,
                      /*utf-8*/unsigned char *namestr,
                      size_t namelen, dhash_t namehsh,
                      DeeObject *__restrict nameobj,
                      DeeObject *value,
                      bool override_existing) {
 dhash_t i,perturb;
 struct jit_object_entry *result_entry;
again:
 result_entry = NULL;
 perturb = i = namehsh & self->ot_mask;
 for (;; JITObjectTable_NEXT(i,perturb)) {
  struct jit_object_entry *entry;
  entry = &self->ot_list[i & self->ot_mask];
  if (entry->oe_nameobj == ITER_DONE) {
   /* Re-use deleted entries. */
   if (!result_entry)
        result_entry = entry;
   continue;
  }
  if (!entry->oe_nameobj) {
   if (!result_entry) {
    /* Check if we must re-hash the table. */
    if (self->ot_size + 1 >= (self->ot_mask*2)/3) {
     size_t new_mask;
     new_mask = (self->ot_mask << 1) | 1;
     if (self->ot_used < self->ot_size)
         new_mask = self->ot_mask; /* It's enough if we just rehash to get rid of deleted entries. */
     if (new_mask < 7) new_mask = 7;
     if likely(JITObjectTable_TryRehash(self,new_mask))
        goto again;
     if (self->ot_size == self->ot_mask) {
      new_mask = (self->ot_mask << 1) | 1;
      if (self->ot_used < self->ot_size)
          new_mask = self->ot_mask; /* It's enough if we just rehash to get rid of deleted entries. */
      for (;;) {
       if likely(JITObjectTable_TryRehash(self,new_mask))
          goto again;
       if unlikely(!Dee_CollectMemory((new_mask + 1) * sizeof(struct jit_object_entry)))
          return -1;
      }
     }
    }
    ++self->ot_size;
    result_entry = entry;
   }
   break;
  }
  if (entry->oe_namehsh != namehsh) continue;
  if (entry->oe_namelen != namelen) continue;
  if (memcmp(entry->oe_namestr,namestr,namelen * sizeof(char)) != 0) continue;
  /* Existing entry! */
  if (override_existing) {
#if 1
   DREF DeeObject *value;
   value = entry->oe_value;
   Dee_XIncref(value);
   entry->oe_value = value;
   COMPILER_BARRIER();
   Dee_XDecref(value);
#else
   DREF DeeObject *name,*value;
   name  = entry->oe_nameobj;
   value = entry->oe_value;
   Dee_Incref(nameobj);
   Dee_XIncref(value);
   entry->oe_nameobj = nameobj;
   entry->oe_namestr = namestr;
   entry->oe_value   = value;
   COMPILER_BARRIER();
   /* Cleanup the old values. */
   Dee_Decref_unlikely(name);
   Dee_XDecref(value);
#endif
  }
  return 1;
 }
 ++self->ot_used;
 Dee_Incref(nameobj);
 result_entry->oe_nameobj = nameobj;
 result_entry->oe_namestr = namestr;
 result_entry->oe_namelen = namelen;
 result_entry->oe_namehsh = namehsh;
 result_entry->oe_value = value;
 Dee_XIncref(value);
 return 0;
}

/* Delete an existing entry for an object with the given name
 * @return: true:  Successfully deleted the entry, after potentially unbinding an associated object.
 * @return: false: The object table didn't include an entry matching the given name. */
INTERN bool DCALL
JITObjectTable_Delete(JITObjectTable *__restrict self,
                      /*utf-8*/unsigned char *namestr,
                      size_t namelen, dhash_t namehsh) {
 dhash_t i,perturb;
 perturb = i = namehsh & self->ot_mask;
 for (;; JITObjectTable_NEXT(i,perturb)) {
  struct jit_object_entry *entry;
  DREF DeeObject *name,*value;
  entry = &self->ot_list[i & self->ot_mask];
  if (entry->oe_nameobj == ITER_DONE) continue;
  if (!entry->oe_nameobj) break;
  if (entry->oe_namehsh != namehsh) continue;
  if (entry->oe_namelen != namelen) continue;
  if (memcmp(entry->oe_namestr,namestr,namelen * sizeof(char)) != 0) continue;
  /* Found it! */
  name = entry->oe_nameobj;
  value = entry->oe_value;
  entry->oe_nameobj = ITER_DONE;
  ASSERT(self->ot_size);
  ASSERT(self->ot_used);
  --self->ot_used;
  COMPILER_BARRIER();
  if (self->ot_used < self->ot_mask/3)
      JITObjectTable_TryRehash(self,self->ot_mask >> 1);
  COMPILER_BARRIER();
  Dee_XDecref(value);
  Dee_Decref_unlikely(name);
  return true;
 }
 return false;
}


/* Lookup a given object within `self'
 * @return: * :   The entry associated with the given name.
 * @return: NULL: Could not find an object matching the specified name. (no error was thrown) */
INTERN struct jit_object_entry *DCALL
JITObjectTable_Lookup(JITObjectTable *__restrict self,
                      /*utf-8*/unsigned char *namestr,
                      size_t namelen, dhash_t namehsh) {
 dhash_t i,perturb;
 perturb = i = namehsh & self->ot_mask;
 for (;; JITObjectTable_NEXT(i,perturb)) {
  struct jit_object_entry *entry;
  entry = &self->ot_list[i & self->ot_mask];
  if (entry->oe_nameobj == ITER_DONE) continue;
  if (!entry->oe_nameobj) break;
  if (entry->oe_namehsh != namehsh) continue;
  if (entry->oe_namelen != namelen) continue;
  if (memcmp(entry->oe_namestr,namestr,namelen * sizeof(char)) != 0) continue;
  return entry;
 }
 return NULL;
}

/* Lookup or create an entry for a given name within `self'
 * @return: * :   The entry associated with the given name.
 * @return: NULL: Failed to create a new entry. (an error _WAS_ thrown) */
INTERN struct jit_object_entry *DCALL
JITObjectTable_Create(JITObjectTable *__restrict self,
                      /*utf-8*/unsigned char *namestr,
                      size_t namelen, dhash_t namehsh,
                      DeeObject *__restrict nameobj) {
 dhash_t i,perturb;
 struct jit_object_entry *result_entry;
again:
 result_entry = NULL;
 perturb = i = namehsh & self->ot_mask;
 for (;; JITObjectTable_NEXT(i,perturb)) {
  struct jit_object_entry *entry;
  entry = &self->ot_list[i & self->ot_mask];
  if (entry->oe_nameobj == ITER_DONE) {
   /* Re-use deleted entries. */
   if (!result_entry)
        result_entry = entry;
   continue;
  }
  if (!entry->oe_nameobj) {
   if (!result_entry) {
    /* Check if we must re-hash the table. */
    if (self->ot_size + 1 >= (self->ot_mask*2)/3) {
     size_t new_mask;
     new_mask = (self->ot_mask << 1) | 1;
     if (self->ot_used < self->ot_size)
         new_mask = self->ot_mask; /* It's enough if we just rehash to get rid of deleted entries. */
     if (new_mask < 7) new_mask = 7;
     if likely(JITObjectTable_TryRehash(self,new_mask))
        goto again;
     if (self->ot_size == self->ot_mask) {
      new_mask = (self->ot_mask << 1) | 1;
      if (self->ot_used < self->ot_size)
          new_mask = self->ot_mask; /* It's enough if we just rehash to get rid of deleted entries. */
      for (;;) {
       if likely(JITObjectTable_TryRehash(self,new_mask))
          goto again;
       if unlikely(!Dee_CollectMemory((new_mask + 1) * sizeof(struct jit_object_entry)))
          return NULL;
      }
     }
    }
    ++self->ot_size;
    result_entry = entry;
   }
   break;
  }
  if (entry->oe_namehsh != namehsh) continue;
  if (entry->oe_namelen != namelen) continue;
  if (memcmp(entry->oe_namestr,namestr,namelen * sizeof(char)) != 0) continue;
  /* Existing entry! */
  return entry;
 }
 ++self->ot_used;
 Dee_Incref(nameobj);
 result_entry->oe_nameobj = nameobj;
 result_entry->oe_namestr = namestr;
 result_entry->oe_namelen = namelen;
 result_entry->oe_namehsh = namehsh;
 result_entry->oe_value   = NULL;
 return result_entry;
}


DECL_END

#endif /* !GUARD_DEX_JIT_OBJECT_TABLE_C */
